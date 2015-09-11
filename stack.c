#include "stack.h"
#include "uart.h"

const uint8_t mymac[6] = {0x02,0x05,0x69,0x55,0x1c,0xc2};
const uint8_t myip[4] = {192,168,2,96};
const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/*#######################################################
# Funktion um die Checksumme zu berechnen, stammt 		#
# ursprünglich von										#
# Ulrich Radig :-),www.ulrichradig.de 					#
# wer genaueres zur Checksummenbildung wissen möchte:	#
# http://www.netfor2.com/udpsum.htm 	 				#
########################################################*/
int checksum (char * pointer, uint32_t result32, uint8_t result16) {
	uint8_t result16_1 = 0x0000;
	unsigned char DataH;
	unsigned char DataL;
	
	//Jetzt werden alle Packete in einer While Schleife addiert
	while(result16 > 1) {
		//schreibt Inhalt Pointer nach DATAH danach inc Pointer
		DataH=*pointer++;

		//schreibt Inhalt Pointer nach DATAL danach inc Pointer
		DataL=*pointer++;

		//erzeugt Int aus Data L und Data H
		result16_1 = ((DataH << 8)+DataL);
		//Addiert packet mit vorherigen
		result32 = result32 + result16_1;
		//decrimiert Länge von TCP Headerschleife um 2
		result16 -=2;
	}

	//Ist der Wert result16 ungerade ist DataL = 0
	if(result16 > 0) {
		//schreibt Inhalt Pointer nach DATAH danach inc Pointer
		DataH=*pointer;
		//erzeugt Int aus Data L ist 0 (ist nicht in der Berechnung) und Data H
		result16_1 = (DataH << 8);
		//Addiert packet mit vorherigen
		result32 = result32 + result16_1;
	}
	
	//Komplementbildung (addiert Long INT_H Byte mit Long INT L Byte)
	result32 = ((result32 & 0x0000FFFF)+ ((result32 & 0xFFFF0000) >> 16));
	result32 = ((result32 & 0x0000FFFF)+ ((result32 & 0xFFFF0000) >> 16));	
	result16 =~(result32 & 0x0000FFFF);
	
	return result16;
}

uint16_t htons(uint16_t hostshort) {
	return ((hostshort>>8)&0xff) | (hostshort<<8);
}

uint16_t ntohs(uint16_t hostshort) __attribute__((weak,alias("htons")));

uint8_t compare_macs(uint8_t mac0[6], uint8_t mac1[6]) {
	for(uint8_t i=0; i < 6; ++i) {
		if(mac0[i] != mac1[i]) {
			return 0;
		}
	}
	return 1;
}

uint8_t compare_ips(uint8_t ip0[4], uint8_t ip1[4]) {
	for(uint8_t i=0; i < 4; ++i) {
		if(ip0[i] != ip1[i]) {
			return 0;
		}
	}
	return 1;
}

// Funktion um den Ethernet II Header zu erzeugen
struct ETH_frame *eth(unsigned char *buff){
	struct ETH_frame *frame = (struct ETH_frame *) buff;
	for(uint8_t i=0; i<6; ++i) {
		frame->destMac[i] = frame->sourceMac[i];
		frame->sourceMac[i] = mymac[i];
	}
	return frame;
}

// Funktion um den IP Header zu erzeugen
struct IP_segment *ip(struct ETH_frame *frame){
	struct IP_segment *ip;

	frame->type_length = htons(TYPE_IP);
	ip = (struct IP_segment *) frame->payload;

	for(uint8_t a=0; a<4; a++) {
		ip->destIp[a] = ip->sourceIp[a];
		ip->sourceIp[a] = myip[a];
	}

	ip->checksum = 0x00; // checksum auf null setzen
	ip->checksum = htons(checksum((char *) ip, 0, IP_HEADERLENGTH)); 

	return ip;
}

/*Funktion um auf ein Arp Request eine Antwort zurück zu senden
####################################################################################*/
void arp(uint8_t len, unsigned char *buff){
	struct ETH_frame *frame = eth(buff); // ETHERNET II header erzeugen
	struct ARP_packet *arp;

	frame->type_length = ntohs(TYPE_ARP);
	arp = (struct ARP_packet *) frame->payload;
	arp->opcode = htons(0x02); //Reply einstellen
	for(uint8_t a=0; a<6; a++) {
			arp->destMac[a] = arp->sourceMac[a];
			arp->sourceMac[a] = mymac[a];
	}
	for(uint8_t a=0; a<4; a++) {
			arp->destIp[a] = arp->sourceIp[a];
			arp->sourceIp[a] = myip[a];
	}
	
	enc28j60PacketSend(ETH_HEADERLENGTH+ARP_LEN, buff);	
}
/*Funktion um ICMP packete zu beantworten
####################################################################################*/
void icmp(uint8_t len, unsigned char *buff){
	struct ETH_frame *frame = eth(buff); // ETHERNET II header erzeugen
	struct IP_segment *ip_pkt = ip(frame); // IP header erzeugen
	
	struct ICMP_header *icmp = (struct ICMP_header *) ip_pkt->payload;
	
	icmp->type = 0x00; // auf reply einstellen
	icmp->code = 0x00; 
	
	icmp->checksum = htons(checksum((char *) icmp, 0, ICMP_HEADERLENGTH));

	enc28j60PacketSend(len-4, buff);
}

/*UDP Funktion, beliebige Antwort
####################################################################################*/
void udp(uint8_t len, unsigned char *buff){ 
	// ETHERNET II header erzeugen
	struct ETH_frame *frame = eth(buff);
	struct IP_segment *ip_pkt = ip(frame);
	ip_pkt->length = htons(IP_UDP_HEADERLENGTH+len);
	struct UDP_packet *udp = (struct UDP_packet *) ip_pkt->payload;
	// checksum falsch?

	//An den Port zurücksenden, von dem das Packet gekommen ist
	uint16_t tempport = udp->destPort; // Ziel Port zwischenspeichern
	udp->destPort = udp->sourcePort; //ZielPort neuschreiben
	udp->sourcePort = tempport; // SourcePort neuschreiben

	//Udp Header&Datenlänge schreiben
	udp->length = htons(UDP_HEADERLENGTH+len);
	udp->destPort = htons(udp->destPort);
	udp->sourcePort = htons(udp->sourcePort);

	//Checksumme ausrechnen...
	udp->checksum = 0x00;
	udp->checksum = htons(checksum((char *) udp, TYPE_UDP+UDP_HEADERLENGTH+len, 16+len));
	
	enc28j60PacketSend(IP_UDP_HEADERLENGTH + ETH_HEADERLENGTH + len, buff);
}



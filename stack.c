#include "stack.h"
#include "uart.h"
#include "util.h"

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
uint16_t checksum(uint16_t len, char *pointer) {
	uint16_t result16_1 = 0;
	uint32_t result32 = 0;
	uint8_t DataH, DataL;
	
	//Jetzt werden alle Packete in einer While Schleife addiert
	while(len > 1) {
		//schreibt Inhalt Pointer nach DATAH danach inc Pointer
		DataH=*pointer++;

		//schreibt Inhalt Pointer nach DATAL danach inc Pointer
		DataL=*pointer++;

		//erzeugt Int aus Data L und Data H
		result16_1 = ((DataH << 8)+DataL);
		//Addiert packet mit vorherigen
		result32 = result32 + result16_1;
		//decrimiert Länge von TCP Headerschleife um 2
		len -= 2;
	}

	//Ist der Wert result16 ungerade ist DataL = 0
	if(len > 0) {
		//schreibt Inhalt Pointer nach DATAH danach inc Pointer
		DataH = *pointer;
		//erzeugt Int aus Data L ist 0 (ist nicht in der Berechnung) und Data H
		result16_1 = (DataH << 8);
		//Addiert packet mit vorherigen
		result32 = result32 + result16_1;
	}
	
	//Komplementbildung (addiert Long INT_H Byte mit Long INT L Byte)
	result32 = ((result32 & 0x0000FFFF)+ ((result32 & 0xFFFF0000) >> 16));
	result32 = ((result32 & 0x0000FFFF)+ ((result32 & 0xFFFF0000) >> 16));	
	len =~(result32 & 0x0000FFFF);
	
	return ntohs(len);
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
struct IP_segment *ip(uint16_t length, struct ETH_frame *frame){
	struct IP_segment *ip_pkt  = (struct IP_segment *) frame->payload;
	frame->type_length = htons(TYPE_IP4);

	for(uint8_t a=0; a<4; a++) {
		ip_pkt->destIp[a] = ip_pkt->sourceIp[a];
		ip_pkt->sourceIp[a] = myip[a];
	}

	ip_pkt->length = htons(length);

	ip_pkt->checksum = 0;
	ip_pkt->checksum = checksum(IP_HEADERLENGTH, (char *) ip_pkt);

	return ip_pkt;
}

// Funktion um auf ein Arp Request eine Antwort zurück zu senden
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
	
	enc28j60PacketSend(len, buff);
}

// Funktion um ICMP packete zu beantworten
void icmp(uint8_t len, unsigned char *buff){
	struct ETH_frame *frame = eth(buff); // ETHERNET II header erzeugen
	struct IP_segment *ip_pkt = ip(len-ETH_HEADERLENGTH, frame); // IP header erzeugen
	
	struct ICMP_header *icmp = (struct ICMP_header *) ip_pkt->payload;
	
	icmp->type = 0x00; // auf reply einstellen
	icmp->code = 0x00; 
	
	icmp->checksum = 0;
	icmp->checksum = checksum(len-IP_HEADERLENGTH-ETH_HEADERLENGTH, (char *) icmp);

	enc28j60PacketSend(len, buff);
}

// UDP Funktion, beliebige Antwort
void udp(uint16_t len, unsigned char *buff){ 
	// ETHERNET II header erzeugen
	struct ETH_frame *frame = eth(buff);
	struct IP_segment *ip_pkt = ip(len+IP_HEADERLENGTH, frame);
	struct UDP_packet *udp = (struct UDP_packet *) ip_pkt->payload;

	//An den Port zurücksenden, von dem das Packet gekommen ist
	uint16_t tempport = udp->destPort;
	udp->destPort = udp->sourcePort;
	udp->sourcePort = tempport;

	// write ports and length
	udp->length = htons(len);

	// checksum is optional
	udp->checksum = 0;
	
	enc28j60PacketSend(len+ETH_HEADERLENGTH+IP_HEADERLENGTH, buff);
}



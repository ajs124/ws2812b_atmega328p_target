#include "stack.h"
#include "uart.h"

/*NIC Einstellungen !! -----------------------------------
#########################################################*/

volatile unsigned char myip[4] = {192,168,2,96};
volatile unsigned char mymac[6] = {0x02,0x05,0x69,0x55,0x1c,0xc2};

/*#######################################################
# Funktion um die Checksumme zu berechnen, stammt 		#
# ursprünglich von										#
# Ulrich Radig :-),www.ulrichradig.de 					#
# wer genaueres zur Checksummenbildung wissen möchte:	#
# http://www.netfor2.com/udpsum.htm 	 				#
########################################################*/
int checksum (unsigned char * pointer,unsigned long result32,unsigned int result16)
{
	unsigned int result16_1 = 0x0000;
	unsigned char DataH;
	unsigned char DataL;
	
	//Jetzt werden alle Packete in einer While Schleife addiert
	while(result16 > 1)
		{
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
	if(result16 > 0)
		{
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
	
	return (result16);
}

uint16_t htons(uint16_t hostshort) {
	return ((hostshort>>8)&0xff) | (hostshort<<8);
}

uint16_t ntohs(uint16_t hostshort) __attribute__((weak,alias("htons")));

/*	Funktionen, um zu überprüfen, ob das Packet für uns bestimmt ist, weitere Prüf'routinen'
	sind als Macros in stack.h definiert, da sie unabhängig von der IP immer auf dieselbe Art
	geprüft werden können
######################################################################################*/
uint8_t checkmymac(void) {
	for(uint8_t i=0; i < 6; ++i) {
		if(buffer[i] != mymac[i]) {
			return 0;
		}
	}
	return 1;
}

uint8_t checkmyip(void) {
	for(uint8_t i=0; i < 4; ++i) {
		if(buffer[38+i] != myip[i]) {
			return 0;
		}
	}
	return 1;
}

// Funktion um den Ethernet II Header zu erzeugen
void eth(unsigned char *buff){
	for(uint8_t a=0; a<6; a++) {
		buff[a] = buff[a+6];
		buff[a+6] = mymac[a];
	}
}

// Funktion um den IP Header zu erzeugen
void ip(unsigned char *buff){
	struct IP_segment *ip;
	unsigned int sum;

	ip = (struct IP_segment *) &buff[24];

	for(uint8_t a=0;a<4; a++)
	{		
		ip->destIp[a] = ip->sourceIp[a];
		ip->sourceIp[a] = myip[a];
	}


	ip->checksum = 0x00; // checksum auf null setzen
	sum = checksum(&buff[14],0x00000000,20); // checksumme ausrechnen
	
	ip->checksum = htons(sum); 
}
/*Funktion um auf ein Arp Request eine Antwort zurück zu senden
####################################################################################*/
void arp(unsigned int len, unsigned char *buff){
		
	eth(buff); // ETHERNET II header erzeugen
	struct ARP_header *arp;
	arp = (struct ARP_header *)&buff[21];
	arp->Opcode = 0x02; //Reply einstellen
	for(unsigned char a=0; a<6; a++)
		{			
			arp->ARP_destMac[a] = arp->ARP_sourceMac[a];
			arp->ARP_sourceMac[a] = mymac[a];
		}
	for(unsigned char a=0; a<4; a++)
		{		
			arp->ARP_destIp[a] = arp->ARP_sourceIp[a];
			arp->ARP_sourceIp[a] = myip[a];
		}
	
	
	enc28j60PacketSend(len-4,buff);	
}
/*Funktion um ICMP packete zu beantworten
####################################################################################*/
void icmp(unsigned int len, unsigned char *buff){
	eth(buff); // ETHERNET II header erzeugen
	ip(buff); // IP header erzeugen
	
	struct ICMP_header *icmp;
	icmp = (struct ICMP_header *)&buff[34];
	
	icmp->ICMP_type = 0x00; // auf reply einstellen
	icmp->ICMP_code = 0x00; 
	
	//Simple ICMP Checksummenbildung, die Idee stammt von
	//Simon, siehe http://avr.auctionant.de/
	if(icmp->ICMP_checksumByteOne >  0xFF-0x08)icmp->ICMP_checksumByteTwo++;
	icmp->ICMP_checksumByteOne+=0x08; 
	
	enc28j60PacketSend(len-4,buff);	
}

/*UDP Funktion, beliebige Antwort
####################################################################################*/
void udp(unsigned int len, unsigned char *buff){ 
	uint16_t tempport,sum;
	unsigned char *temp = buff;
	temp += ETH_HEADERLENGTH;
	temp += IP_HEADERLENGTH;
	struct UDP_packet *udp = (struct UDP_packet *) temp;
	
	// ETHERNET II header erzeugen
	eth(buff);
	
	// IP header erzeugen. ohne htons() weil unsigned char array
	buff[16] = ((IP_UDP_HEADERLENGTH+len) & 0xFF00)>>8;
	buff[17] = ((IP_UDP_HEADERLENGTH+len) & 0x00FF); // ip header length anpassen
	
	ip(buff);

	//An den Port zurücksenden, von dem das Packet gekommen ist
	tempport = udp->destPort; // Ziel Port zwischenspeichern
	udp->destPort = udp->sourcePort; //ZielPort neuschreiben
	udp->sourcePort = tempport; // SourcePort neuschreiben

	//Udp Header&Datenlänge schreiben
	udp->length = htons(UDP_HEADERLENGTH+len);
	udp->destPort = htons(udp->destPort);
	udp->sourcePort = htons(udp->sourcePort);

	//Checksumme ausrechnen...
	udp->checksum = 0x00; // ???
	sum = checksum(&buff[26],TYPE_UDP+UDP_HEADERLENGTH+len,16+len);
	udp->checksum = htons(sum);
	
	//Und ab damit :-)
	enc28j60PacketSend(IP_UDP_HEADERLENGTH + ETH_HEADERLENGTH + len, buff);
}



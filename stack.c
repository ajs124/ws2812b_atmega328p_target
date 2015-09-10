#include "stack.h"

/*NIC Einstellungen !! -----------------------------------
#########################################################*/

volatile unsigned char myip[4] = {192,168,1,96};
volatile unsigned char mymac[6] = {0xab,0xbc,0x6f,0x55,0x1c,0xc2};

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
/*	Funktionen, um zu überprüfen, ob das Packet für uns bestimmt ist, weitere Prüf'routinen'
	sind als Macros in stack.h definiert, da sie unabhängig von der IP immer auf dieselbe Art
	geprüft werden können
######################################################################################*/
uint8_t Checkmymac(void){
	if(buffer[0] == mymac[0] && buffer[1] == mymac[1] && buffer[2] == mymac[2] && buffer[3] == mymac[3] && buffer[4] == mymac[4] && buffer[5] == mymac[5])return(1);
	return(0);
}

uint8_t Checkmyip(void){
	if(buffer[38] == myip[0] && buffer[39] == myip[1] && buffer[40] == myip[2] && buffer[41] == myip[3])return(1);
	return(0);
}

/*Funktion um den Ethernet II Header zu erzeugen
####################################################################################*/
void eth(unsigned char *buff){
	
	for(unsigned char a=0; a<6; a++)
		{		
			buff[a] = buff[a+6];
			buff[a+6] = mymac[a];
		}
}
/*Funktion um den IP Header zu erzeugen
####################################################################################*/
void ip(unsigned char *buff){
	struct IP_header *ip;
	unsigned int sum;
	
	ip = (struct IP_Header *)&buff[24];

	for(unsigned char a=0;a<4; a++)
	{		
		ip->IP_destIp[a] = ip->IP_sourceIp[a];
		ip->IP_sourceIp[a] = myip[a];
	}


	ip->IP_checksum = 0x00; // checksum auf null setzen
	sum = checksum(&buff[14],0x00000000,20); // checksumme ausrechnen
	
	ip->IP_checksum = ((sum & 0xFF00) >> 8)|((sum & 0x00FF)<<8); 
}
/*Funktion um auf ein Arp Request eine Antwort zurück zu senden
####################################################################################*/
void arp(unsigned int len, unsigned char *buff){
		
	eth(buff); // ETHERNET II header erzeugen
	struct ARP_header *arp;
	arp = (struct ARP_Header *)&buff[21];
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
	icmp = (struct ICMP_Header *)&buff[34];
	
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
	struct UDP_header *udp;
	udp = (struct UDP_Header *)&buff[34];
	
	// ETHERNET II header erzeugen
	eth(buff); 
	
	// IP header erzeugen
	buff[16] = ((IP_UDP_HEADERLENGHT+len) & 0xFF00)>>8;
	buff[17] = ((IP_UDP_HEADERLENGHT+len) & 0x00FF); // ip header lenght anpassen
	
	ip(buff); 
		
	//An den Port zurücksenden, von dem das Packet gekommen ist
	tempport = udp->UDP_destPort; // Ziel Port zwischenspeichern
	udp->UDP_destPort = udp->UDP_sourcePort; //ZielPort neuschreiben
	udp->UDP_sourcePort = tempport; // SourcePort neuschreiben

	//Udp Header&Datenlänge schreiben
	udp->UDP_lenght_h = ((UDP_HEADERLENGHT+len) & 0xFF00)>>8;
	udp->UDP_lenght_l = ((UDP_HEADERLENGHT+len) & 0x00FF);
	
	//Checksumme ausrechnen...
	udp->UDP_checksum = 0x00;
	sum = checksum(&buff[26],TYPE_UDP+UDP_HEADERLENGHT+len,16+len); 
	udp->UDP_checksum = ((sum & 0xFF00) >> 8)|((sum & 0x00FF)<<8);
	
	//Und ab damit :-)
	enc28j60PacketSend(IP_UDP_HEADERLENGHT + ETH_HEADERLENGHT +len,buff);	
}



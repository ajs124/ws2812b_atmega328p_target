/*	##################################
	-Der buffer ist in stack.h definiert
	 er ist in der Grösse ziemlich variabel,
	 wenn man nicht gerade TCP verwendet,
	 dann sollte auch eine Buffergrösse von
	 256 Reichen, für nur arp und udp evt
	 sogar 128
	-Ip & Mac wird in stack.c festgelegt!
	-der zu verwendende Pin für /CS wird
	 am Anfang von enc28h60.h festgelegt
	####################################
*/
#include <avr/io.h>
#include "stack.h"
#include "enc28j60.h"


#define TERMINATE 1
#define DONT_TERMINATE 0

/*einfache memcpy*/ 
void printinbuffer(uint8_t *buff,uint8_t *text,uint8_t terminate){
	while(*text){
		*buff++ = *text++;
	}
	if(terminate) *buff++ = 0x00;
}


/*	einfache strcmp, zwecks Lerneffekt auch selbst gemacht*/
uint8_t compare(uint8_t *buffone, uint8_t *bufftwo){
	
	uint8_t counterone=0,countertwo=0;
	while(*bufftwo){
		if(*buffone++ == *bufftwo++){counterone++;}
		countertwo++;
	}
	if(counterone==countertwo) return(1);
	return(0);

}


int main(void){

	
	unsigned int packet_lenght;
	
	
	/*ENC Initialisieren*/
	enc28j60Init();
	//Mac Adresse setzen(stack.h, dort wird auch die Ip festgelegt)
	nicSetMacAddress(mymac);
	
	/*	Leds konfigurieren
		LEDA : Link status
		LEDB : Receive activity
		Led Settings : enc28j60 datasheet, page 11*/
	
	enc28j60PhyWrite(0x14,0b0000010000100000);
	
	/*	Leds konfigurieren
		LEDA : Link status
		LEDB : Blink Fast
		Led Settings : enc28j60 datasheet, page 11
		Auskommentieren, wenn mans lieber blinken sieht ;-)		*/
	
	//enc28j60PhyWrite(0x14,0b0000010000100000);

	
	while(1){
	//Buffer des Enc's abhohlen :-)
	packet_lenght = enc28j60PacketReceive(BUFFER_SIZE, buffer);
		
		/*Wenn ein Packet angekommen ist, ist packet_lenght =! 0*/
		if(packet_lenght){
		
		
			/*Ist das Packet ein Broadcast packet, vom Typ Arp und an unsere Ip gerichtet?*/
			if(Checkbroadcast() && Checkarppackage() && Checkmyip()){
						arp(packet_lenght, buffer);
			}
	
			/*Ist das Packet kein Broadcast, sondern explizit an unsere mac adresse gerichtet?*/
			if(Checkmymac()){
				
				/*Handelt es sich um ein ICMP Packet? ->beantworten (Pong)*/
				if (buffer[IP_TYPEFIELD] == TYPE_ICMP)icmp(packet_lenght,buffer);
				
				
				/*Handelt es sich um ein UDP Packet, das auf Port 85 reinkommt?*/
				if(buffer[IP_TYPEFIELD] == TYPE_UDP && buffer[UDP_PORT_L] == 85){	
					
					/*	Folgendes dient bloss als Anwendungsbeispiel, 
						die empfangenen Daten liegen im buffer ab adresse UDP_DATA
						und können natürlich auch für ganz andere Zweche verwendet
						werden.
						TODO: -Funktion um nicht nur auf UDP Packete zu antworten,
							   sondern auch welche zu erzeugen
							  -TCP(muss ich erst noch verstehen :-) )
					*/
					
					if(compare(&buffer[UDP_DATA], "test")){
						
						printinbuffer(&buffer[UDP_DATA], "Das Test Packetchen ist angekommen :-)",TERMINATE);
						//Die Länge 38 bezieht sich hier nur auf die Nutzdaten, headerlänge etc wird von der
						//Udp Funktion natürlich selbst übernommen...
						udp(38,buffer); 
					}
					
				}
	
			}
			
		}
	
	
	}
	
	return (1);
}

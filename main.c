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
#include <string.h>
#include "stack.h"
#include "enc28j60.h"
#include "uart.h"

#define TERMINATE 1
#define DONT_TERMINATE 0

void printinbuffer(uint8_t *buff,uint8_t *text,uint8_t terminate){
	while(*text){
		*buff++ = *text++;
	}
	if(terminate) *buff++ = '\0';
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
    init_uart();
	
	unsigned int packet_length;
	
    uart_puts("beginning startup procedure\r\n");    
	/*ENC Initialisieren*/
	enc28j60Init();
    uart_puts("initialization finished\r\n");
	//Mac Adresse setzen(stack.h, dort wird auch die Ip festgelegt)
	nicSetMacAddress(mymac);
	uart_puts("mac address set\r\n");
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
		packet_length = enc28j60PacketReceive(BUFFER_SIZE, buffer);
		
		/*Wenn ein Packet angekommen ist, ist packet_length =! 0*/
		if(packet_length){
			/*Ist das Packet ein Broadcast Packet, vom Typ Arp und an unsere Ip gerichtet?*/
			if(Checkbroadcast() && Checkarppackage() && Checkmyip()){
				arp(packet_length, buffer);
			}

			/*Ist das Packet kein Broadcast, sondern explizit an unsere mac adresse gerichtet?*/
			if(Checkmymac()){
				/*Handelt es sich um ein ICMP Packet? ->beantworten (Pong)*/
				if (buffer[IP_TYPEFIELD] == TYPE_ICMP)
					icmp(packet_length,buffer);
				
				/*Handelt es sich um ein UDP Packet, das auf Port 85 reinkommt?*/
				if(buffer[IP_TYPEFIELD] == TYPE_UDP && buffer[UDP_PORT_L] == 85){	
					if(compare(&buffer[UDP_DATA], "test")){
						printinbuffer(&buffer[UDP_DATA], "Das Test Packetchen ist angekommen :-)",TERMINATE);
						//Die Länge 38 bezieht sich hier nur auf die Nutzdaten, headerlänge etc wird von der
						//Udp Funktion natürlich selbst übernommen...
						udp(38,buffer); 
					}
/*					if(compare(&buffer[UDP_DATA], "uart")) {
						uint16_t udp_len = (uint16_t) &buffer[UDP_LEN];
						char out[udp_len];
						strncpy(out, &buffer[UDP_DATA]+5, udp_len-5);
						uart_puts(out);
						uart_puts("\r\n");
					}*/
				}
			}
		}
	}
	
	return 1;
}

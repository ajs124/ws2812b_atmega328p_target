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
#if DEBUG
	#include <stdlib.h>
	#include <stdio.h>
#endif
#include "uart.h"
#include "stack.h"
#include "enc28j60.h"

#define TERMINATE 1
#define DONT_TERMINATE 0

void printinbuffer(unsigned char *buff, char *text, uint8_t terminate){
	while(*text){
		*buff++ = *text++;
	}
	if(terminate) *buff++ = '\0';
}


/*	einfache strcmp, zwecks Lerneffekt auch selbst gemacht*/
uint8_t compare(unsigned char *buffone, char *bufftwo){
	uint8_t counterone=0,countertwo=0;
	while(*bufftwo){
		if(*buffone++ == *bufftwo++){counterone++;}
		countertwo++;
	}
	if(counterone==countertwo) return(1);
	return(0);

}

// based on https://stackoverflow.com/questions/7775991/how-to-get-hexdump-of-a-structure-data
#if DEBUG
void hexdump (void *addr, int len) {
    uint8_t i;
    char buff[17], c[8];
    char *pc = (char*)addr;

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0) {
                uart_puts("  ");
                uart_puts(buff);
                uart_puts("\r\n");
            }

            // Output the offset.
            sprintf(c, "  %04x  ", i);
            uart_puts(c);
        }

        // Now the hex code for the specific character.
        sprintf(c, " %02x", pc[i]);
        uart_puts(c);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        uart_puts("   ");
        i++;
    }

    // And print the final ASCII bit.
    uart_puts("  ");
    uart_puts(buff);
    uart_puts("\r\n");
}
#endif

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
		// Buffer des Enc's abhohlen :-)
		packet_length = enc28j60PacketReceive(BUFFER_SIZE, buffer);
		
		// Wenn ein Packet angekommen ist, ist packet_length =! 0
		if(packet_length) {
//			hexdump(buffer, 64);
			// Ist das Packet ein Broadcast Packet, vom Typ Arp und an unsere Ip gerichtet?
			if(checkbroadcast() && checkarppackage() && checkmyip()){
				arp(packet_length, buffer);
			}

			// Ist das Packet kein Broadcast, sondern explizit an unsere mac adresse gerichtet?
			if(checkmymac()){
				struct ETH_frame *frame = (struct ETH_frame *) buffer;
				if(frame->type_length > 1500 && frame->type_length == 0x0800) {
					struct IP_segment *ip = (struct IP_segment *) frame->payload;
					if(ip->protocol == TYPE_ICMP) {
						icmp(packet_length, buffer);
						continue;
					} else if(ip->protocol == TYPE_UDP) {
						struct UDP_packet *pkt = (struct UDP_packet *) ip->payload;
						pkt->length = ntohs(pkt->length);
						pkt->destPort = ntohs(pkt->destPort);
						pkt->sourcePort = ntohs(pkt->sourcePort);
						pkt->checksum = ntohs(pkt->checksum);

						if(pkt->destPort == 85 && compare(pkt->data, "test")) {
							printinbuffer(pkt->data, "Test erfolgreich!", TERMINATE);
							udp(18, buffer);
							continue;
						}

						if(pkt->destPort == 86) {
							uart_puts((char*) pkt->data);
							uart_puts("\r\n");
							continue;
						}

						if(pkt->destPort == 7) {
							udp(pkt->length, buffer);
						}
					}
				}
			}
		}
	}
	
	return 1;
}

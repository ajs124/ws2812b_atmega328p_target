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
#endif
#include "uart.h"
#include "stack.h"
#include "enc28j60.h"
#include "util.h"

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

	
	while(1) {
		// Buffer des Enc's abhohlen :-)
		packet_length = enc28j60PacketReceive(BUFFER_SIZE, buffer);
		
		// Wenn ein Packet angekommen ist, ist packet_length =! 0
		if(packet_length) {
			struct ETH_frame *frame = (struct ETH_frame *) buffer;
			frame->type_length = ntohs(frame->type_length);
			// arping uses unicast after the first by default
			if(compare_macs(frame->destMac, (uint8_t *) BROADCAST_MAC) || compare_macs(frame->destMac, (uint8_t *) mymac)) {
				if(frame->type_length == TYPE_ARP) {
					struct ARP_packet *arp_pkt = (struct ARP_packet *) frame->payload;
					if(compare_ips(arp_pkt->destIp, (uint8_t *) myip)) {
						arp(packet_length, buffer);
					}
					continue;
				}
			}

			if(compare_macs(frame->destMac, (uint8_t *) mymac)) {
				uart_puts("unicast received. macs match\r\n");
				if(frame->type_length == TYPE_IP) {
					struct IP_segment *ip = (struct IP_segment *) frame->payload;
					#if DEBUG
					hexdump(ip, 64);
					#endif
					if(ip->protocol == TYPE_ICMP) {
						icmp(packet_length, buffer);
						continue;
					} else if(ip->protocol == TYPE_UDP) {
						struct UDP_packet *pkt = (struct UDP_packet *) ip->payload;
						#if DEBUG
						hexdump(pkt, 64);
						#endif
						pkt->length = ntohs(pkt->length);
						pkt->destPort = ntohs(pkt->destPort);
						pkt->sourcePort = ntohs(pkt->sourcePort);
						pkt->checksum = ntohs(pkt->checksum);
						#if DEBUG
						hexdump(pkt, 64);
						#endif

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

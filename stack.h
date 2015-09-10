#include "enc28j60.h"

uint8_t Checkmyip(void);
uint8_t Checkmymac(void);
extern void eth( unsigned char *buff);
extern void ip(unsigned char *buff);
extern void arp(unsigned int len, unsigned char *buff);
extern void udp(unsigned int len, unsigned char *buff);
extern void icmp(unsigned int len, unsigned char *buff);

/*Buffer Grösse*/
#define BUFFER_SIZE 512
uint8_t buffer[BUFFER_SIZE];

volatile unsigned char mymac[6];
volatile unsigned char myip[4];

#define Checkbroadcast() buffer[0] == 0xff && buffer[1] == 0xff && buffer[2] == 0xff && buffer[3] == 0xff && buffer[4] == 0xff && buffer[5] == 0xff
#define Checkarppackage() buffer[12] == 0x08 && buffer[13] == 0x06

#define IP_TYPEFIELD 23
#define PORT_FIELD_L 37

#define UDP_DATA 42
#define UDP_PORT_L 37
#define UDP_PORT_H 36
#define UDP_LEN 38
#define PORT_HTTP 80
#define PORT_FTP 21


/*-------------------------------------------

	STACK.C Structs

-------------------------------------------*/
#define ETH_HEADERLENGHT 14
#define IP_UDP_HEADERLENGHT 28
#define IP_HEADERLENGHT 20
#define UDP_HEADERLENGHT 8

#define TYPE_UDP 0x11
#define TYPE_TCP 0x06
#define TYPE_ICMP 0x01

struct ETH_header	{
	unsigned char ETH_destMac[6];	
	unsigned char ETH_sourceMac[6];
	
};
struct ARP_header	{
	unsigned char Opcode;	
	unsigned char ARP_sourceMac[6];	
	unsigned char ARP_sourceIp[4];
	unsigned char ARP_destMac[6];
	unsigned char ARP_destIp[4];
	
};
struct IP_header{
	uint16_t IP_checksum;
	uint8_t IP_sourceIp[4];	
	uint8_t IP_destIp[4];
};
struct ICMP_header{
	uint8_t ICMP_type;
	uint8_t ICMP_code;
	uint8_t ICMP_checksumByteOne;
	uint8_t ICMP_checksumByteTwo;
//Rest wird ausser Acht gelassen, da er nicht zwingend verändert werden muss!
	
};
struct UDP_header{
	uint16_t UDP_sourcePort;	
	uint16_t UDP_destPort;
	uint8_t UDP_length_h;
	uint8_t UDP_length_l;
	uint16_t UDP_checksum;
};


#include "enc28j60.h"

uint8_t checkmyip(void);
uint8_t checkmymac(void);
extern void eth( unsigned char *buff);
extern void ip(unsigned char *buff);
extern void arp(unsigned int len, unsigned char *buff);
extern void udp(unsigned int len, unsigned char *buff);
extern void icmp(unsigned int len, unsigned char *buff);
uint16_t htons(uint16_t hostshort);
uint16_t ntohs(uint16_t hostshort);


/*Buffer Grösse*/
#define BUFFER_SIZE 1024
unsigned char buffer[BUFFER_SIZE];

volatile unsigned char mymac[6];
volatile unsigned char myip[4];

#define checkbroadcast() buffer[0] == 0xff && buffer[1] == 0xff && buffer[2] == 0xff && buffer[3] == 0xff && buffer[4] == 0xff && buffer[5] == 0xff
#define checkarppackage() buffer[12] == 0x08 && buffer[13] == 0x06

#define IP_TYPEFIELD 23
#define PORT_FIELD_L 37

#define UDP_DATA 42
#define UDP_PORT 36
#define UDP_LEN 38
#define PORT_HTTP 80
#define PORT_FTP 21


/*-------------------------------------------

	STACK.C Structs

-------------------------------------------*/
#define ETH_HEADERLENGTH 14
#define IP_HEADERLENGTH 20
#define UDP_HEADERLENGTH 8
#define IP_UDP_HEADERLENGTH IP_HEADERLENGTH + UDP_HEADERLENGTH

#define TYPE_UDP 0x11
#define TYPE_TCP 0x06
#define TYPE_ICMP 0x01

struct ETH_frame {
	uint8_t ETH_destMac[6];
	uint8_t ETH_sourceMac[6];
	uint16_t type_length;
	uint8_t payload[BUFFER_SIZE-ETH_HEADERLENGTH];
};

struct ARP_header {
	uint8_t Opcode;
	uint8_t ARP_sourceMac[6];
	uint8_t ARP_sourceIp[4];
	uint8_t ARP_destMac[6];
	uint8_t ARP_destIp[4];
};

struct IP_segment {
	uint8_t version : 4;
	uint8_t ihl : 4;
	uint8_t tos;
	uint16_t length;
	uint16_t identification;
	uint8_t flags: 2;
	uint16_t offset : 14;
	uint8_t ttl : 4;
	uint8_t protocol : 4;
	uint16_t checksum;
	uint8_t sourceIp[4];
	uint8_t destIp[4];
//	uint8_t padding[ihl];
	uint8_t payload[BUFFER_SIZE-ETH_HEADERLENGTH-IP_HEADERLENGTH];
};

struct ICMP_header{
	uint8_t ICMP_type;
	uint8_t ICMP_code;
	uint8_t ICMP_checksumByteOne;
	uint8_t ICMP_checksumByteTwo;
//Rest wird ausser Acht gelassen, da er nicht zwingend verändert werden muss!
	
};

struct UDP_packet {
	uint16_t sourcePort;
	uint16_t destPort;
	uint16_t length;
	uint16_t checksum;
	unsigned char data[BUFFER_SIZE-ETH_HEADERLENGTH-IP_UDP_HEADERLENGTH];
};
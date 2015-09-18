#include "enc28j60.h"

#ifndef STACK_H
#define STACK_H

extern struct ETH_frame *eth(unsigned char *buff);
extern struct IP_segment *ip(uint16_t length, struct ETH_frame *frame);
extern void arp(uint8_t len, unsigned char *buff);
extern void udp(uint16_t len, unsigned char *buff);
extern void icmp(uint8_t len, unsigned char *buff);
extern uint8_t compare_macs(uint8_t mac0[6], uint8_t mac1[6]);
extern uint8_t compare_ips(uint8_t ip0[4], uint8_t ip1[4]);
extern uint16_t htons(uint16_t hostshort);
extern uint16_t ntohs(uint16_t hostshort);


// size chosen by expriment. this works for returning all packets send with nc -u, because linux spilts at 1066 bytes per frame
#define BUFFER_SIZE 1100
unsigned char buffer[BUFFER_SIZE];

extern const uint8_t mymac[6];
extern const uint8_t myip[4];
extern const uint8_t BROADCAST_MAC[6];

#define BROADCAST_BIT 512

#define PORT_HTTP 80
#define PORT_FTP 21


// STACK.C Structs
#define ETH_HEADERLENGTH 14
#define IP_HEADERLENGTH 20
#define ICMP_HEADERLENGTH 8
#define UDP_HEADERLENGTH 8
#define IP_UDP_HEADERLENGTH IP_HEADERLENGTH + UDP_HEADERLENGTH
#define ARP_LEN 28

// 
#define TYPE_UDP 0x11
#define TYPE_TCP 0x06
#define TYPE_ICMP 0x01

// ethertypes
#define TYPE_IP4 0x0800
#define TYPE_IP6 0x86DD
#define TYPE_ARP 0x0806
#define TYPE_WOL 0x0842
#define TYPE_APPLETALK 0x809B
#define TYPE_VLAN 0x8100

struct ETH_frame {
	uint8_t destMac[6];
	uint8_t sourceMac[6];
	uint16_t type_length;
	uint8_t payload[BUFFER_SIZE-ETH_HEADERLENGTH];
};

struct ARP_packet {
	uint16_t htype;
	uint16_t ptype;
	uint8_t hlen;
	uint8_t plen;
	uint16_t opcode;
	uint8_t sourceMac[6];
	uint8_t sourceIp[4];
	uint8_t destMac[6];
	uint8_t destIp[4];
};

struct IP_segment {
	uint8_t version : 4;
	uint8_t ihl : 4;
	uint8_t dscp : 6;
	uint8_t ecn : 2;
	uint16_t length;
	uint16_t identification;
	uint8_t flags: 2;
	uint16_t offset : 14;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t checksum;
	uint8_t sourceIp[4];
	uint8_t destIp[4];
//	uint8_t options[ihl];
	uint8_t payload[BUFFER_SIZE-ETH_HEADERLENGTH-IP_HEADERLENGTH];
};

struct ICMP_header{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint8_t rest[4];
};

struct UDP_packet {
	uint16_t sourcePort;
	uint16_t destPort;
	uint16_t length;
	uint16_t checksum;
	unsigned char data[BUFFER_SIZE-ETH_HEADERLENGTH-IP_UDP_HEADERLENGTH];
};

struct TCP_segment {
	uint16_t sourcePort;
};

#endif
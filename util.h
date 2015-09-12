#ifndef UTIL_H
#define UTIL_H

#define TERMINATE 1
#define DONT_TERMINATE 0

extern void printinbuffer(unsigned char *buff, char *text, uint8_t terminate);
extern uint8_t compare(unsigned char *buffone, char *bufftwo);
extern void hexdump (void *addr, int len);

#endif
#if DEBUG
    #include <stdio.h>
    #include <uart.h>
#endif
#include <inttypes.h>

void printinbuffer(unsigned char *buff, char *text, uint8_t terminate){
    while(*text) {
        *buff++ = *text++;
    }
    if(terminate)
        *buff++ = '\0';
}

uint8_t compare(unsigned char *buffone, char *bufftwo){
    while(*bufftwo && *buffone){
        if(*buffone++ != *bufftwo++)
            return 0;
    }
    return 1;

}

// based on https://stackoverflow.com/questions/7775991/how-to-get-hexdump-of-a-structure-data
#if DEBUG
void hexdump(void *addr, int len) {
    uint8_t i;
    char buff[17], c[8];
    uint8_t *pc = (uint8_t*) addr;

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

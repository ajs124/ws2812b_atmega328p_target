#ifdef DEBUG
    #include <stdlib.h>
    #include "uart.h"
#endif
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h> 
#include <avr/sleep.h>
#include "spi.h"

int main() {
    cli();
    // alle Initialisierungsfunktionen hier aufrufen
    #ifdef DEBUG
        init_uart();
    #endif
    sei();

    uint16_t i = 0;
    char buf[16];
    while(1) {
        ultoa(i, buf, 10);
        uart_puts(buf);
        uart_puts("\r\n");
        ++i;
        _delay_ms(1000);
    }
}

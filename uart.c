#include <avr/interrupt.h>
#include "ws2812b.h"
#include "psu.h"
#include "idle.h"

void init_uart() {
    UBRR0 = 0;
    UCSR0B = (1<<RXEN0); // enable recieve
    UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00)|(0<<USBS0); // 8n1
    UCSR0A |= (1<<U2X0); // double speed bit
    sei();
}

void uart_putc(unsigned char c) {
    UCSR0B |= (1<<TXEN0);
    while (!(UCSR0A & (1<<UDRE0)));  /* warten bis Senden moeglich */
    UDR0 = c;                        /* sende Zeichen */
    UCSR0B &= ~(1<<TXEN0);
}
 
void uart_puts (char *s) {
    while (*s) {
        uart_putc(*s);
        ++s;
    }
}

void uart_loop() {
    while(1) {
        while(!(UCSR0A & (1<<RXC0)));
        uint8_t temp = UDR0;
        // reset timer
        reset_idle();
        if(temp == 0xFF) {
            // frame finished
            current = 0;
            psu_on();
            ws2812b_show();
        } else if(temp == 0xFE) {
            psu_off();
        } else if(current < numleds*3) { // protect against overflow
            leds[current++] = temp;
        }
    }
}

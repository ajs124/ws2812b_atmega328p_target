#include <avr/interrupt.h>
#include "ws2812b.h"
#include "psu.h"
#include "idle.h"

void init_uart() {
    UBRR0 = 1;
    UCSR0B = (1<<RXEN0) | (1<<RXCIE0); // enable recieve + interrupt
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

ISR(USART_RX_vect) {
    uint8_t temp = UDR0;
    // reset timer
    reset_idle();
    if(temp == 0xFF) {
        // frame finished
        current = 0;
        psu_on();
        ws2812b_show();
    } else if (temp == 0xFE) {
        psu_off();
    } else {
        leds[current++] = temp;
    }
}

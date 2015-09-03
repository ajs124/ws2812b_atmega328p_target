#ifdef DEBUG
#include <avr/interrupt.h>
#define USART_BAUDRATE 38400
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

void init_uart() {
    UBRR0 = UBRR_VALUE;
    UCSR0B = (1<<TXEN0);
    UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00)|(0<<USBS0);
}

uint8_t uart_putc(unsigned char c) {
    while (!(UCSR0A & (1<<UDRE0)));  /* warten bis Senden moeglich */
    UDR0 = c;                      /* sende Zeichen */
    return 0;
}
 
void uart_puts (char *s) {
    while (*s) {   /* so lange *s != '\0' also ungleich dem "String-Endezeichen(Terminator)" */
        uart_putc(*s);
        s++;
    }
}
#endif

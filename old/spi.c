#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"

#define DDR_SPI DDRB
#define SCK PB5
#define MISO PB4
#define MOSI PB3
#define SS PB2
#define CS PD4

#define DD_SCK DDB5
#define DD_MOSI DDB3
#define DD_SS DDB2

void init_spi() {
    // SCK, MOSI & SS as output
    DDR_SPI = (1 << DD_SCK) | (1 << DD_MOSI) | (1 << DD_SS);
   // enable spi, master, set sck to f_osc/something
    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << SPR1);
}

char spi_putc(char c) {
    // CS → low
    PORTB &= ~(1 << CS);
    SPDR = c;
    while(!(SPSR & (1 << SPIF)));
    SPDR = 0;
    while(!(SPSR & (1 << SPIF)));
    // CS → high
    PORTB |= (1 << CS);
    return SPDR;
}

void spi_puts(char *in, char *out) {
    while (*in) {
        *out = spi_putc(*in);
        ++in;
        ++out;
    }
}

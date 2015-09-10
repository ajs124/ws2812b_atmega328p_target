#include <avr/io.h>
#include "ethernet.h"
#include "spi.h"

// read control register
uint8_t eth_rcr(uint8_t bank, uint8_t reg) {
    // TODO: select register via BSEL in ECON1
    // eth_wrc(ECON1, …);
    reg &= 0b0001111; // mask first 3 bits
    return spi_putc(reg);
}

// read buffer memory
char *eth_rbm() {
    return 0;
}

// write control register
void eth_wcr(uint8_t bank, uint8_t reg, char content) {

}

// write buffer memory
void eth_wbm(uint8_t *b) {

}

// bit field set
void eth_bfs(uint8_t field, uint8_t bit) {

}

// bit field clear
void eth_bfc(uint8_t field, uint8_t bit) {

}

// soft reset
void eth_src() {

}

// hard reset
void eth_hrc() {

}

#include "uart.h"
#include <stdlib.h>

#include <util/delay.h>

void init_eth() {
    // CLKRDY == 1?
    uint8_t ready = 0;
    while(!ready) {
        PORTD |= (1 << PD4);
        eth_SPDR = 0x1D; // ESTAT
        while(!(SPSR & (1<<SPIF)));
        SPDR = 0;
        while(!(SPSR & (1<<SPIF)));
        PORTD = 0xFF;
        ready = SPDR;
        ultoa(ready, buf, 10);
        uart_puts(buf);
        uart_puts("\r\n");
        _delay_ms(1);
    }
//    }
}

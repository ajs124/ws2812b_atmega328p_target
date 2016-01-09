#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "ws2812b.h"
#include "psu.h"
#include "idle.h"

void init_io() {
    DDRD |= (1<<PD6) | (1<<PD3); // led data pin & psu power pin as output
    psu_on(); // turn on psu
}

int main(void){
    init_uart();
	
    uart_puts("beginning startup procedure\r\n");
    init_io();
    ws2812b_init();
    init_timer1();
    uart_puts("initialization finished\r\n");

    uart_loop();
	
    return 1;
}

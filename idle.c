#include <avr/interrupt.h>
#include <util/delay.h>
#include "psu.h"
#include "idle.h"
#include "ws2812b.h"

void pretty_shutdown() {
    // find last led with data
    uint16_t last = numleds;
    while(leds[last--] == 0);
    while(last > 0) {
        // shift array down by one
        for(uint16_t i=1; i <= last; i++) {
            leds[i-1] = leds[i];
        }
        leds[last] = 0;
        ws2812b_show();
        _delay_ms(3);
        last--;
    }
}

ISR(TIMER1_OVF_vect) {
    if(++idle_counter >= 7) {
        pretty_shutdown();
        if(idle_counter >= 7) {
            psu_off();
        }
    }
}

void init_timer1() {
    TCCR1B |= (1 << CS12) | (1 << CS10); // prescale F_CPU/1024
    TCNT1 = 0;                 // overflow afer ~4.2s. ((16*10^6)/1024)^-1*2^16=4.194...
    idle_counter = 1;
    TIMSK1 |= (1 << TOIE2); // Enable overflow interrupt
}

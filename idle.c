#include <avr/interrupt.h>
#include <util/delay.h>
#include "psu.h"
#include "idle.h"
#include "ws2812b.h"

#define IDLE_TIME 16 // in seconds
#define WAITS (((int)(IDLE_TIME/4.2))+1) // timer overflows after ~4.2s, see init

void pretty_shutdown() {
    // find last led with data
    uint16_t last = numleds*3;
    while(!leds[last--] && !leds[last--] && !leds[last--]);
    last++;
    while(last > 3 && idle_counter >= WAITS) {
        // shift array down by three
        for(uint16_t i=3; i < last-3; i++) {
            leds[i-3] = leds[i];
        }
        leds[last] = 0;
        leds[last-1] = 0;
        leds[last-2] = 0;
        last -= 3;
        ws2812b_show();
        _delay_ms(3);
    }
}

ISR(TIMER1_OVF_vect) {
    if(++idle_counter >= WAITS ) {
        pretty_shutdown();
        if(idle_counter >= WAITS) {
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

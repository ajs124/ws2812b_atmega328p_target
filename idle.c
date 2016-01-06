#include <avr/interrupt.h>
#include "psu.h"
#include "idle.h"

ISR(TIMER1_OVF_vect) {
    if(++idle_counter >= 7) {
        psu_off();
    }
}

void init_timer1() {
    TCCR1B |= (1 << CS12) | (1 << CS10); // prescale F_CPU/1024
    TCNT1 = 0;                 // overflow afer ~4.2s. ((16*10^6)/1024)^-1*2^16=4.194...
    idle_counter = 1;
    TIMSK1 |= (1 << TOIE2); // Enable overflow interrupt
}

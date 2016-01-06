#ifndef IDLE_H
#define IDLE_H

volatile uint8_t idle_counter;

void init_timer1();

inline void reset_idle() {
    idle_counter = 0;
    TCNT1 = 0;
}

#endif

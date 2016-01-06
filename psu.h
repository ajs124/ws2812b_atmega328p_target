#ifndef PSU_H
#define PSU_H

inline void psu_on() {
    DDRD |= (1 << PD3);
    PORTD &= ~(1 << PD3);
}

inline void psu_off() {
    DDRD &= ~(1 << PD3);
    PORTD |= (1 << PD3);
}

#endif

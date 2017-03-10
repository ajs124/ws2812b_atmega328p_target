/*-------------------------------------------------------------------------
  Derived from an Arduino library to control WS2812-based RGB LED devices.
  Currently handles 800 KHz bitstreams on 16 MHz ATmega MCUs.

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries,
  contributions by PJRC, Michael Miller and other members of the open
  source community.

  -------------------------------------------------------------------------
  This file was part of the Adafruit NeoPixel library.

  NeoPixel is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  NeoPixel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with NeoPixel.  If not, see
  <http://www.gnu.org/licenses/>.
  -------------------------------------------------------------------------*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ws2812b.h"

#if numleds > 650
    #error Number of LEDs over 650, this will most definitly not fit in memory, sorry
#endif

uint8_t reset_time_over;

void startTimer();

void ws2812b_init(void) {
  PORTD |= (1 << PD6);
  // initialize array with dummy pattern
  for(uint16_t i = 0; i < numleds*3; ++i) {
      leds[i] = 42;
  }
  reset_time_over = 1;
  // show dummy pattern
  ws2812b_show();
}

void startTimer() {
  reset_time_over = 0;
  TCCR0B |= (1 << CS01); // prescale F_CPU/8
  TCNT0 = 0xFF-100;      // overflow afer 50µs. (50*10^-6)/(((16*10^6)/8)^-1)=50µs/(1/(16MHz/8))
  TIMSK0 |= (1 << TOIE0); // Enable overflow interrupt
}

ISR(TIMER0_OVF_vect) {
  TIMSK0 &= ~(1<<TOIE0); // disable overflow
  reset_time_over = 1;
}

void ws2812b_show(void) {
  while(reset_time_over == 0); // wait for reset
  cli(); // Need 100% focus on instruction timing

  volatile uint16_t i = numleds*3; // Loop counter
  volatile uint8_t
   *port = &PORTD,
   *ptr = leds,             // Pointer to next byte
    b   = *ptr++,           // Current byte value
    hi  = *port | (1<<PD6),  // PORT w/output bit set high
    lo  = *port & ~(1<<PD6), // PORT w/output bit set low
    next = lo,
    bit = 8;

    // 20 inst. clocks per bit: HHHHHxxxxxxxxLLLLLLL
    // ST instructions:         ^   ^        ^       (T=0,5,13)

    asm volatile(
     "head20:"                   "\n\t" // Clk  Pseudocode    (T =  0)
      "st   %a[port],  %[hi]"    "\n\t" // 2    PORT = hi     (T =  2)
      "sbrc %[byte],  7"         "\n\t" // 1-2  if(b & 128)
       "mov  %[next], %[hi]"     "\n\t" // 0-1   next = hi    (T =  4)
      "dec  %[bit]"              "\n\t" // 1    bit--         (T =  5)
      "st   %a[port],  %[next]"  "\n\t" // 2    PORT = next   (T =  7)
      "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T =  8)
      "breq nextbyte20"          "\n\t" // 1-2  if(bit == 0) (from dec above)
      "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 10)
      "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 12)
      "nop"                      "\n\t" // 1    nop           (T = 13)
      "st   %a[port],  %[lo]"    "\n\t" // 2    PORT = lo     (T = 15)
      "nop"                      "\n\t" // 1    nop           (T = 16)
      "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 18)
      "rjmp head20"              "\n\t" // 2    -> head20 (next bit out)
     "nextbyte20:"               "\n\t" //                    (T = 10)
      "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 11)
      "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 13)
      "st   %a[port], %[lo]"     "\n\t" // 2    PORT = lo     (T = 15)
      "nop"                      "\n\t" // 1    nop           (T = 16)
      "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 18)
       "brne head20"             "\n"   // 2    if(i != 0) -> (next byte)
      : [port]  "+e" (port),
        [byte]  "+r" (b),
        [bit]   "+r" (bit),
        [next]  "+r" (next),
        [count] "+w" (i)
      : [ptr]    "e" (ptr),
        [hi]     "r" (hi),
        [lo]     "r" (lo));

  PORTD &= ~(1 << PD6);
  startTimer();
  sei();
}

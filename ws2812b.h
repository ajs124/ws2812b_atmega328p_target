#ifndef WS2812B_H
#define WS2812B_H

void ws2812b_init(void);
void ws2812b_show(void);

#define numleds 300

uint8_t leds[numleds*3];
uint16_t current;

#endif

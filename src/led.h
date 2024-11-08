#pragma once

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

#define NUM_LEDS 8
#define BRIGHTNESS 0xf0 // brightness byte (0xe0 - 0xff)
#define DEFAULT_GREY 0x08
#define LED_SPI_PORT spi0
#define LED_PIN_SCK 2
#define LED_PIN_TX 3

extern uint8_t led_data[NUM_LEDS][4];

void init_leds(void);
void update_leds(void);
void scroll_down_leds(void);
void set_first_led(uint8_t red, uint8_t green, uint8_t blue);
void set_led(int n, uint8_t red, uint8_t green, uint8_t blue);
void hue2rgb(uint8_t hue, uint8_t *r, uint8_t *g, uint8_t *b);

#ifdef __cplusplus
 }
#endif


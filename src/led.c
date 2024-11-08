#include "led.h"
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

uint8_t led_data[NUM_LEDS][4];

void init_leds(void)
{
    spi_init(LED_SPI_PORT, 5000000); // Hz SPI clock
    gpio_set_function(LED_PIN_SCK, GPIO_FUNC_SPI);  // GP2 as SPI0_SCK
    gpio_set_function(LED_PIN_TX, GPIO_FUNC_SPI);   // GP3 as SPI0_TX

    // Initialize the LED buffer with a gray color
    for (int i = 0; i < NUM_LEDS; i++) {
        led_data[i][0] = BRIGHTNESS; // Brightness byte (0xE0 - 0xFF)
        led_data[i][1] = DEFAULT_GREY;       // Blue (gray color, all channels equal)
        led_data[i][2] = DEFAULT_GREY;       // Green
        led_data[i][3] = DEFAULT_GREY;       // Red
    }
    update_leds();
}

void update_leds(void)
{
    uint8_t start_frame[4] = {0, 0, 0, 0};  // Start frame
    uint8_t end_frame[4] = {0xFF, 0xFF, 0xFF, 0xFF}; // End frame (enough for 8 LEDs)

    spi_write_blocking(LED_SPI_PORT, start_frame, 4);

    // Send LED data
    for (int i = 0; i < NUM_LEDS; i++) {
        spi_write_blocking(LED_SPI_PORT, led_data[i], 4);
    }

    spi_write_blocking(LED_SPI_PORT, end_frame, 4);
}

void scroll_down_leds(void)
{
    // Shift all LEDs down
    for (int i = NUM_LEDS - 1; i > 0; i--) {
        memcpy(led_data[i], led_data[i - 1], 4);
    }

    led_data[0][0] = BRIGHTNESS; // Brightness byte (0xE0 - 0xFF)
    led_data[0][1] = DEFAULT_GREY;       // Blue (gray color, all channels equal)
    led_data[0][2] = DEFAULT_GREY;       // Green
    led_data[0][3] = DEFAULT_GREY;       // Red
}

void set_first_led(uint8_t red, uint8_t green, uint8_t blue)
{
    set_led(0, red, green, blue);
}

void set_led(int n, uint8_t red, uint8_t green, uint8_t blue)
{
    if (n >= NUM_LEDS) return;
    led_data[n][0] = BRIGHTNESS; // Keep the same brightness
    led_data[n][1] = blue;       // Set Blue
    led_data[n][2] = green;      // Set Green
    led_data[n][3] = red;        // Set Red
}

void hue2rgb(uint8_t hue, uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t region = hue / 43;          // Divide 256 into 6 regions, approx 43 each
    uint8_t remainder = (hue % 43) * 6; // Map remainder to [0, 255] scale

    switch (region) {
        case 0: *r = 255; *g = remainder; *b = 0; break;          // Red to Yellow
        case 1: *r = 255 - remainder; *g = 255; *b = 0; break;    // Yellow to Green
        case 2: *r = 0; *g = 255; *b = remainder; break;          // Green to Cyan
        case 3: *r = 0; *g = 255 - remainder; *b = 255; break;    // Cyan to Blue
        case 4: *r = remainder; *g = 0; *b = 255; break;          // Blue to Magenta
        case 5: *r = 255; *g = 0; *b = 255 - remainder; break;    // Magenta to Red
    }
}
#pragma once

#define SQUIM_PORT 11000

#define WIRELESS_ENCRYPTION CYW43_AUTH_WPA2_AES_PSK
#define BUFFER_SIZE 64
#define BUFFER_WATERMARK_50 (BUFFER_SIZE/2)

typedef struct {
    uint8_t data[BUFFER_SIZE][TLV_MAX_PACKET_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} circular_buffer;

extern circular_buffer  udp_buffer;

int init_wifi_stuff(void);
void close_wifi_stuff(void);

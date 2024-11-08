#pragma once

#define SQUIM_PORT 11000

#define WIRELESS_ENCRYPTION CYW43_AUTH_WPA2_AES_PSK
#define UDP_BUFFER_SIZE 64
#define UDP_BUFFER_WATERMARK_50 (UDP_BUFFER_SIZE/2)

typedef struct {
    uint8_t data[UDP_BUFFER_SIZE][TLV_MAX_PACKET_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} circular_buffer8;

extern circular_buffer8  udp_buffer;

int init_wifi_stuff(void);
void close_wifi_stuff(void);

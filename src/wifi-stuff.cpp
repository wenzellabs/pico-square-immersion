#include <stdio.h>
#include <tlv.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/critical_section.h"
#include <lwip/inet.h>
#include "lwip/timeouts.h"
#include <wifi-stuff.hpp>


static struct udp_pcb *pcb = NULL;

circular_buffer udp_buffer = { .data = {0}, .head = 0, .tail = 0, .count = 0 };

// fwd declaration fo udp rx callback
void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

int init_wifi_stuff(void)
{
    if (cyw43_arch_init()) {
        printf("failed to init WiFi\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, WIRELESS_ENCRYPTION, 10000)) {
        printf("WiFi connect failed\n");
        cyw43_arch_deinit();
        return -1;
    }

    pcb = udp_new();
    if (!pcb) {
        printf("failed to create PCB\n");
        return -1;
    }

    // Bind the PCB to any IP address on SQUIM port
    err_t err = udp_bind(pcb, IP_ADDR_ANY, SQUIM_PORT);
    if (err != ERR_OK) {
        printf("failed to bind PCB: %d\n", err);
        udp_remove(pcb);
        return -1;
    }

    // Set the receive callback function
    udp_recv(pcb, udp_receive_callback, NULL);

    printf("listening for broadcast messages on %s:%d...\n", ipaddr_ntoa(&cyw43_state.netif[0].ip_addr), SQUIM_PORT);

    return 0;
}

#define INFO_WATERMARK      0x01
#define WARNING_OVERFLOW    0x02
#define WARNING_LONG        0x04
#define WARNING_UNICAST     0x08

void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    // no gcc warnings for unused params
    (void)arg;
    (void)pcb;
    (void)addr;
    (void)port;
    // disable interrupts
    uint32_t irq_state = save_and_disable_interrupts();
    uint8_t *packet = (uint8_t *)p->payload;
    uint16_t length = p->len;

    uint8_t warning = 0;   // avoid printf with interrupts disabled
    uint8_t watermark = 0;

    // firewall a) only allow broadcast
    if (ip_addr_isbroadcast(ip_current_dest_addr(), netif_default))
    {
        // firewall b) only allow small packets, TLV_MAX_PACKET_SIZE
        if (length <= TLV_MAX_PACKET_SIZE)
        {
            if (udp_buffer.count < BUFFER_SIZE) {
                // all firewalls passed, and still a free slot in our circlular buffer
                memcpy(udp_buffer.data[udp_buffer.head], packet, length);
                udp_buffer.head = (udp_buffer.head + 1) % BUFFER_SIZE;
                udp_buffer.count++;
                if (udp_buffer.count > BUFFER_WATERMARK_50)
                {
                    warning |= INFO_WATERMARK;
                    watermark = udp_buffer.count;
                }
            } else {
                warning |= WARNING_OVERFLOW;
            }
        }else{
            warning |= WARNING_LONG;
        }
    }else{
        warning |= WARNING_UNICAST;
    }
    pbuf_free(p);

    restore_interrupts(irq_state);

    // log with interrupts enabled
    if (warning & INFO_WATERMARK)
    {
        printf("INFO: buffer 50%% watermark reached (%d/%d)\n", watermark, BUFFER_SIZE);
    }
    if (warning & WARNING_OVERFLOW)
    {
        printf("WARNING: overflow, packet dropped\n");
    }
    if (warning & WARNING_LONG)
    {
        printf("WARNING: too long, packet dropped\n");
    }
    if (warning & WARNING_UNICAST)
    {
        printf("WARNING: non-broadcast, packet dropped\n");
    }
}

void close_wifi_stuff(void)
{
    udp_remove(pcb);
    cyw43_arch_deinit();
}
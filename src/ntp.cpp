#include "ntp.hpp"
#include <stdio.h>
#include <cstring>

NTP_client::NTP_client() : _state(STATE_BOOTUP), timeout_alarm(0) {
    // Convert the server IP address string to `ip_addr_t`
    ipaddr_aton(NTP_SERVER_IP, &ntp_server_address);
    
    // Allocate a new UDP PCB for NTP communication
    ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (!ntp_pcb) {
        printf("Failed to create UDP PCB\n");
        return;
    }

    // Set up the receive callback for UDP packets
    udp_recv(ntp_pcb, ntp_recv, this);

    next_update_time = make_timeout_time_ms(NTP_UPDATE_PERIOD);
}

NTP_client::~NTP_client()
{
    // Clean up the UDP PCB
    if (ntp_pcb) {
        udp_remove(ntp_pcb);
    }
    if (timeout_alarm > 0) {
        cancel_alarm(timeout_alarm);
    }
}

// Main function to be called frequently
void NTP_client::update_time()
{
    if (_state == STATE_BOOTUP)
    {
        static int count = 3;
        if (count-- > 0) send_request();
    }
    if (absolute_time_diff_us(get_absolute_time(), next_update_time) <= 0 && _state == STATE_IDLE)
    {
        send_request();
    }
}

void NTP_client::send_request()
{
    // Prepare NTP request payload (48 bytes, most set to 0)
    cyw43_arch_lwip_begin();
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 48, PBUF_RAM);
    if (!p) {
        printf("Failed to allocate buffer for NTP request\n");
        return;
    }
    uint8_t *req = (uint8_t *) p->payload;
    memset(req, 0, 48);
    req[0] = 0x1b;  // NTP request header

    // Send the request to the NTP server
    udp_sendto(ntp_pcb, p, &ntp_server_address, NTP_SERVER_PORT);

    pbuf_free(p);
    cyw43_arch_lwip_end();

    if (_state == STATE_IDLE) _state = STATE_REQUEST_SENT;
    timeout_alarm = add_alarm_in_ms(NTP_TIMEOUT_PERIOD, ntp_timeout_handler, this, true);
}

void NTP_client::process_response(struct pbuf *p)
{
    int64_t uptime_us = get_absolute_time();

    uint8_t mode = pbuf_get_at(p, 0) & 0x7;
    uint8_t stratum = pbuf_get_at(p, 1);

    if (p->tot_len == 48 && mode == 0x4 && stratum != 0) {
        uint8_t seconds_buf[4] = {0};
        pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
        uint32_t seconds_since_1900 = seconds_buf[0] << 24 | seconds_buf[1] << 16 | seconds_buf[2] << 8 | seconds_buf[3];

        // Extract fractional seconds from bytes 44 to 47
        uint8_t fractional_buf[4] = {0};
        pbuf_copy_partial(p, fractional_buf, sizeof(fractional_buf), 44);
        uint32_t fractional_seconds = fractional_buf[0] << 24 | fractional_buf[1] << 16 | fractional_buf[2] << 8 | fractional_buf[3];
        // Convert fractional seconds (32-bit) to nanoseconds
        uint32_t microseconds = (fractional_seconds * 1e6) / (1ULL << 32);

        int64_t now = (1000000LL * (int64_t)seconds_since_1900 + (int64_t)microseconds);
        int64_t old_powerup_time = powerup_time;
        powerup_time = now - uptime_us;
        if (_state != STATE_BOOTUP) // only 1st response sets, others skew
        {
            if (old_powerup_time - (int64_t)powerup_time < -NTP_SKEW_LIMIT)
            {
                powerup_time = old_powerup_time + NTP_SKEW_LIMIT;
            }else{
                if (old_powerup_time - (int64_t)powerup_time >  NTP_SKEW_LIMIT)
                {
                    powerup_time = old_powerup_time - NTP_SKEW_LIMIT;
                }
            }
        }
        printf("powerup_time %lld (delta %lld)\n", powerup_time, old_powerup_time-powerup_time);

        // and seed the prng
        srand(now^rand());

    } else {
        printf("Invalid NTP response\n");
    }

    _state = STATE_IDLE;
    next_update_time = make_timeout_time_ms(NTP_UPDATE_PERIOD);

    // Cancel timeout alarm
    if (timeout_alarm > 0) {
        cancel_alarm(timeout_alarm);
        timeout_alarm = 0;
    }
}

// Static callback for receiving UDP data
void NTP_client::ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    // no gcc warnings for unused params
    (void)pcb;

    NTP_client *client = (NTP_client *)arg;
    if (ip_addr_cmp(addr, &client->ntp_server_address) && port == NTP_SERVER_PORT) {
        client->process_response(p);
    }
    pbuf_free(p);
}

// Static timeout handler if no response is received
int64_t NTP_client::ntp_timeout_handler(alarm_id_t id, void *user_data)
{
    // no gcc warnings for unused params
    (void)id;

    NTP_client *client = (NTP_client *)user_data;
    if (client->_state == STATE_REQUEST_SENT)
    {
        printf("NTP request timed out\n");
        client->_state = STATE_IDLE;
        client->next_update_time = make_timeout_time_ms(NTP_UPDATE_PERIOD);
    }
    return 0;
}

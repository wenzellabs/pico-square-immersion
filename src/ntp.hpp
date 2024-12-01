#pragma once

#include <string>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"


//#define NTP_SERVER_IP "77.48.28.248"    // pool.ntp.org // went down 2024-09-18
#define NTP_SERVER_IP "162.159.200.123" // pool.ntp.org
//#define NTP_SERVER_IP "162.159.200.1"   // pool.ntp.org
//#define NTP_SERVER_IP "213.192.54.169"  // pool.ntp.org

#if 0
even more
0.de.pool.ntp.org has address 167.235.76.111
0.de.pool.ntp.org has address 80.153.195.191
0.de.pool.ntp.org has address 217.144.138.234
0.de.pool.ntp.org has address 178.63.52.50
1.de.pool.ntp.org has address 185.41.106.152
1.de.pool.ntp.org has address 148.251.5.46
1.de.pool.ntp.org has address 131.234.220.232
1.de.pool.ntp.org has address 167.235.69.67
2.de.pool.ntp.org has address 185.163.116.98
2.de.pool.ntp.org has address 85.215.166.214
2.de.pool.ntp.org has address 207.180.217.145
2.de.pool.ntp.org has address 168.119.239.121
3.de.pool.ntp.org has address 116.202.118.202
3.de.pool.ntp.org has address 31.209.85.243
3.de.pool.ntp.org has address 129.250.35.250
3.de.pool.ntp.org has address 185.252.140.125
ptbtime1.ptb.de has address 192.53.103.108
ptbtime2.ptb.de has address 192.53.103.104
ptbtime3.ptb.de has address 192.53.103.103
ptbtime4.ptb.de has address 194.94.95.123
time.google.com has address 216.239.35.12
time.google.com has address 216.239.35.4
time.google.com has address 216.239.35.8
time.google.com has address 216.239.35.0
#endif

#define NTP_SERVER_PORT 123

class NTP_client {
public:
    NTP_client();
    ~NTP_client();

    void update_time();  // call from time to time to manage NTP requests and responses
    uint64_t powerup_time;
    bool run;
private:
    static const uint32_t NTP_UPDATE_PERIOD         =  30 * 1000;  // 30 seconds
    static const uint32_t NTP_UPDATE_PERIOD_BOOTUP  =   1 * 1000;  //  1 second
    static const uint32_t NTP_TIMEOUT_PERIOD        =   5 * 1000;  //  5 seconds
    static const  int64_t NTP_SKEW_LIMIT            =   5 * 1000;  //  5 microseconds

    enum State {
        STATE_BOOTUP,
        STATE_IDLE,
        STATE_REQUEST_SENT
    };

    void send_request();            // Sends an NTP request
    void process_response(struct pbuf *p);  // Processes an NTP response
    static void ntp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
    static int64_t ntp_timeout_handler(alarm_id_t id, void *user_data);

    ip_addr_t ntp_server_address;
    struct udp_pcb *ntp_pcb;

    State _state;
    absolute_time_t next_update_time;
    alarm_id_t timeout_alarm;
};

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <generated_tlv.h>

#define TLV_CB_FUNC void (*func)(tlv_packet_t *)
#define TLV_MAX_CALLBACKS 16

#define TLV_HEADER_LENGTH 2 /* bytes */

typedef struct tlv_header_s
{
    uint8_t type; // see generated_tlv.h
    uint8_t len;  // length of the packet == (header + payload)
} __attribute__((__packed__)) tlv_header_t;

#define TLV_MAX_PACKET_SIZE 255 /* LoRa has it, 8bit length field */
#define TLV_MAX_PAYLOAD_SIZE (TLV_MAX_PACKET_SIZE - TLV_HEADER_LENGTH)


typedef struct tlv_packet_s
{
    union {
        uint8_t packet[TLV_MAX_PACKET_SIZE];
        struct
        {
            tlv_header_t header;
            uint8_t payload[TLV_MAX_PAYLOAD_SIZE];
        };
    };
} __attribute__((__packed__)) tlv_packet_t;

#ifdef __cplusplus
}
#endif


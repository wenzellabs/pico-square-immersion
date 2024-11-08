#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define TLV_HEADER_LENGTH 2 /* bytes */

typedef struct tlv_header_s
{
    uint8_t type; // see tlv_type.h
    uint8_t len;  // length of the packet == (header + payload)
} __attribute__((__packed__)) tlv_header_t;

#ifdef __cplusplus
}
#endif

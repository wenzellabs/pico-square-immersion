#pragma once

#include <tlv_type_payload.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define TLV_TYPE_RESERVED       0x00 // must not be used
#define TLV_TYPE_TIME           0x01

#define TLV_TYPE_NOTE_ON        0x11
#define TLV_TYPE_NOTE_OFF       0x12

#define TLV_TYPE_BEAT           0x20
#define TLV_TYPE_START          0x21

#ifdef __cplusplus
}
#endif

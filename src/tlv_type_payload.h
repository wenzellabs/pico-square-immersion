#pragma once

#include <stdint.h>

// TLV_TYPE_TIME
typedef struct tlv_type_time_s
{
    uint32_t utc; // seconds in the epoch
    uint32_t ms;  // ms after utc
} __attribute__((__packed__)) tlv_type_time_t;

// TLV_TYPE_NOTE_ON
typedef struct tlv_type_note_on_s
{
    uint32_t utc;       // seconds in the epoch
    uint32_t ms;        // ms after utc
    uint8_t  note;      // MIDI encoded note (0-127, A=440Hz=69)
    uint8_t  ch;        // MIDI encoded channel (1-16)
    uint8_t  velocity;  // key velocity
} __attribute__((__packed__)) tlv_type_note_on_t;

// TLV_TYPE_NOTE_OFF
typedef struct tlv_type_note_off_s
{
    uint32_t utc;       // seconds in the epoch
    uint32_t ms;        // ms after utc
    uint8_t  note;      // MIDI encoded note (0-127, A=440Hz=69)
    uint8_t  ch;        // MIDI encoded channel (1-16)
    uint8_t  velocity;  // key velocity
} __attribute__((__packed__)) tlv_type_note_off_t;

// TLV_TYPE_BEAT
typedef struct tlv_type_beat_s
{
    uint8_t  bpm;       // MIDI beat (0xf8 / 24)
    uint32_t count;     // beat count into the piece
} __attribute__((__packed__)) tlv_type_beat_t;

// TLV_TYPE_START
typedef struct tlv_type_start_s
{
    uint32_t utc;       // seconds in the epoch
    uint32_t ms;        // ms after utc
    uint8_t  bpm;       // MIDI beat (0xf8 / 24)
    uint32_t count;     // beat count into the piece
} __attribute__((__packed__)) tlv_type_start_t;


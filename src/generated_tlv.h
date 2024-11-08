/* this stuff is automagically generated from */
/* tlv_generator.py - do not edit */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define PACKED __attribute__((__packed__))


#define TLV_TYPE_TIME 0x01
typedef struct tlv_type_time_s
{
    uint64_t us_since_1900;
} PACKED tlv_type_time_t;

#define TLV_TYPE_NOTE_ON 0x11
typedef struct tlv_type_note_on_s
{
    uint64_t us_since_1900;
    uint8_t note;
    uint8_t channel;
    uint8_t velocity;
} PACKED tlv_type_note_on_t;

#define TLV_TYPE_NOTE_OFF 0x12
typedef struct tlv_type_note_off_s
{
    uint64_t us_since_1900;
    uint8_t note;
    uint8_t channel;
    uint8_t velocity;
} PACKED tlv_type_note_off_t;

#define TLV_TYPE_PANIC 0x1f
typedef struct tlv_type_panic_s
{
} PACKED tlv_type_panic_t;

#define TLV_TYPE_BEAT 0x20
typedef struct tlv_type_beat_s
{
    uint8_t bpm;
    uint32_t count;
} PACKED tlv_type_beat_t;

#define TLV_TYPE_START 0x21
typedef struct tlv_type_start_s
{
    uint64_t us_since_1900;
    uint8_t bpm;
    uint32_t count;
} PACKED tlv_type_start_t;

#define TLV_TYPE_KEY_NOTES 0x30
typedef struct tlv_type_key_notes_s
{
    uint8_t root;
    uint8_t third;
    uint8_t fifth;
    uint8_t seventh;
    uint8_t ninth;
    uint8_t eleventh;
    uint8_t thirteenth;
} PACKED tlv_type_key_notes_t;

#define TLV_TYPE_CHORD 0x31
typedef struct tlv_type_chord_s
{
    uint64_t on;
    uint64_t off;
    uint8_t note[16];
} PACKED tlv_type_chord_t;

#define TLV_TYPE_SCALE 0x32
typedef enum
{
    SCALE_TYPE_MAJOR = 1,
    SCALE_TYPE_MINOR = 2,
    SCALE_TYPE_HARMONIC_MINOR = 3,
    SCALE_TYPE_MELODIC_MINOR = 4,
    SCALE_TYPE_DORIAN = 5,
    SCALE_TYPE_PHRYGIAN = 6,
    SCALE_TYPE_LYDIAN = 7,
    SCALE_TYPE_MIXOLYDIAN = 8,
    SCALE_TYPE_LOCRIAN = 9,
    SCALE_TYPE_MAJOR_PENTATONIC = 10,
    SCALE_TYPE_MINOR_PENTATONIC = 11,
    SCALE_TYPE_BLUES_MINOR = 12,
    SCALE_TYPE_BLUES_MAJOR = 13,
    SCALE_TYPE_WHOLE_TONE = 14,
    SCALE_TYPE_CHROMATIC = 15,
} tlv_enum_scale_type_t;
typedef struct tlv_type_scale_s
{
    uint8_t root;
    tlv_enum_scale_type_t scale_type;
} PACKED tlv_type_scale_t;


#ifdef __cplusplus
}
#endif

/* end of automagically generated code */

#pragma once

#include <midi-state-machine.hpp>
#include <TLV_registry.hpp>
#include <ntp.hpp>

// TLV_TYPE_NOTE_ON or TLV_TYPE_NOTE_OFF
typedef struct tlv_type_note_s
{
    uint64_t us_since_1900;  // microseconds since 1900
    uint8_t  note;           // MIDI encoded note (0-127, A=440Hz=69)
    uint8_t  channel;        // MIDI encoded channel (1-16)
    uint8_t  velocity;       // key velocity
    uint8_t  onoff;          // 1=note_on; 0=note_off
} __attribute__((__packed__)) tlv_type_note_t;

#define NOTE_BUFFER_SIZE 1024
#define NOTE_BUFFER_WATERMARK_50 (NOTE_BUFFER_SIZE/2)

typedef struct {
    tlv_type_note_t note[NOTE_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} circular_note_buffer;

class Network_source
{
public:
    Network_source(MIDI_state_machine *const midi_state_machine);
    virtual ~Network_source();

    void rx_task();
    void set_ntp(NTP_client *const ntp);

    bool has_wifi;

private:
    MIDI_state_machine *const _midi_state_machine;
    NTP_client *_ntp;
    TLV_registry _tlv_reg;
    circular_note_buffer note_buffer;

    uint64_t _start;
    uint8_t _bpm;
    uint64_t _beat;

    uint8_t _root;
    uint8_t _temp_root;
    uint8_t _offbeat;
    uint8_t _scale_type;

    char _artist[235];
    char _title[235];

    void init_tlv(TLV_registry& registry);

    void process_udp_data();
    void play_note();
    void enqueue_note(tlv_packet_t *tp, uint8_t onoff);

    void tlv_time(tlv_packet_t *tp);
    void note_on(tlv_packet_t *tp);
    void note_off(tlv_packet_t *tp);
    void note_on_off(tlv_packet_t *tp);
    void beat(tlv_packet_t *tp);
    void start(tlv_packet_t *tp);
    void panic(tlv_packet_t *tp);
    void scale(tlv_packet_t *tp);
    void chord(tlv_packet_t *tp);
    void artist(tlv_packet_t *tp);
    void title(tlv_packet_t *tp);

    void chord_flavour0(tlv_packet_t *tp);
    void chord_flavour1(tlv_packet_t *tp);
    void chord_flavour2(tlv_packet_t *tp);
    void chord_flavour3(tlv_packet_t *tp);
    void double_chord_down(tlv_packet_t *tp);
    void chord_flavour4(tlv_packet_t *tp);
    void chord_flavour5(tlv_packet_t *tp);
    void sort_notes(tlv_packet_t *tp);
    void chord_flavour6(tlv_packet_t *tp);
    void chord_flavour7(tlv_packet_t *tp);
    void sort_notes_out2in(tlv_packet_t *tp);
    void chord_flavour8(tlv_packet_t *tp);
    void chord_flavour9(tlv_packet_t *tp);

    void chord_flavour000_b0rken(tlv_packet_t *tp);
    void chord_flavour001_b0rken(tlv_packet_t *tp);
};

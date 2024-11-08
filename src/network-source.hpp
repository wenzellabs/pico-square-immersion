#pragma once

#include <midi-state-machine.hpp>
#include <TLV_registry.hpp>

class Network_source
{
public:
    Network_source(MIDI_state_machine *const midi_state_machine);
    virtual ~Network_source();

    void rx_task();

private:
    MIDI_state_machine *const _midi_state_machine;
    TLV_registry _tlv_reg;

    void init_tlv(TLV_registry& registry);

    void process_udp_data();

    void tlv_time(tlv_packet_t *tp);
    void note_on(tlv_packet_t *tp);
    void note_off(tlv_packet_t *tp);
    void beat(tlv_packet_t *tp);
    void start(tlv_packet_t *tp);
};

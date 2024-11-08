#include "pico/critical_section.h"
#include <network-source.hpp>
#include <TLV_registry.hpp>
#include <wifi-stuff.hpp>
#include <stdio.h>
#include <tlv.h>


Network_source::Network_source(MIDI_state_machine *const midi_state_machine) :
    _midi_state_machine(midi_state_machine)
{
  init_tlv(_tlv_reg);
  int ret = init_wifi_stuff();
  if (ret != 0)
  {
    printf("no WiFi\n");
  }
}

Network_source::~Network_source()
{
  close_wifi_stuff();
}

void Network_source::rx_task()
{
    while (udp_buffer.count > 0) {
        uint8_t *packet = udp_buffer.data[udp_buffer.tail];

        // Call run_callbacks on the _tlv_reg instance
        _tlv_reg.run_callbacks((tlv_packet_t *)packet);

        udp_buffer.tail = (udp_buffer.tail + 1) % BUFFER_SIZE;
        uint32_t irq_state = save_and_disable_interrupts();
        udp_buffer.count--;
        restore_interrupts(irq_state);
    }
}

void Network_source::tlv_time(tlv_packet_t *tp)
{
  tlv_type_time_t *p = (tlv_type_time_t *)tp->payload;
  p->utc += p->ms / 1000;
  p->ms = p->ms % 1000;
  printf("time is %ld.%03ld\n", p->utc, p->ms);
}

void Network_source::note_on(tlv_packet_t *tp)
{
  tlv_type_note_on_t *p = (tlv_type_note_on_t *)tp->payload;

  uint8_t packet[4];
  packet[0] = 0x09;  // note on
  packet[1] = p->ch; // channel
  packet[2] = p->note;
  packet[3] = p->velocity;

  _midi_state_machine->consume_event_packet(packet);

  //printf("note on  %d velo %d ch %d\n", p->note, p->velocity, p->ch);
}

void Network_source::note_off(tlv_packet_t *tp)
{
  tlv_type_note_off_t *p = (tlv_type_note_off_t *)tp->payload;

  uint8_t packet[4];
  packet[0] = 0x08;  // note off
  packet[1] = p->ch; // channel
  packet[2] = p->note;
  packet[3] = p->velocity;

  _midi_state_machine->consume_event_packet(packet);

  //printf("note off %d velo %d ch %d\n", p->note, p->velocity, p->ch);
}

void Network_source::beat(tlv_packet_t *tp)
{
  static uint32_t last_count = 0xffffffff;
  tlv_type_beat_t *p = (tlv_type_beat_t *)tp->payload;
  if (p->count != last_count + 1)
  {
    printf("WARNING: beat jumped from %ld to %ld\n", last_count, p->count);
  }
  last_count = p->count;
  printf("bpm %d, beat %ld\n", p->bpm, p->count);
}

void Network_source::start(tlv_packet_t *tp)
{
  tlv_type_start_t *p = (tlv_type_start_t *)tp->payload;
  p->utc += p->ms / 1000; // FIXME refactor, see tlv_time
  p->ms = p->ms % 1000;
  printf("start will be at %ld.%03ld with bpm %d, beat %ld\n", p->utc, p->ms, p->bpm, p->count);
}

void Network_source::init_tlv(TLV_registry& registry) {
    registry.set_callback(TLV_TYPE_TIME, [this](tlv_packet_t *p) { this->tlv_time(p); });
    registry.set_callback(TLV_TYPE_NOTE_ON, [this](tlv_packet_t *p) { this->note_on(p); });
    registry.set_callback(TLV_TYPE_NOTE_OFF, [this](tlv_packet_t *p) { this->note_off(p); });
    registry.set_callback(TLV_TYPE_BEAT, [this](tlv_packet_t *p) { this->beat(p); });
    registry.set_callback(TLV_TYPE_START, [this](tlv_packet_t *p) { this->start(p); });
}

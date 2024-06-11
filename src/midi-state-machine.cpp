/*
 * MIDI State Machine of Simple Stupid Synthesizer
 *
 * Copyright (C) 2023 Jürgen Reuter
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * As a special exception to the GNU General Public License, if you
 * distribute this file as part of a program that contains a
 * configuration script generated by Autoconf, you may include it
 * under the same distribution terms that you use for the rest of that
 * program.
 *
 * For updates and more info or contacting the author, visit:
 * <https://github.com/soundpaint>
 *
 * Author's web site: www.juergen-reuter.de
 */

#include "midi-state-machine.hpp"
#include <math.h>
#include "pico/stdlib.h"
#include "bsp/board.h"

const double
MIDI_state_machine::OCTAVE_FREQ_RATIO = 2.0;

const uint8_t
MIDI_state_machine::NOTES_PER_OCTAVE = 12;

const double
MIDI_state_machine::A4_FREQ = 440.0;

const uint8_t
MIDI_state_machine::A4_NOTE_NUMBER = 69;

const uint8_t
MIDI_state_machine::COUNT_HEADROOM_BITS = 0x8;

const uint8_t
MIDI_state_machine::CHANNEL_PROGRAM_INIT = 0x00;

const uint8_t
MIDI_state_machine::CHANNEL_PRESSURE_INIT = 0x7f;

const uint16_t
MIDI_state_machine::CHANNEL_PITCH_BEND_INIT = 0x2000;

const uint32_t
MIDI_state_machine::COUNT_INC = ((long)1u) << COUNT_HEADROOM_BITS;

MIDI_state_machine::
MIDI_state_machine(IMidi_event_listener *listener,
                   const uint32_t sample_freq,
                   const uint8_t gpio_pin_activity_indicator) :
  _gpio_pin_activity_indicator(gpio_pin_activity_indicator)
{
  _listener = listener;
  init(sample_freq, gpio_pin_activity_indicator);
}

MIDI_state_machine::~MIDI_state_machine()
{
  _listener = 0;
}

void
MIDI_state_machine::osc_init(const uint32_t sample_freq)
{
  const double count_inc = COUNT_INC;
  const double log_note_step_ratio = log(OCTAVE_FREQ_RATIO) / NOTES_PER_OCTAVE;
  for (uint8_t osc = 0; osc < NUM_KEYS; osc++) {
    const double osc_freq =
      A4_FREQ * exp((osc - (double)A4_NOTE_NUMBER) * log_note_step_ratio);
    // half (0.5) inc, since square wave elongation toggles twice per period
    const uint32_t count_wrap =
      round(0.5 * count_inc * sample_freq / osc_freq);
    MIDI_state_machine::osc_status_t *osc_status = &_osc_statuses[osc];
    osc_status->count_wrap = count_wrap;
    osc_status->count = 0;
    osc_status->velocity = 0;
    osc_status->elongation = 0;
  }
}

void
MIDI_state_machine::channels_init()
{
  _cumulated_channel_pressure = 0;
  for (uint8_t channel = 0; channel < NUM_CHN; channel++) {
    channel_status_t *channel_status = &_midi_status.channel_status[channel];
    channel_status->program = CHANNEL_PROGRAM_INIT;
    channel_status->channel_pressure = CHANNEL_PRESSURE_INIT;
    _cumulated_channel_pressure += CHANNEL_PRESSURE_INIT;
    channel_status->pitch_bend = CHANNEL_PITCH_BEND_INIT;
    for (uint8_t key = 0; key < NUM_KEYS; key++) {
      key_status_t *key_status = &channel_status->key_status[key];
      key_status->velocity = 0;
    }
  }
}

void
MIDI_state_machine::led_init(const uint8_t gpio_pin_activity_indicator)
{
  gpio_init(gpio_pin_activity_indicator);
  gpio_set_dir(gpio_pin_activity_indicator, GPIO_OUT);
}

void
MIDI_state_machine::init(const uint32_t sample_freq,
                         const uint8_t gpio_pin_activity_indicator)
{
  _timestamp_active_sensing = time_us_64();
  osc_init(sample_freq);
  channels_init();
  led_init(gpio_pin_activity_indicator);
  board_init();
  tusb_init();
}

MIDI_state_machine::osc_status_t *
MIDI_state_machine::get_osc_statuses()
{
  return &_osc_statuses[0];
}

void
MIDI_state_machine::add_to_osc_status(const uint8_t osc,
                                      const int8_t delta_velocity)
{
  osc_status_t *osc_status = &_osc_statuses[osc];
  osc_status->velocity += delta_velocity;
  const int16_t elongation = osc_status->elongation;
  if (elongation > 0) {
    osc_status->elongation += delta_velocity;
  } else if (elongation < 0) {
    osc_status->elongation -= delta_velocity;
  } else {
    osc_status->elongation = delta_velocity;
  }
}

void
MIDI_state_machine::set_note_velocity(const uint8_t channel, const uint8_t key,
                                      const uint8_t velocity)
{
  channel_status_t *channel_status = &_midi_status.channel_status[channel];
  key_status_t *key_status = &channel_status->key_status[key];
  const uint8_t prev_velocity = key_status->velocity;
  key_status->velocity = velocity;
  add_to_osc_status(key, velocity - prev_velocity);
  gpio_put(_gpio_pin_activity_indicator, velocity > 0 ? 1 : 0);
}

void
MIDI_state_machine::handle_all_sound_off(const uint8_t channel)
{
  for (uint8_t key = 0; key < NUM_KEYS; key++) {
    set_note_velocity(channel, key, 0);
  }
}

void
MIDI_state_machine::handle_all_notes_off(const uint8_t channel)
{
  /*
   * Since by now, we do not implement pedal control, all notes off is
   * currently identical with all sound off.
   */
  handle_all_sound_off(channel);
}

void
MIDI_state_machine::handle_control_change(const uint8_t channel,
                                          const uint8_t controller,
                                          const uint8_t)
{
  if (controller < 0xf8) {
    // ordinary control change: not implemented => ignore
    return;
  }
  // controllers 120-127: channel mode message
  switch (controller) {
  case 0xf8:
    handle_all_sound_off(channel);
    break;
  case 0xfb:
    handle_all_notes_off(channel);
    break;
  case 0xfc:
  case 0xfd:
  case 0xfe:
  case 0xff:
    /*
     * Not implemented.  But anyway, also turn off all notes, as the
     * specification requires.
     */
    handle_all_notes_off(channel);
    break;
  default:
    // not implemented => ignore
    break;
  }
}

void
MIDI_state_machine::set_program_change(const uint8_t channel,
                                       const uint8_t program)
{
  channel_status_t *channel_status = &_midi_status.channel_status[channel];
  channel_status->program = program;
  // track program changes, but otherwise ignore them for now
}

void
MIDI_state_machine::set_pitch_bend_change(const uint8_t channel,
                                          const uint8_t lsb, const uint8_t msb)
{
  channel_status_t *channel_status = &_midi_status.channel_status[channel];
  channel_status->pitch_bend = ((msb & 0x7f) << 7) | (lsb & 0x7f);
  // track pitch bend changes, but otherwise ignore them for now
}

uint16_t
MIDI_state_machine::get_cumulated_channel_pressure()
{
  return _cumulated_channel_pressure;
}

void
MIDI_state_machine::set_channel_pressure(const uint8_t channel,
                                         const uint8_t pressure)
{
  channel_status_t *channel_status = &_midi_status.channel_status[channel];
  const int8_t delta_pressure = pressure - channel_status->channel_pressure;
  channel_status->channel_pressure = pressure;
  _cumulated_channel_pressure += delta_pressure;
}

/*
 * For the structure of event packets, see Sect. 4, "USB-MIDI Event
 * Packets" in the "Universal Serial Bus Device Class Definition for
 * MIDI Devices" at https://usb.org/sites/default/files/midi10.pdf.
 */
void
MIDI_state_machine::consume_event_packet(const uint8_t *event_packet)
{
  const uint8_t code_index_number = event_packet[0] & 0xf;
  switch (code_index_number) {
  case 0x8:
    {
      // note off
      const uint8_t channel = event_packet[1] & 0xf;
      const uint8_t key = event_packet[2] & 0x7f;
      set_note_velocity(channel, key, 0);
      break;
    }
  case 0x9:
    {
      // note on
      const uint8_t channel = event_packet[1] & 0xf;
      const uint8_t key = event_packet[2] & 0x7f;
      const uint8_t velocity = event_packet[3] & 0x7f;
      set_note_velocity(channel, key, velocity);
      break;
    }
  case 0xa:
    {
      // polyphonic key pressure
      const uint8_t channel = event_packet[1] & 0xf;
      const uint8_t key = event_packet[2] & 0x7f;
      const uint8_t velocity = event_packet[3] & 0x7f;
      set_note_velocity(channel, key, velocity);
      break;
    }
  case 0xb:
    {
      // control change, including channel mode message
      const uint8_t channel = event_packet[1] & 0xf;
      const uint8_t controller = event_packet[2] & 0x7f;
      const uint8_t value = event_packet[3] & 0x7f;
      handle_control_change(channel, controller, value);
      break;
    }
  case 0xc:
    {
      // program change
      const uint8_t channel = event_packet[1] & 0xf;
      const uint8_t program = event_packet[2] & 0x7f;
      set_program_change(channel, program);
      break;
    }
  case 0xd:
    {
      // channel pressure
      const uint8_t channel = event_packet[1] & 0xf;
      const uint8_t velocity = event_packet[2] & 0x7f;
      set_channel_pressure(channel, velocity);
      break;
    }
  case 0xe:
    {
      // pitch bend change
      const uint8_t channel = event_packet[1] & 0xf;
      const uint8_t lsb = event_packet[2] & 0x7f;
      const uint8_t msb = event_packet[3] & 0x7f;
      set_pitch_bend_change(channel, lsb, msb);
      break;
    }
  default:
    {
      // not implemented => ignore
      break;
    }
  }
}

void
MIDI_state_machine::rx_task()
{
  tud_task();
  if (!tud_midi_mounted()) {
    return;
  }
  while (tud_midi_available()) {
    /*
     * For the structure of the 4-byte packets, see "USB_MIDI Event
     * Packets" in: USB device class definition
     * (usb.org/sites/default/files/midi10.pdf), page 16.
     */
    uint8_t event_packet[4];
    if (tud_midi_packet_read(event_packet)) {
      consume_event_packet(&event_packet[0]);
    }
  }
}

void
MIDI_state_machine::produce_tx_data(uint8_t *buffer,
                                    __unused const size_t max_buffer_size,
                                    size_t *const buffer_size)
{
  /* when deadline for next active sensing has expired, produce a
     packet containing a sensive acting MIDI code */
  const uint64_t timestamp_now = time_us_64();
  if (timestamp_now - _timestamp_active_sensing > 300000) {
    _timestamp_active_sensing += 300000;
    buffer[0] = 0xFE; // MIDI code for active sensing
    *buffer_size = 1;
  } else {
    *buffer_size = 0;
  }
}

void
MIDI_state_machine::tx_task()
{
  static uint8_t tx_data_buffer[3];
  size_t buffer_size;
  produce_tx_data(tx_data_buffer, sizeof(tx_data_buffer), &buffer_size);
  tud_midi_stream_write(0, tx_data_buffer, buffer_size);
}

/*
 * Local variables:
 *   mode: c++
 *  coding: utf-8
 * End:
 */

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

#ifndef MIDI_STATE_MACHINE_HPP
#define MIDI_STATE_MACHINE_HPP

#include <ctime>

class MIDI_state_machine {
public:
  static const uint8_t NUM_KEYS = 0x80;
  static const uint8_t NUM_CHN = 0x10;
  static const uint32_t COUNT_INC;
  typedef struct {
    uint32_t count_wrap;
    uint32_t count;
    uint16_t velocity;
    int16_t elongation;
  } osc_status_t;
  typedef struct {
    uint8_t velocity;
  } key_status_t;
  typedef struct {
    key_status_t key_status[NUM_KEYS];
    uint8_t program;
    uint16_t pitch_bend;
    uint8_t channel_pressure;
  } channel_status_t;
  typedef struct {
    channel_status_t channel_status[NUM_CHN];
  } midi_status_t;
  MIDI_state_machine();
  virtual ~MIDI_state_machine();
  void init(const uint32_t sample_freq,
            const uint8_t gpio_pin_activity_indicator);
  osc_status_t *get_osc_statuses();
  uint16_t get_cumulated_channel_pressure();
  void rx_task();
  void tx_task();
private:
  static const double OCTAVE_FREQ_RATIO;
  static const uint8_t NOTES_PER_OCTAVE;
  static const double A4_FREQ; // freqency of concert pitch [Hz]
  static const uint8_t A4_NOTE_NUMBER; // MIDI note number of concert pitch
  static const uint8_t COUNT_HEADROOM_BITS;
  static const uint8_t CHANNEL_PROGRAM_INIT;
  static const uint8_t CHANNEL_PRESSURE_INIT;
  static const uint16_t CHANNEL_PITCH_BEND_INIT;
  uint8_t _gpio_pin_activity_indicator;
  osc_status_t _osc_statuses[NUM_KEYS];
  midi_status_t _midi_status;
  uint16_t _cumulated_channel_pressure;
  uint8_t _skip_count = 0;
  uint8_t _msg_count = 0;
  uint64_t _timestamp_active_sensing;
  void handle_all_sound_off();
  void handle_all_notes_off();
  void handle_control_change(const uint8_t controller, const uint8_t value);
  void set_program_change(const uint8_t channel, const uint8_t program);
  void set_pitch_bend_change(const uint8_t channel, const uint8_t lsb,
                             const uint8_t msb);
  void set_channel_pressure(const uint8_t channel, const uint8_t pressure);
  void osc_init(const uint32_t sample_freq);
  void channels_init();
  void led_init(const uint8_t gpio_pin_activity_indicator);
  void add_to_osc_status(const uint8_t osc, const int8_t delta_velocity);
  void set_note_velocity(const uint8_t channel, const uint8_t key,
                         const uint8_t velocity);
  void consume_event_packet(const uint8_t *event_packet);
  void produce_tx_data(uint8_t *buffer,
                       __unused const size_t max_buffer_size,
                       size_t *const buffer_size);
};

#endif /* MIDI_STATE_MACHINE_HPP */

/*
 * Local variables:
 *   mode: c++
 *  coding: utf-8
 * End:
 */

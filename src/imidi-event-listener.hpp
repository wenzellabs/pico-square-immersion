/*
 * MIDI Event Listener Interface of Simple Stupid Synthesizer
 *
 * Copyright (C) 2024 Jürgen Reuter
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

#ifndef IMIDI_EVENT_LISTENER_HH
#define IMIDI_EVENT_LISTENER_HH

class IMidi_event_listener
{
public:
  virtual void midi_note_off(const uint8_t channel, const uint8_t key,
                             const uint8_t velocity) = 0;
  virtual void midi_note_on(const uint8_t channel, const uint8_t key,
                            const uint8_t velocity) = 0;
  virtual void midi_notes_change_velocity(const uint8_t key,
                                          const int8_t delta_velocity) = 0;
  virtual void midi_polyphonic_pressure(const uint8_t channel,
                                        const uint8_t key,
                                        const uint8_t velocity) = 0;
  virtual void midi_control_change(const uint8_t channel,
                                   const uint8_t controller,
                                   const uint8_t value) = 0;
  virtual void midi_program_change(const uint8_t channel,
                                   const uint8_t program) = 0;
  virtual void midi_channel_pressure(const uint8_t channel,
                                     const uint8_t velocity) = 0;
  virtual void midi_pitch_bend_change(const uint8_t channel,
                                      const uint8_t lsb, const uint8_t msb) = 0;
  virtual void midi_all_sound_off(const uint8_t channel) = 0;
  virtual void midi_reset_all_controllers(const uint8_t channel,
                                          const uint8_t value) = 0;
  virtual void midi_local_control_off(const uint8_t channel) = 0;
  virtual void midi_local_control_on(const uint8_t channel) = 0;
  virtual void midi_all_notes_off(const uint8_t channel) = 0;
  virtual void midi_omni_mode_off(const uint8_t channel) = 0;
  virtual void midi_omni_mode_on(const uint8_t channel) = 0;
  virtual void midi_mono_mode_on(const uint8_t channel,
                                 const uint8_t numberOfChannels) = 0;
  virtual void midi_poly_mode_on(const uint8_t channel) = 0;
  virtual void midi_time_code_quarter_frame(const uint8_t msg_type,
                                            const uint8_t values) = 0;
  virtual void midi_song_position_pointer(const uint8_t lsb,
                                          const uint8_t msb) = 0;
  virtual void midi_song_select(const uint8_t select) = 0;
  virtual void midi_tune_request() = 0;
  virtual void midi_timing_clock() = 0;
  virtual void midi_start() = 0;
  virtual void midi_cont() = 0;
  virtual void midi_stop() = 0;
  virtual void midi_active_sensing() = 0;
  virtual void midi_reset() = 0;
  virtual void midi_sys_ex_start() = 0;
  virtual void midi_sys_ex_data(const uint8_t db1, const uint8_t db2,
                                const uint8_t db3) = 0;
  virtual void midi_sys_ex_end(const uint8_t db1) = 0;
  virtual void midi_sys_ex_end(const uint8_t db1, const uint8_t db2) = 0;
  virtual void midi_sys_ex_end(const uint8_t db1, const uint8_t db2,
                               const uint8_t db3) = 0;
};

#endif /* IMIDI_EVENT_LISTENER_HH */

/*
 * Local variables:
 *   mode: c++
 *   coding: utf-8
 * End:
 */

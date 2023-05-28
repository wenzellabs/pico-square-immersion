/*
 * PWM Audio Target of Simple Stupid Synthesizer
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

#ifndef PWM_AUDIO_TARGET_HPP
#define PWM_AUDIO_TARGET_HPP

#include "audio-target.hpp"
#include "pico/audio_pwm.h"

class PWM_audio_target : public Audio_target {
public:
  PWM_audio_target(const uint32_t sample_freq,
                   const uint8_t gpio_pin_pwm_mono);
  PWM_audio_target(const uint32_t sample_freq,
                   const uint8_t gpio_pin_pwm_left,
                   const uint8_t gpio_pin_pwm_right);
  virtual ~PWM_audio_target();
  void init(const uint16_t buffer_count = DEFAULT_BUFFER_COUNT,
            const uint16_t buffer_sample_count = DEFAULT_BUFFER_SAMPLE_COUNT,
            const enum audio_correction_mode mode = fixed_dither);
private:
  struct audio_pwm_channel_config
  _target_audio_config_l = default_left_channel_config;
  struct audio_pwm_channel_config
  _target_audio_config_r = default_right_channel_config;
  struct audio_pwm_channel_config
  _target_audio_config_mono = default_mono_channel_config;
};

#endif /* PWM_AUDIO_TARGET_HPP */

/*
 * Local variables:
 *   mode: c++
 *  coding: utf-8
 * End:
 */

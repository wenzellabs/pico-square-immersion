/*
 * I2S Audio Target of Simple Stupid Synthesizer
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

#include "i2s-audio-target.hpp"
#include "pico/stdlib.h"
#include "pico/audio.h"

I2S_audio_target::I2S_audio_target(const uint32_t sample_freq,
                                   const uint8_t gpio_pin_i2s_clock_base,
                                   const uint8_t gpio_pin_i2s_data)
  : Audio_target(sample_freq, true)
{
  _target_audio_config.clock_pin_base = gpio_pin_i2s_clock_base;
  _target_audio_config.data_pin = gpio_pin_i2s_data;
}

I2S_audio_target::~I2S_audio_target()
{
}

void
I2S_audio_target::init(const uint16_t buffer_count,
                       const uint16_t buffer_sample_count)
{
  _target_producer_pool =
    audio_new_producer_pool(&_target_audio_buffer_format,
                            buffer_count, buffer_sample_count);
  const struct audio_format *output_audio_format =
    audio_i2s_setup(&_target_audio_format, &_target_audio_config);
  if (!output_audio_format) {
    panic("unable to open audio device for selected audio format\n");
  }

  const uint8_t channel_count = 2; // I2S is always stereo
  const __unused bool ok =
    audio_i2s_connect_extra(_target_producer_pool, false, channel_count,
                            buffer_sample_count * channel_count, NULL);
  if (!ok) {
    panic("failed connecting I2S to producer pool");
  }

  audio_i2s_set_enabled(true);

  // I2S enhance signal quality
  const uint8_t gpio_pin_i2s_clock_base = _target_audio_config.clock_pin_base;
  const uint8_t gpio_pin_i2s_data = _target_audio_config.data_pin;
  gpio_set_drive_strength(gpio_pin_i2s_clock_base, GPIO_DRIVE_STRENGTH_2MA);
  gpio_set_drive_strength(gpio_pin_i2s_clock_base + 1, GPIO_DRIVE_STRENGTH_2MA);
  gpio_set_drive_strength(gpio_pin_i2s_data, GPIO_DRIVE_STRENGTH_2MA);
  gpio_disable_pulls(gpio_pin_i2s_clock_base);
  gpio_disable_pulls(gpio_pin_i2s_clock_base);
  gpio_disable_pulls(gpio_pin_i2s_data);
}

/*
 * Local variables:
 *   mode: c++
 *  coding: utf-8
 * End:
 */
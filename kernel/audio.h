/*
 * Copyright (c) 2014 Marco Maccaferri and Others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AUDIO_H_
#define AUDIO_H_

#include "platform.h"

#if defined(__cplusplus)
extern "C" {
#endif

int  audio_open(uint32_t samples);
void audio_close();

int  audio_get_sample_rate();
int  audio_get_channels();
int  audio_get_sample_size();

uint32_t audio_write(int16_t * stream, uint32_t samples);
void     audio_write_sample(int16_t sample);

void audio_play();
void audio_stop();

void audio_dma_irq();

extern void audio_callback(int16_t * buffer, uint32_t samples);

#if defined(__cplusplus)
}
#endif

#endif /* AUDIO_H_ */

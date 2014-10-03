#ifndef AUDIO_H_
#define AUDIO_H_

#include "platform.h"

#if defined(__cplusplus)
extern "C" {
#endif

void audio_init();

int  audio_get_sample_rate();
int  audio_get_sample_size();

uint32_t audio_write(uint32_t * stream, uint32_t samples);
void     audio_write_sample(uint32_t sample);

void audio_play();
void audio_stop();

void audio_dma_irq();

extern void audio_callback(uint32_t * buffer, uint32_t samples);

#if defined(__cplusplus)
}
#endif

#endif /* AUDIO_H_ */

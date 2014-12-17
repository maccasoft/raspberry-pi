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

#include <stdlib.h>
#include <string.h>

#include "audio.h"

#define SAMPLE_SHIFT       3

static uint32_t            max_samples;

static volatile int        cur_buffer;
static volatile DMA_CB     dma_cb[2];
static unsigned char     * dma_buffer[2];

static volatile uint32_t * write_ptr;
static volatile uint32_t   write_size;

static int16_t           * audio_buffer;

int audio_open(uint32_t samples) {
    volatile uint32_t * ptr;

    max_samples = samples * 2;

    for (int i = 0; i < 2; i++) {
        if (dma_buffer[i] != NULL) {
            free(dma_buffer[i]);
        }
        dma_buffer[i] = (unsigned char *)malloc(max_samples * 4 + 15);
        if (dma_buffer[i] == NULL) {
            return -1;
        }
        memset(dma_buffer[i], 0, max_samples * 4 + 15);
    }

    audio_buffer = (int16_t *)malloc(max_samples * 2);
    if (audio_buffer == NULL) {
        return -1;
    }
    memset(audio_buffer, 0, max_samples * 2);

    ptr = (uint32_t *) (GPIO_BASE + GPIO_GPFSEL4);
    *ptr = GPIO_FSEL0_ALT0 + GPIO_FSEL5_ALT0;
    usleep(110);

    PWM->ctl = 0;
    usleep(110);

    CLK->PWMCTL = CM_PASSWORD | CM_KILL;
    while ((CLK->PWMCTL & CM_BUSY) != 0)
        usleep(1);

    CLK->PWMDIV = CM_PASSWORD | (2 << 12);
    CLK->PWMCTL = CM_PASSWORD | CM_ENAB | CM_SRC_PLLDPER;
    while ((CLK->PWMCTL & CM_BUSY) == 0)
        usleep(1);

    PWM->rng1 = PWM->rng2 = 11336;

    for (int i = 0; i < 2; i++) {
        dma_cb[i].ti = DMA_DEST_DREQ | DMA_PERMAP_5 | DMA_SRC_INC | DMA_INTEN;
        dma_cb[i].source_ad = 0x40000000 | (((uint32_t) dma_buffer[i] + 15) & ~0xf);
        dma_cb[i].dest_ad = 0x7E000000 | PWM_BASE | PWM_FIF1;
        dma_cb[i].txfr_len = max_samples * 4;
        dma_cb[i].stride = 0;
        dma_cb[i].nextconbk = 0;
    }

    PWM->dmac = PWM_ENAB | 0x0008; // PWM DMA Enable
    PWM->ctl = PWM_USEF2 | PWM_PWEN2 | PWM_USEF1 | PWM_PWEN1 | PWM_CLRF1;

    cur_buffer = 0;
    write_ptr = (uint32_t *)dma_cb[cur_buffer].source_ad;
    write_size = 0;

    return 0;
}

void audio_close() {
    IRQ->irq1Disable = INTERRUPT_DMA0;
    DMA->ch[0].cs = DMA_RESET;

    for (int i = 0; i < 2; i++) {
        if (dma_buffer[i] != NULL) {
            free(dma_buffer[i]);
            dma_buffer[i] = NULL;
        }
    }
}

int audio_get_sample_rate() {
    return 22050;
}

int audio_get_channels() {
    return 2;
}

int audio_get_sample_size() {
    return 16 - SAMPLE_SHIFT;
}

void audio_play() {
    uint32_t data;

    for (int i = 0; i < 2; i++) {
        memset(dma_buffer[i], 0, max_samples * 4 + 15);
    }

    IRQ->irq1Enable = INTERRUPT_DMA0;

    DMA->enable = DMA_EN0;  // Set DMA Channel 0 Enable Bit
    DMA->ch[0].conblk_ad = 0x40000000 | (uint32_t) &dma_cb[cur_buffer];  // Set Control Block Data Address Into DMA Channel 0 Controller
    DMA->ch[0].cs = DMA_ACTIVE | DMA_INT; // Start DMA

    cur_buffer = cur_buffer == 0 ? 1 : 0;

    audio_callback(audio_buffer, max_samples);

    int16_t * src = audio_buffer;
    volatile uint32_t * dst = (uint32_t *) dma_cb[cur_buffer].source_ad;

    for (int i = 0; i < max_samples; i++) {
        data = *src++ + 32768;
        *dst++ = data >> SAMPLE_SHIFT;
    }
}

void audio_stop() {
    IRQ->irq1Disable = INTERRUPT_DMA0;
}

void audio_dma_irq() {
    uint32_t data;

    DMA->enable = DMA_EN0;  // Set DMA Channel 0 Enable Bit
    DMA->ch[0].conblk_ad = 0x40000000 | (uint32_t) &dma_cb[cur_buffer];  // Set Control Block Data Address Into DMA Channel 0 Controller
    DMA->ch[0].cs = DMA_ACTIVE | DMA_INT; // Start DMA

    cur_buffer = cur_buffer == 0 ? 1 : 0;

    audio_callback(audio_buffer, max_samples);

    int16_t * src = audio_buffer;
    volatile uint32_t * dst = (uint32_t *) dma_cb[cur_buffer].source_ad;

    for (int i = 0; i < max_samples; i++) {
        data = *src++ + 32768;
        *dst++ = data >> SAMPLE_SHIFT;
    }
}

uint32_t audio_write(int16_t * stream, uint32_t samples) {
    uint32_t data;
    uint32_t written = 0;

    while (write_size < max_samples && written < samples) {
        data = *stream++ + 32768;
        *write_ptr++ = data >> SAMPLE_SHIFT;
        write_size++;
        written++;

        if (write_size >= max_samples) {
            while((DMA->ch[0].cs & DMA_ACTIVE) != 0)
                ;

            DMA->enable = DMA_EN0;
            DMA->ch[0].conblk_ad = 0x40000000 | (uint32_t) &dma_cb[cur_buffer];
            DMA->ch[0].cs = DMA_ACTIVE;

            cur_buffer = cur_buffer == 0 ? 1 : 0;
            write_ptr = (uint32_t *)dma_cb[cur_buffer].source_ad;
            write_size = 0;
        }
    }

    return written;
}

void audio_write_sample(int16_t sample) {
    uint32_t data = sample + 32768;
    *write_ptr++ = data >> SAMPLE_SHIFT;
    write_size++;

    if (write_size >= max_samples) {
        while((DMA->ch[0].cs & DMA_ACTIVE) != 0)
            ;

        DMA->enable = DMA_EN0;
        DMA->ch[0].conblk_ad = 0x40000000 | (uint32_t) &dma_cb[cur_buffer];
        DMA->ch[0].cs = DMA_ACTIVE;

        cur_buffer = cur_buffer == 0 ? 1 : 0;
        write_ptr = (uint32_t *)dma_cb[cur_buffer].source_ad;
        write_size = 0;
    }
}

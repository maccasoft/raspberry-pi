/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

/* Output audio to nowhere... */

#include "SDL_audio.h"
#include "../SDL_audio_c.h"
#include "SDL_raspberryaudio.h"

#include "../../core/raspberry/SDL_raspberry.h"

static SDL_AudioDevice * device;
static int running;
static int locked;

static volatile int      cur_buffer;
static volatile DMA_CB   dma_cb[2];
static unsigned char   * dma_buffer[2];

static Uint16          * audio_buffer;

void sdl_audio_dma_irq() {
    int16_t * src;
    uint32_t data;

    DMA->enable = DMA_EN0;  // Set DMA Channel 0 Enable Bit
    DMA->ch[0].conblk_ad = 0x40000000 | (uint32_t) &dma_cb[cur_buffer]; // Set Control Block Data Address Into DMA Channel 0 Controller
    DMA->ch[0].cs = DMA_ACTIVE | DMA_INT; // Start DMA

    cur_buffer = cur_buffer == 0 ? 1 : 0;

    if (locked) {
        return;
    }

    if (device->convert.needed) {
        (*device->spec.callback)(device->spec.userdata, (Uint8 *) device->convert.buf, device->convert.len);
        SDL_ConvertAudio(&device->convert);
        src = (int16_t *) device->convert.buf;
    }
    else {
        (*device->spec.callback)(device->spec.userdata, (Uint8 *) audio_buffer, device->spec.size);
        src = (int16_t *) audio_buffer;
    }

    volatile uint32_t * dst = (uint32_t *) dma_cb[cur_buffer].source_ad;

    for (int i = 0; i < device->spec.samples; i++) {
        data = *src++ + 32768;
        *dst++ = data >> 3;
    }
    dma_cb[cur_buffer].txfr_len = device->spec.samples * 4;

    if (device->spec.channels == 2) {
        for (int i = 0; i < device->spec.samples; i++) {
            data = *src++ + 32768;
            *dst++ = data >> 3;
        }
        dma_cb[cur_buffer].txfr_len += device->spec.samples * 4;
    }
}

static int RASPBERRYAUD_OpenDevice(_THIS, const char *devname, int iscapture) {
    volatile uint32_t * ptr;
    unsigned int buffer_size;

    // Force our parameters
    this->spec.freq = 22050;
    this->spec.channels = 2;
    this->spec.format = AUDIO_S16;

    // Allocate dma buffers (32bit samples size, 16-bytes aligned)
    for (int i = 0; i < 2; i++) {
        if (dma_buffer[i] != NULL) {
            free(dma_buffer[i]);
        }
        dma_buffer[i] = (unsigned char *)malloc(this->spec.samples * 2 * 4 + 15);
        memset(dma_buffer[i], 0, this->spec.samples * 2 * 4 + 15);
    }

    // Allocate callback buffer (16bit samples size)
    audio_buffer = (Uint16 *)malloc(this->spec.samples * this->spec.channels * 2);
    memset(audio_buffer, 0, this->spec.samples * this->spec.channels * 2);

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
        uint32_t source_ad = (uint32_t) dma_buffer[i];
        while((source_ad & 0xF) != 0) { // Align to 16-bytes boundary
            source_ad++;
        }
        dma_cb[i].ti = DMA_DEST_DREQ | DMA_PERMAP_5 | DMA_SRC_INC | DMA_INTEN;
        dma_cb[i].source_ad = 0x40000000 + source_ad;
        dma_cb[i].dest_ad = 0x7E000000 | 0x20C000 | 0x18;
        dma_cb[i].txfr_len = this->spec.samples * 2 * 4;
        dma_cb[i].stride = 0;
        dma_cb[i].nextconbk = 0;
    }

    PWM->dmac = PWM_ENAB | 0x0001; // PWM DMA Enable
    PWM->ctl = PWM_USEF2 | PWM_PWEN2 | PWM_USEF1 | PWM_PWEN1 | PWM_CLRF1;

    device = this;
    cur_buffer = 0;
    running = 0;
    locked = 0;

    return 0; /* always succeeds. */
}

static void RASPBERRYAUD_CloseDevice(_THIS) {
    IRQ->irq1Disable = INTERRUPT_DMA0;
    DMA->ch[0].cs = DMA_RESET;

    if (audio_buffer != NULL) {
        free(audio_buffer);
        audio_buffer = NULL;
    }

    for (int i = 0; i < 2; i++) {
        if (dma_buffer[i] != NULL) {
            free(dma_buffer[i]);
            dma_buffer[i] = NULL;
        }
    }
}

static void RASPBERRYAUD_LockDevice(_THIS) {
    locked++;
}

static void RASPBERRYAUD_UnlockDevice(_THIS) {
    if (this->paused) {
        IRQ->irq1Disable = INTERRUPT_DMA0;
        DMA->ch[0].cs = DMA_RESET;
        running = 0;
    }
    else if (!running) {
        IRQ->irq1Enable = INTERRUPT_DMA0;

        DMA->enable = DMA_EN0;  // Set DMA Channel 0 Enable Bit
        DMA->ch[0].conblk_ad = 0x40000000 | (uint32_t) &dma_cb[cur_buffer]; // Set Control Block Data Address Into DMA Channel 0 Controller
        DMA->ch[0].cs = DMA_ACTIVE | DMA_INT; // Start DMA

        cur_buffer = cur_buffer == 0 ? 1 : 0;

        running = 1;
    }

    if (locked > 0)
        locked--;
}

static int RASPBERRYAUD_Init(SDL_AudioDriverImpl * impl) {

    /* Set the function pointers */
    impl->OpenDevice = RASPBERRYAUD_OpenDevice;
    impl->CloseDevice = RASPBERRYAUD_CloseDevice;
    impl->LockDevice = RASPBERRYAUD_LockDevice;
    impl->UnlockDevice = RASPBERRYAUD_UnlockDevice;
    impl->OnlyHasDefaultOutputDevice = 1;
    impl->ProvidesOwnCallbackThread = 1;
    impl->SkipMixerLock = 1;

    return 1; /* this audio target is available. */
}

AudioBootStrap RASPBERRYAUD_bootstrap = {
    "rpi", "SDL Raspberry Pi audio driver", RASPBERRYAUD_Init, 0
};

/* vi: set ts=4 sw=4 expandtab: */

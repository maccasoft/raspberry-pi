#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "wiring.h"
#include "console.h"
#include "audio.h"

#include "audio_data.h"

#if BYTES_PER_PIXEL == 2

#define BORDER_COLOR        RGB(26, 5, 10)
#define BACKGROUND_COLOR    RGB(12, 0, 4)

#elif BYTES_PER_PIXEL == 4

#define BORDER_COLOR        RGB(213, 41, 82)
#define BACKGROUND_COLOR    RGB(98, 0, 32)

#endif

#if defined(__cplusplus)
extern "C" {
#endif

__attribute__ ((interrupt ("IRQ"))) void interrupt_irq() {
    if ((IRQ->irqBasicPending & INTERRUPT_ARM_TIMER) != 0)
        ;
    if ((IRQ->irq1Pending & INTERRUPT_DMA0) != 0) {
        audio_dma_irq();
    }
}

#if defined(__cplusplus)
}
#endif

typedef struct {
    char   *curPtr;
    char   *filePtr;
    size_t  fileSize;
} ogg_file;

ogg_file of;

size_t AR_readOgg(void *dst, size_t size1, size_t size2, void *fh) {
    ogg_file *of = (ogg_file *)fh;
    size_t len = size1 * size2;
    if (of->curPtr + len > of->filePtr + of->fileSize) {
        len = of->filePtr + of->fileSize - of->curPtr;
    }
    memcpy(dst, of->curPtr, len);
    of->curPtr += len;
    return len;
}

int AR_seekOgg(void *fh, int64_t to, int type) {
    ogg_file *of = (ogg_file *)fh;

    switch (type) {
        case SEEK_CUR:
            of->curPtr += to;
            break;
        case SEEK_END:
            of->curPtr = of->filePtr + of->fileSize - to;
            break;
        case SEEK_SET:
            of->curPtr = of->filePtr + to;
            break;
        default:
            return -1;
    }
    if (of->curPtr < of->filePtr) {
        of->curPtr = of->filePtr;
        return -1;
    }
    if (of->curPtr > of->filePtr + of->fileSize) {
        of->curPtr = of->filePtr + of->fileSize;
        return -1;
    }
    return 0;
}

int AR_closeOgg(void *fh) {
    return 0;
}

long AR_tellOgg(void *fh) {
    ogg_file *of = (ogg_file *)fh;
    return (of->curPtr - of->filePtr);
}

void audio_callback(uint32_t * stream, uint32_t samples) {
    uint32_t s1, s2, sample;

    for (int i = 0; i < samples; i += 2) {
        s1 = s2 = 0;

        if (AR_readOgg(&s1, 1, sizeof(s1), &of) != sizeof(s1)) {
            AR_seekOgg(&of, 0, SEEK_SET);
            AR_readOgg(&s1, 1, sizeof(s1), &of);
        }

        AR_readOgg(&s2, 1, sizeof(s2), &of);

        sample = (s1 + s2) >> 1;

        *stream++ = sample;
        *stream++ = sample;
    }
}

void audio_write_chunk() {
    uint32_t s1, s2, sample;

    for (int i = 0; i < 256; i++) {
        s1 = s2 = 0;

        if (AR_readOgg(&s1, 1, sizeof(s1), &of) != sizeof(s1)) {
            AR_seekOgg(&of, 0, SEEK_SET);
            AR_readOgg(&s1, 1, sizeof(s1), &of);
        }

        AR_readOgg(&s2, 1, sizeof(s2), &of);

        sample = (s1 + s2) >> 1;

        audio_write_sample(sample);
        audio_write_sample(sample);
    }
}

void main() {
    int led_status = LOW;
    struct timer_wait tw;

    fb_init(384, 288);
    fb_fill_rectangle(0, 0, fb_width - 1, fb_height - 1, BORDER_COLOR);

    initscr(40, 25);
    cur_fore = WHITE;
    cur_back = BACKGROUND_COLOR;
    clear();

    mvaddstr(1, 9, "**** RASPBERRY-PI ****");
    mvaddstr(3, 7, "BARE-METAL SYSTEM TEMPLATE\r\n");

    pinMode(16, OUTPUT);
    register_timer(&tw, 250000);

    of.filePtr = of.curPtr = (char *)audio_data;
    of.fileSize = sizeof(audio_data);

    audio_init();
    //audio_play();

    addstr("\r\nREADY\r\n");

    while(1) {
        audio_write_chunk();

        if (compare_timer(&tw)) {
            led_status = led_status == LOW ? HIGH : LOW;
            digitalWrite(16, led_status);
        }
    }
}

#include "audio.h"

#define MAX_BUFFER       2048

static volatile int      cur_buffer;
static volatile DMA_CB   dma_cb[2];
static volatile uint32_t dma_buffer1[MAX_BUFFER] __attribute__((aligned (16)));
static volatile uint32_t dma_buffer2[MAX_BUFFER] __attribute__((aligned (16)));

static volatile uint32_t * write_ptr;
static volatile uint32_t   write_size;

void audio_init() {
    volatile uint32_t * ptr;

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

    dma_cb[0].ti = DMA_DEST_DREQ | DMA_PERMAP_5 | DMA_SRC_INC | DMA_INTEN;
    dma_cb[0].source_ad = 0x40000000 + (uint32_t) dma_buffer1;
    dma_cb[0].dest_ad = 0x7E000000 | 0x20C000 | 0x18;
    dma_cb[0].txfr_len = sizeof(dma_buffer1);
    dma_cb[0].stride = 0;
    dma_cb[0].nextconbk = 0;

    dma_cb[1].ti = DMA_DEST_DREQ | DMA_PERMAP_5 | DMA_SRC_INC | DMA_INTEN;
    dma_cb[1].source_ad = 0x40000000 | (uint32_t) dma_buffer2;
    dma_cb[1].dest_ad = 0x7E000000 | 0x20C000 | 0x18;
    dma_cb[1].txfr_len = sizeof(dma_buffer2);
    dma_cb[1].stride = 0;
    dma_cb[1].nextconbk = 0;

    PWM->dmac = PWM_ENAB | 0x0001; // PWM DMA Enable
    PWM->ctl = PWM_USEF2 | PWM_PWEN2 | PWM_USEF1 | PWM_PWEN1 | PWM_CLRF1;

    cur_buffer = 0;
    write_ptr = (uint32_t *)dma_cb[cur_buffer].source_ad;
    write_size = 0;
}

int audio_get_sample_rate() {
    return 22050;
}

int audio_get_sample_size() {
    return 13;
}

void audio_play() {
    IRQ->irq1Enable = INTERRUPT_DMA0;

    DMA->enable = DMA_EN0;  // Set DMA Channel 0 Enable Bit
    DMA->ch[0].conblk_ad = 0x40000000 | (uint32_t) &dma_cb[cur_buffer];  // Set Control Block Data Address Into DMA Channel 0 Controller
    DMA->ch[0].cs = DMA_ACTIVE | DMA_INT; // Start DMA

    cur_buffer = cur_buffer == 0 ? 1 : 0;

    audio_callback((uint32_t *) dma_cb[cur_buffer].source_ad, MAX_BUFFER);
}

void audio_stop() {
    IRQ->irq1Disable = INTERRUPT_DMA0;
    DMA->ch[0].cs = DMA_RESET;
}

void audio_dma_irq() {
    DMA->enable = DMA_EN0;  // Set DMA Channel 0 Enable Bit
    DMA->ch[0].conblk_ad = 0x40000000 | (uint32_t) &dma_cb[cur_buffer];  // Set Control Block Data Address Into DMA Channel 0 Controller
    DMA->ch[0].cs = DMA_ACTIVE | DMA_INT; // Start DMA

    cur_buffer = cur_buffer == 0 ? 1 : 0;

    audio_callback((uint32_t *) dma_cb[cur_buffer].source_ad, MAX_BUFFER);
}

uint32_t audio_write(uint32_t * stream, uint32_t samples) {
    uint32_t written = 0;

    while (write_size < MAX_BUFFER && written < samples) {
        *write_ptr++ = *stream++ >> 3;
        write_size++;
        written++;
    }

    if (write_size >= MAX_BUFFER) {
        while((DMA->ch[0].cs & DMA_ACTIVE) != 0)
            ;

        DMA->enable = DMA_EN0;
        DMA->ch[0].conblk_ad = 0x40000000 | (uint32_t) &dma_cb[cur_buffer];
        DMA->ch[0].cs = DMA_ACTIVE;

        cur_buffer = cur_buffer == 0 ? 1 : 0;
        write_ptr = (uint32_t *)dma_cb[cur_buffer].source_ad;
        write_size = 0;

        while (write_size < MAX_BUFFER && written < samples) {
            *write_ptr++ = *stream++ >> 3;
            write_size++;
            written++;
        }
    }

    return written;
}

void audio_write_sample(uint32_t sample) {
    *write_ptr++ = sample >> 2;
    write_size++;

    if (write_size >= MAX_BUFFER) {
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

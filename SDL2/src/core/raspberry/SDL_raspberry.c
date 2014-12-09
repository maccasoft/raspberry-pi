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

#include "../../SDL_internal.h"
#include "SDL_raspberry.h"

#ifdef HAVE_USPI
#include "uspi.h"
#endif

extern void RASPBERRYAUD_DmaInterruptHandler();

// Port function select bits

#define FSEL_INPT       0b000
#define FSEL_OUTP       0b001
#define FSEL_ALT0       0b100
#define FSEL_ALT1       0b101
#define FSEL_ALT2       0b110
#define FSEL_ALT3       0b111
#define FSEL_ALT4       0b011
#define FSEL_ALT5       0b010

// gpioToGPFSEL:
//  Map a BCM_GPIO pin to it's Function Selection
//  control port. (GPFSEL 0-5)
//  Groups of 10 - 3 bits per Function - 30 bits per port

static uint8_t gpioToGPFSEL [] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};

// gpioToShift
//  Define the shift up for the 3 bits per pin in each GPFSEL port

static uint8_t gpioToShift [] = {
    0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
    0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
    0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
    0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
    0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
};

// gpioToGPSET:
//  (Word) offset to the GPIO Set registers for each GPIO pin

static uint8_t gpioToGPSET [] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

// gpioToGPCLR:
//  (Word) offset to the GPIO Clear registers for each GPIO pin

static uint8_t gpioToGPCLR [] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

// gpioToGPLEV:
//  (Word) offset to the GPIO Input level registers for each GPIO pin

static uint8_t gpioToGPLEV [] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

// gpioToPUDCLK
//  (Word) offset to the Pull Up Down Clock regsiter

static uint8_t gpioToPUDCLK [] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static void clockDelay() {
    unsigned int timer = 150;

    while(timer--) {
        __asm ("mov r0, r0"); /* No-op */
    }
}

static void bcm_mmio_write(uint32_t reg, uint32_t data) {
    __asm (
        "mcr p15, #0, %[zero], c7, c10, #5" : : [zero] "r" (0)
    );

    *(volatile uint32_t *) (reg) = data;

    __asm (
        "mcr p15, #0, %[zero], c7, c10, #5" : : [zero] "r" (0)
    );
}

static uint32_t bcm_mmio_read(uint32_t reg) {
    __asm (
        "mcr p15, #0, %[zero], c7, c10, #5" : : [zero] "r" (0)
    );
    return *(volatile uint32_t *) (reg);
}

void Raspberry_MailboxWrite(uint8_t channel, uint32_t data) {
    while (bcm_mmio_read(MAIL_BASE + MAIL_STATUS) & MAIL_FULL)
        ;
    bcm_mmio_write(MAIL_BASE + MAIL_WRITE, (data & 0xfffffff0) | (uint32_t) (channel & 0xf));
}

uint32_t Raspberry_MailboxRead(uint8_t channel) {
    while (1) {
        while (bcm_mmio_read(MAIL_BASE + MAIL_STATUS) & MAIL_EMPTY)
            ;

        uint32_t data = bcm_mmio_read(MAIL_BASE + MAIL_READ);
        uint8_t read_channel = (uint8_t) (data & 0xf);
        if (read_channel == channel) {
            return (data & 0xfffffff0);
        }
    }
}

void Raspberry_pinMode(int pin, int mode) {
    int fSel = gpioToGPFSEL[pin];
    int shift = gpioToShift[pin];

    if (mode == 0) {
        GPIO->gpfsel[fSel] = (GPIO->gpfsel[fSel] & ~(7 << shift)) | (FSEL_OUTP << shift);
    }
    else if (mode == 1) {
        GPIO->gpfsel[fSel] = (GPIO->gpfsel[fSel] & ~(7 << shift)) | (FSEL_INPT << shift);

        /* Set pull-up */
        GPIO->gppud = 2;
        usleep(5);
        GPIO->gppudclk[gpioToPUDCLK[pin]] = 1 << (pin & 31);
        usleep(5);

        GPIO->gppud = 0;
        usleep(5);
        GPIO->gppudclk[gpioToPUDCLK[pin]] = 0;
        usleep(5);
    }
}

int Raspberry_digitalRead(int pin) {
    if ((GPIO->gplev[gpioToGPLEV[pin]] & (1 << (pin & 31))) != 0) {
        return 1;
    }
    else {
        return 0;
    }
}

void Raspberry_digitalWrite(int pin, int value) {
    if (value == 0) {
        GPIO->gpclr[gpioToGPCLR[pin]] = 1 << (pin & 31);
    }
    else {
        GPIO->gpset[gpioToGPSET[pin]] = 1 << (pin & 31);
    }
}

void SDL_Interrupt_Handler() {
    if ((IRQ->irq1Pending & INTERRUPT_DMA0) != 0)
        RASPBERRYAUD_DmaInterruptHandler();
#ifdef HAVE_USPI
    else
        USPiInterruptHandler ();
#endif
}

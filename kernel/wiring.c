#include "platform.h"
#include "wiring.h"

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

void pinMode(int pin, int mode) {
    int fSel = gpioToGPFSEL[pin];
    int shift = gpioToShift[pin];

    if (mode == OUTPUT) {
        GPIO->gpfsel[fSel] = (GPIO->gpfsel[fSel] & ~(7 << shift)) | (FSEL_OUTP << shift);
    }
    else {
        GPIO->gpfsel[fSel] = (GPIO->gpfsel[fSel] & ~(7 << shift)) | (FSEL_INPT << shift);

        if (mode == INPUT_PULLUP) {
            GPIO->gppud = 2;
        }
        else if (mode == INPUT_PULLDOWN) {
            GPIO->gppud = 1;
        }
        else {
            GPIO->gppud = 0;
        }
        clockDelay(5);
        GPIO->gppudclk[gpioToPUDCLK[pin]] = 1 << (pin & 31);
        clockDelay (5);

        GPIO->gppud = 0;
        clockDelay(5);
        GPIO->gppudclk[gpioToPUDCLK[pin]] = 0;
        clockDelay(5);
    }
}

int digitalRead(int pin) {
    if ((GPIO->gplev[gpioToGPLEV[pin]] & (1 << (pin & 31))) != 0) {
        return HIGH;
    }
    else {
        return LOW;
    }
}

void digitalWrite(int pin, int value) {
    if (value == LOW) {
        GPIO->gpclr[gpioToGPCLR[pin]] = 1 << (pin & 31);
    }
    else {
        GPIO->gpset[gpioToGPSET[pin]] = 1 << (pin & 31);
    }
}

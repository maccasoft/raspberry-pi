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

#include "SDL_raspberry.h"

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

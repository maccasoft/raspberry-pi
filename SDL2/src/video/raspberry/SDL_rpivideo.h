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

#ifndef _SDL_rpivideo_h
#define _SDL_rpivideo_h

#include "../SDL_sysvideo.h"

#define PERIPHERAL_BASE                 0x20000000 // Peripheral Base Address
#define MAIL_BASE       (PERIPHERAL_BASE + 0xB880) // Mailbox Base Address

/* Mailbox */
#define MAIL_READ      0x00 // Mailbox Read Register
#define MAIL_CONFIG    0x1C // Mailbox Config Register
#define MAIL_STATUS    0x18 // Mailbox Status Register
#define MAIL_WRITE     0x20 // Mailbox Write Register

#define MAIL_EMPTY  0x40000000 // Mailbox Status Register: Mailbox Empty (There is nothing to read from the Mailbox)
#define MAIL_FULL   0x80000000 // Mailbox Status Register: Mailbox Full  (There is no space to write into the Mailbox)

#define MAIL_TAGS     0x8 // Mailbox Channel 8: Tags (ARM to VC)

#define TAG_ALLOCATE_BUFFER     0x40001
#define TAG_RELEASE_BUFFER      0x48001
#define TAG_BLANK_SCREEN        0x40002
#define TAG_GET_PHYS_WH         0x40003
#define TAG_TEST_PHYS_WH        0x44003
#define TAG_SET_PHYS_WH         0x48003
#define TAG_GET_VIRT_WH         0x40004
#define TAG_TEST_VIRT_WH        0x44004
#define TAG_SET_VIRT_WH         0x48004
#define TAG_GET_DEPTH           0x40005
#define TAG_TEST_DEPTH          0x44005
#define TAG_SET_DEPTH           0x48005
#define TAG_GET_PIXEL_ORDER     0x40006
#define TAG_TEST_PIXEL_ORDER    0x44006
#define TAG_SET_PIXEL_ORDER     0x48006
#define TAG_GET_ALPHA_MODE      0x40007
#define TAG_TEST_ALPHA_MODE     0x44007
#define TAG_SET_ALPHA_MODE      0x48007
#define TAG_GET_PITCH           0x40008
#define TAG_GET_VIRT_OFFSET     0x40009
#define TAG_TEST_VIRT_OFFSET    0x44009
#define TAG_SET_VIRT_OFFSET     0x48009
#define TAG_GET_OVERSCAN        0x4000a
#define TAG_TEST_OVERSCAN       0x4400a
#define TAG_SET_OVERSCAN        0x4800a
#define TAG_GET_PALETTE         0x4000b
#define TAG_TEST_PALETTE        0x4400b
#define TAG_SET_PALETTE         0x4800b
#define TAG_GET_LAYER           0x4000c
#define TAG_TST_LAYER           0x4400c
#define TAG_SET_LAYER           0x4800c
#define TAG_GET_TRANSFORM       0x4000d
#define TAG_TST_TRANSFORM       0x4400d
#define TAG_SET_TRANSFORM       0x4800d
#define TAG_TEST_VSYNC          0x4400e
#define TAG_SET_VSYNC           0x4800e

__attribute__ ((naked)) void dmb();
__attribute__ ((naked)) void flush_cache();

void     Raspberry_MailboxWrite(uint8_t channel, uint32_t data);
uint32_t Raspberry_MailboxRead(uint8_t channel);

#endif /* _SDL_rpivideo_h */

/* vi: set ts=4 sw=4 expandtab: */

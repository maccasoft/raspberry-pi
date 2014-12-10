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

#include <string.h>

#include "platform.h"
#include "fb.h"

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

uint32_t fb_width;
uint32_t fb_height;
uint32_t fb_addr;
uint32_t fb_size;
uint32_t fb_pitch;

static uint32_t fb_buffer_addr[2];
static uint32_t fb_buffer;

static int phys_width;
static int phys_height;

void fb_init(int width, int height) {
    unsigned int mb_addr = 0x40007000;      // 0x7000 in L2 cache coherent mode
    volatile unsigned int *mailbuffer = (unsigned int *) mb_addr;

    /* Get the display size */
    mailbuffer[0] = 8 * 4;              // size of this message
    mailbuffer[1] = 0;                  // this is a request
    mailbuffer[2] = TAG_GET_PHYS_WH;    // get physical width/height tag
    mailbuffer[3] = 8;                  // value buffer size
    mailbuffer[4] = 0;                  // request/response
    mailbuffer[5] = 0;                  // space to return width
    mailbuffer[6] = 0;                  // space to return height
    mailbuffer[7] = 0;
    mbox_write(MAIL_TAGS, mb_addr);

    mbox_read(MAIL_TAGS);
    if (mailbuffer[1] == MAIL_FULL) {
        phys_width = mailbuffer[5];
        phys_height = mailbuffer[6];
    }

    if (width == 0 || height == 0) {
        width = phys_width;
        height = phys_height;
    }

    /* Now set the physical and virtual sizes and bit depth and allocate the framebuffer */
    mailbuffer[0] = 22 * 4;
    mailbuffer[1] = 0;

    mailbuffer[2] = TAG_SET_PHYS_WH;
    mailbuffer[3] = 8;
    mailbuffer[4] = 8;
    mailbuffer[5] = width;
    mailbuffer[6] = height;

    mailbuffer[7] = TAG_SET_VIRT_WH;
    mailbuffer[8] = 8;
    mailbuffer[9] = 8;
    mailbuffer[10] = width;
    mailbuffer[11] = height * 2;

    mailbuffer[12] = TAG_SET_DEPTH;
    mailbuffer[13] = 4;
    mailbuffer[14] = 4;
    mailbuffer[15] = BPP;

    mailbuffer[16] = TAG_ALLOCATE_BUFFER;
    mailbuffer[17] = 8;  // response size = 8
    mailbuffer[18] = 4;  // request size = 4
    mailbuffer[19] = 16; // requested alignment of buffer, space for returned address
    mailbuffer[20] = 0;  // space for returned size

    mailbuffer[21] = 0;  // terminating tag
    mbox_write(MAIL_TAGS, mb_addr);

    mbox_read(MAIL_TAGS);

    /* Check the allocate_buffer response */
    if (mailbuffer[1] == MAIL_FULL && mailbuffer[18] == (MAIL_FULL | 8)) {
        fb_addr = mailbuffer[19];
        fb_size = mailbuffer[20];

        /* Get the pitch of the display */
        mailbuffer[0] = 7 * 4;
        mailbuffer[1] = 0;
        mailbuffer[2] = TAG_GET_PITCH;
        mailbuffer[3] = 4;
        mailbuffer[4] = 0;
        mailbuffer[5] = 0;
        mailbuffer[6] = 0;
        mbox_write(MAIL_TAGS, mb_addr);

        mbox_read(MAIL_TAGS);
        if (mailbuffer[1] == MAIL_FULL && mailbuffer[4] == (MAIL_FULL | 4)) {
            fb_pitch = mailbuffer[5];
        }
    }

    fb_width = width;
    fb_height = height;

    fb_buffer_addr[0] = fb_addr;
    fb_buffer_addr[1] = fb_addr + height * fb_pitch;
    fb_buffer = 0;
}

void fb_wait_vsync() {
    unsigned int mb_addr = 0x40007000;      // 0x7000 in L2 cache coherent mode
    volatile unsigned int *mailbuffer = (unsigned int *) mb_addr;

    mailbuffer[0] = 7 * 4;
    mailbuffer[1] = 0;

    mailbuffer[2] = TAG_SET_VSYNC;
    mailbuffer[3] = 4;
    mailbuffer[4] = 4;
    mailbuffer[5] = 0;

    mailbuffer[6] = 0;
    mbox_write(MAIL_TAGS, mb_addr);

    mbox_read(MAIL_TAGS);
}

pixel_t * fb_get_pixel_address(int x, int y) {
    return (pixel_t *) (fb_addr + y * fb_pitch + x * BYTES_PER_PIXEL);
}

void fb_draw_rectangle(int x0, int y0, int x1, int y1, pixel_t color) {
    volatile pixel_t * fb = (pixel_t *) (fb_addr + y0 * fb_pitch + x0 * BYTES_PER_PIXEL);

    int dx = x1 - x0;
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    for (int cx = x0; cx != x1; cx += sx) {
        *fb = color;
        fb += sx;
    }
    fb -= dx;

    for (; y0 != y1; y0 += sy) {
        *fb = color;
        fb[dx] = color;
        fb += (sy * fb_pitch) / BYTES_PER_PIXEL;
    }

    for (int cx = x0; cx != x1; cx += sx) {
        *fb = color;
        fb += sx;
    }
}

void fb_fill_rectangle(int x0, int y0, int x1, int y1, pixel_t color) {
    volatile pixel_t * fb = (pixel_t *) (fb_addr + y0 * fb_pitch + x0 * BYTES_PER_PIXEL);

    int dx = x1 - x0;
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    for (; y0 != y1; y0 += sy) {
        for (int cx = x0; cx != x1; cx += sx) {
            *fb = color;
            fb += sx;
        }
        *fb = color;
        fb -= dx;
        fb += (sy * fb_pitch) / BYTES_PER_PIXEL;
    }
    for (int cx = x0; cx != x1; cx += sx) {
        *fb = color;
        fb += sx;
    }
    *fb = color;
}

void fb_begin_doublebuffer() {
    fb_addr = fb_buffer == 0 ? fb_buffer_addr[1] : fb_buffer_addr[0];
}

void fb_end_doublebuffer() {
    fb_addr = fb_buffer_addr[fb_buffer];
}

pixel_t * fb_flip() {
    unsigned int mb_addr = 0x40007000;      // 0x7000 in L2 cache coherent mode
    volatile unsigned int *mailbuffer = (unsigned int *) mb_addr;

    flush_cache();

    if (fb_addr != fb_buffer_addr[fb_buffer]) {
        fb_buffer = fb_buffer == 0 ? 1 : 0;
        fb_addr = fb_buffer == 0 ? fb_buffer_addr[1] : fb_buffer_addr[0];

        mailbuffer[0] = 12 * 4;
        mailbuffer[1] = 0;

        mailbuffer[2] = TAG_SET_VSYNC;
        mailbuffer[3] = 4;
        mailbuffer[4] = 4;
        mailbuffer[5] = 0;

        mailbuffer[6] = TAG_SET_VIRT_OFFSET;
        mailbuffer[7] = 8;
        mailbuffer[8] = 8;
        mailbuffer[9] = 0;
        mailbuffer[10] = fb_buffer * fb_height;

        mailbuffer[11] = 0;
        mbox_write(MAIL_TAGS, mb_addr);

        mbox_read(MAIL_TAGS);
    }

    return (pixel_t *) fb_addr;
}

void fb_blit(blit_info * info) {
    int size = info->dst_w * sizeof(pixel_t);
    uchar_t * dst_ptr = info->dst + info->dst_y * info->dst_pitch + info->dst_x * sizeof(pixel_t);
    uchar_t * src_ptr = info->src + info->src_y * info->src_pitch + info->src_x * sizeof(pixel_t);

    for (int y = 0; y < info->dst_h; y++) {
        memcpy(dst_ptr, src_ptr, size);
        dst_ptr += info->dst_pitch;
        src_ptr += info->src_pitch;
    }
}

void fb_blit_colorkey(blit_info * info, pixel_t key) {
    pixel_t data;
    uchar_t * dst_ptr = info->dst + info->dst_y * info->dst_pitch + info->dst_x * sizeof(pixel_t);
    uchar_t * src_ptr = info->src + info->src_y * info->src_pitch + info->src_x * sizeof(pixel_t);

    for (int y = 0; y < info->dst_h; y++) {
        pixel_t * src = (pixel_t *) src_ptr;
        pixel_t * dst = (pixel_t *) dst_ptr;
        for (int x = 0; x < info->dst_w; x++, src++, dst++) {
            data = *src;
            if (data != key) {
                *dst = data;
            }
        }
        dst_ptr += info->dst_pitch;
        src_ptr += info->src_pitch;
    }
}

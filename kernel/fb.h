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

#ifndef FB_H_
#define FB_H_

#include <stdint.h>

#define BYTES_PER_PIXEL 4
#define BPP             (BYTES_PER_PIXEL << 3)

#if BYTES_PER_PIXEL == 2
typedef unsigned short  pixel_t;

#define RGB(r,g,b)      (pixel_t)((r << 11) | (g << 6) | b)
#define RGBA(r,g,b,a)   (pixel_t)((r << 11) | (g << 6) | b)

#define RED             RGB(31, 0, 0)
#define GREEN           RGB(0, 31, 0)
#define BLUE            RGB(0, 0, 31)
#define YELLOW          RGB(31, 31, 0)
#define MAGENTA         RGB(31, 0, 31)
#define CYAN            RGB(0, 31, 31)
#define WHITE           RGB(31, 31, 31)
#define BLACK           RGB(0, 0, 0)

#define DARK_RED        RGB(15, 0, 0)
#define DARK_GREEN      RGB(0, 15, 0)
#define DARK_BLUE       RGB(0, 0, 15)
#define DARK_YELLOW     RGB(15, 15, 0)
#define DARK_MAGENTA    RGB(15, 0, 15)
#define DARK_CYAN       RGB(0, 15, 15)
#define LIGHT_GRAY      RGB(15, 15, 15)
#define DARK_GRAY       RGB(7, 7, 7)

#elif BYTES_PER_PIXEL == 4

typedef unsigned int    pixel_t;

#define RGB(r,g,b)      (pixel_t)((255 << 24) | (b << 16) | (g << 8) | r)
#define RGBA(r,g,b,a)   (pixel_t)((a << 24) | (b << 16) | (g << 8) | r)

#define RED             RGB(255, 0, 0)
#define GREEN           RGB(0, 255, 0)
#define BLUE            RGB(0, 0, 255)
#define YELLOW          RGB(255, 255, 0)
#define MAGENTA         RGB(255, 0, 255)
#define CYAN            RGB(0, 255, 255)
#define WHITE           RGB(255, 255, 255)
#define BLACK           RGB(0, 0, 0)

#define DARK_RED        RGB(127, 0, 0)
#define DARK_GREEN      RGB(0, 127, 0)
#define DARK_BLUE       RGB(0, 0, 127)
#define DARK_YELLOW     RGB(127, 127, 0)
#define DARK_MAGENTA    RGB(127, 0, 127)
#define DARK_CYAN       RGB(0, 127, 127)
#define LIGHT_GRAY      RGB(127, 127, 127)
#define DARK_GRAY       RGB(63, 63 63)

#endif

extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_addr;
extern uint32_t fb_size;
extern uint32_t fb_pitch;

#ifndef uchar_t
typedef unsigned char uchar_t;
#endif

typedef struct {
    uchar_t * dst;
    int32_t   dst_x;
    int32_t   dst_y;
    int32_t   dst_w;
    int32_t   dst_h;
    int32_t   dst_pitch;
    uchar_t * src;
    int32_t   src_x;
    int32_t   src_y;
    int32_t   src_pitch;
} blit_info;

#if defined(__cplusplus)
extern "C" {
#endif

void fb_init(int width, int height);

void fb_wait_vsync();

void fb_begin_doublebuffer();
void fb_end_doublebuffer();
pixel_t * fb_flip();

pixel_t * fb_get_pixel_address(int x, int y);

void fb_draw_rectangle(int x0, int y0, int x1, int y1, pixel_t color);
void fb_fill_rectangle(int x0, int y0, int x1, int y1, pixel_t color);

void fb_blit(blit_info * info);
void fb_blit_colorkey(blit_info * info, pixel_t key);

#if defined(__cplusplus)
}
#endif

#endif /* FB_H_ */

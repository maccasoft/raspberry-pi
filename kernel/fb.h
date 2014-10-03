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

#if defined(__cplusplus)
extern "C" {
#endif

void fb_init(int width, int height);

void fb_wait_vsync();

void fb_begin_doublebuffer();
void fb_end_doublebuffer();
void fb_flip();

pixel_t * fb_get_pixel_address(int x, int y);

void fb_draw_rectangle(int x0, int y0, int x1, int y1, pixel_t color);
void fb_fill_rectangle(int x0, int y0, int x1, int y1, pixel_t color);

#if defined(__cplusplus)
}
#endif

#endif /* FB_H_ */

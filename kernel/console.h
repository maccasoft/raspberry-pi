#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "fb.h"

extern int con_width;
extern int con_height;
extern int txt_width;
extern int txt_height;
extern int cur_x;
extern int cur_y;
extern pixel_t cur_fore;
extern pixel_t cur_back;

#if defined(__cplusplus)
extern "C" {
#endif

void initscr(int width, int height);
void clear();
void move(int y, int x);

int addch(int c);
int addstr(const char *s);

int mvaddch(int y, int x, int c);
int mvaddstr(int y, int x, const char *s);

void refresh();

#define getyx(y,x)  y = cur_y; x = cur_x

#if defined(__cplusplus)
}
#endif

#endif /* CONSOLE_H_ */

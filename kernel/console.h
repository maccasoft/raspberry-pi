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

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "fb.h"

#define KEY_ESC     0x2900

#define KEY_F1      0x3A00
#define KEY_F2      0x3B00
#define KEY_F3      0x3C00
#define KEY_F4      0x3D00
#define KEY_F5      0x3E00
#define KEY_F6      0x3F00
#define KEY_F7      0x4000
#define KEY_F8      0x4100
#define KEY_F9      0x4200
#define KEY_F10     0x4300
#define KEY_F11     0x4400
#define KEY_F12     0x4500

#define KEY_CANC    0x4C00
#define KEY_INS     0x4900
#define KEY_PGDN    0x4E00
#define KEY_PGUP    0x4B00
#define KEY_END     0x4D00
#define KEY_HOME    0x4A00
#define KEY_RIGHT   0x4F00
#define KEY_LEFT    0x5000
#define KEY_DOWN    0x5100
#define KEY_UP      0x5200

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

int addstrf(const char *ptr, ...);
int mvaddstrf(int y, int x, const char *ptr, ...);

int getch();

void refresh();

#define getyx(y,x)  y = cur_y; x = cur_x

void toggle_cursor();
void hide_cursor();

#if defined(__cplusplus)
}
#endif

#endif /* CONSOLE_H_ */

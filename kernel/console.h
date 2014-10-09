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

void toggle_cursor();
void hide_cursor();

#if defined(__cplusplus)
}
#endif

#endif /* CONSOLE_H_ */

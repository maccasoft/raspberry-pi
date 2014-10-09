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

#ifndef WIRING_H_
#define WIRING_H_

// Pin modes

#define OUTPUT              0
#define INPUT               1
#define INPUT_PULLUP        2
#define INPUT_PULLDOWN      3

#define LOW                 0
#define HIGH                1

#if defined(__cplusplus)
extern "C" {
#endif

void pinMode(int pin, int mode);
int  digitalRead(int pin);
void digitalWrite(int pin, int value);

#if defined(__cplusplus)
}
#endif

#endif /* WIRING_H_ */

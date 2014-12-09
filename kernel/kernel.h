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

#ifndef KERNEL_H_
#define KERNEL_H_

#include "fb.h"
#include "console.h"
#include "audio.h"
#include "emmc.h"
#include "ff.h"

#ifdef HAVE_USPI
#include "uspi.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

int usb_init();

int keyboard_init();

#if defined(__cplusplus)
}
#endif

#endif /* KERNEL_H_ */

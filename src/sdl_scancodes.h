/* ES40 emulator.
 * Copyright (C) 2007 by Brian Wheeler
 *
 * E-mail : bdwheele@indiana.edu
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * Although this is not required, the author would appreciate being notified of, 
 * and receiving any modifications you may make to the source code that might serve
 * the general public.
 */

/**
 * \file
 * Contains the SDL scancodes for console emulation.
 *
 * X-1.1        Camiel Vanderhoeven                             1-DEC-2007
 *      Initial version in CVS.
 *
 * X-0.0        bdwheele@indiana.edu     
 *      Generated file.
 *
 * \author  Brian Wheeler (bdwheele@indiana.edu)
 **/

#if defined(_WIN32)
#include <SDL_keysym.h>
#else
#include <SDL/SDL_keysym.h>
#endif

#define MAP_ASCII_LEN 127
u8 map_ascii[] = {
  0,0,0,0,0,0,0,0,  /* ignore 0-7 */
  0x66, // bkspc
  0x0d, // tab
  0x5A, // enter
  0,0, // ignore
  0x5A, // enter (again)
  0,0,0,0,0,0,0,0,0,0,0,0,0, // 14 - 26 ignore
  0x08, // esc
  0,0,0,0, // 28-31 ignore.
  0x29, // space
  0x16, // !
  0x52, // "
  0x26, // #
  0x25, // $
  0x2e, // percent (37) is missing from sdl?
  0x3d, // &
  0x52, // '
  0x46,
  0x45,
  0x3E,
  0x55,
  0x41,
  0x4e, // -
  0x49,
  0x4A,  
  0x45, // start of 0-9  should be at 48.
  0x16,
  0x1e,
  0x26,
  0x25,
  0x2e,
  0x36,
  0x3d,
  0x3e,
  0x46, // end of 0-9
  0x4c,
  0x4c,
  0x41,
  0x55,
  0x49,
  0x4A,
  0x1E, 
  0x1c, // a-z
  0x32,
  0x21,
  0x23,
  0x24,
  0x2b,
  0x34,
  0x33,
  0x43,
  0x3b,
  0x42,
  0x4b,
  0x3a,
  0x31,
  0x44,
  0x4d,
  0x15,
  0x2d,
  0x1b,
  0x2c,
  0x3c,
  0x2a,
  0x1d,
  0x22,
  0x35,
  0x1a, // end of a-z
  0x54,
  0x5c,
  0x5b,
  0x36,
  0x4e,
  0x0e,
  0x1c, // a-z
  0x32,
  0x21,
  0x23,
  0x24,
  0x2b,
  0x34,
  0x33,
  0x43,
  0x3b,
  0x42,
  0x4b,
  0x3a,
  0x31,
  0x44,
  0x4d,
  0x15,
  0x2d,
  0x1b,
  0x2c,
  0x3c,
  0x2a,
  0x1d,
  0x22,
  0x35,
  0x1a, // end of a-z
  0x54,
  0x5c,
  0x5b,
  0x0e,
  0x64 /* delete */
};


// sdlk numbers starting at 256 SDLK_KP0
#define MAP_KEYPAD_LEN 16
u8 map_keypad[] = {
  0x70,
  0x69,
  0x72,
  0x7A,
  0x6B,
  0x73,
  0x74,
  0x6C,
  0x75,
  0x7D,
  0x71,
  0x77,
  0x7E,
  0x84,
  0x7C,
  0x79

};

// sdlk numbers starting at 273 SDLK_UP
#define MAP_MIDPAD_LEN 9
u8 map_midpad[] = {
  0x63,
  0x60,
  0x6A,
  0x61,
  0x67,
  0x6E,
  0x65,
  0x6F,
  0x6D
};

// sdlk numbers starting at 282 SDLK_F1
#define MAP_FKEYS_LEN 12
u8 map_fkeys[] = {
  0x07,
  0x0F,
  0x17,
  0x1F,
  0x27,
  0x2F,
  0x37,
  0x3F,
  0x47,
  0x4F,
  0x56,
  0x5E
};

// sdlk numbers starting at 300. SDLK_NUMLOCK
#define MAP_MODKEYS_LEN 9
u8 map_modkeys[] = {
  0x76,
  0x14,
  0x5f,
  0x59,
  0x12,
  0x58,
  0x11,
  0x39,
  0x19
};

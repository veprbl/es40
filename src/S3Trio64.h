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
 * Contains the definitions for emulated S3 Trio 64 Video Card device.
 *
 * X-1.1        Camiel Vanderhoeven                             1-DEC-2007
 *      Initial version in CVS.
 *
 * X-0.0        bdwheele@indiana.edu     
 *      Generated file.
 *
 * \author  Brian Wheeler (bdwheele@indiana.edu)
 **/

#if !defined(INCLUDED_S3Trio64_H_)
#define INCLUDED_S3Trio64_H_

#include "SystemComponent.h"

#include <stdlib.h>
#if defined(_WIN32)
#include <SDL.h>
#else
#include <SDL/SDL.h>
#endif


/**
 * S3 Trio 64 Video Card
 **/

/* video card has 4M of ram */
#define VIDEO_RAM_SIZE 22
#define CRTC_MAX 0x20

class CS3Trio64 : public CSystemComponent
{
  public:
    virtual void SaveState(FILE * f);
    virtual void RestoreState(FILE * f);
    virtual int DoClock();
    virtual void WriteMem(int index, u64 address, int dsize, u64 data);
    virtual u64 ReadMem(int index, u64 address, int dsize);
    CS3Trio64(class CSystem * c);
    virtual ~CS3Trio64();
    virtual void ResetPCI();

    void setmode();
    void screenrefresh();


  private:
    u64 config_read(u64 address, int dsize);
    void config_write(u64 address, int dsize, u64 data);

    u64 mem_read(u64 address, int dsize);
    void mem_write(u64 address, int dsize, u64 data);

    u64 io_read(u64 address, int dsize);
    void io_write(u64 address, int dsize, u64 data);
    
    u64 legacy_read(u64 address, int dsize);
    void legacy_write(u64 address, int dsize, u64 data);

    u64 rom_read(u64 address, int dsize);
    void rom_write(u64 address, int dsize, u64 data);

    struct SS3Trio64State {
      u8 config_data[256];
      u8 config_mask[256];
      u8 framebuffer[1<<VIDEO_RAM_SIZE];
      u8 legacybuffer[131072];
      u64 video_base;
      u8 crtc_index;
      u64 crtc_data[CRTC_MAX];
      u8  video_mode;
#define MODE_TEXT 0

      u8 cursor_x;
      u8 cursor_y;
      u64 cursor_ttl;
#define BLINK_RATE 1000

      u64 refresh_ttl;
#define REFRESH_RATE 100

      SDL_Surface *screen;

    } state;
};

#endif // !defined(INCLUDED_S3Trio64_H_)

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
 * X-1.6        Camiel Vanderhoeven                             28-DEC-2007
 *      Keep the compiler happy.
 *
 * X-1.5        Camiel Vanderhoeven                             17-DEC-2007
 *      SaveState file format 2.1
 *
 * X-1.4        Brian Wheeler                                   10-DEC-2007
 *      Added SDL.h.
 *
 * X-1.3        Camiel Vanderhoeven                             10-DEC-2007
 *      Use new base class VGA.
 *
 * X-1.2        Camiel Vanderhoeven/Brian Wheeler               6-DEC-2007
 *      Changed implementation (with thanks to the Bochs project!!)
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

#include "VGA.h"
#include "Configurator.h"

#include <stdlib.h>
#include <SDL.h>
#include "gui/vga.h"

/* video card has 4M of ram */
#define VIDEO_RAM_SIZE 22
#define CRTC_MAX 0x57
#define VGA_BASE 0x3b0

/**
 * S3 Trio 64 Video Card
 **/

class CS3Trio64 : public CVGA
{
  public:
    virtual int SaveState(FILE * f);
    virtual int RestoreState(FILE * f);
    virtual int DoClock();
    virtual void WriteMem_Legacy(int index, u32 address, int dsize, u32 data);
    virtual u32 ReadMem_Legacy(int index, u32 address, int dsize);

    virtual void WriteMem_Bar(int func, int bar, u32 address, int dsize, u32 data);
    virtual u32 ReadMem_Bar(int func, int bar, u32 address, int dsize);

    CS3Trio64(CConfigurator * cfg, class CSystem * c, int pcibus, int pcidev);
    virtual ~CS3Trio64();

    void update(void);

    virtual u8 get_actl_palette_idx(u8 index);

private:
    u32 mem_read(u32 address, int dsize);
    void mem_write(u32 address, int dsize, u32 data);

    u32 io_read(u32 address, int dsize);
    void io_write(u32 address, int dsize, u32 data);

    void io_write_b(u32 address, u8 data);

    void write_b_3c0(u8 data);
    void write_b_3c2(u8 data);
    void write_b_3c4(u8 data);
    void write_b_3c5(u8 data);
    void write_b_3c6(u8 data);
    void write_b_3c7(u8 data);
    void write_b_3c8(u8 data);
    void write_b_3c9(u8 data);
    void write_b_3ce(u8 data);
    void write_b_3cf(u8 data);
    void write_b_3d4(u8 data);
    void write_b_3d5(u8 data);

    u8 read_b_3c0();
    u8 read_b_3c1();
    u8 read_b_3c3();
    u8 read_b_3c4();
    u8 read_b_3c5();
    u8 read_b_3c9();
    u8 read_b_3cc();
    u8 read_b_3cf();
    u8 read_b_3d4();
    u8 read_b_3d5();
    u8 read_b_3da();


    u32 legacy_read(u32 address, int dsize);
    void legacy_write(u32 address, int dsize, u32 data);

    u32 rom_read(u32 address, int dsize);
    void rom_write(u32 address, int dsize, u32 data);

    void redraw_area(unsigned x0, unsigned y0, unsigned width, unsigned height);
    void determine_screen_dimensions(unsigned *piHeight, unsigned *piWidth);

    char bios_message[200];
    int bios_message_size;

    void vga_mem_write(u32 addr, u8 value);
    u8 vga_mem_read(u32 addr);

    struct SS3Trio64State {
//      u8 disabled;

//      u8 framebuffer[1<<VIDEO_RAM_SIZE];
//      u8 legacybuffer[131072];
//      u64 video_base;

      bool vga_enabled;
      bool  vga_mem_updated;
      u16 charmap_address;
      bool x_dotclockdiv2;
      bool y_doublescan;
      unsigned line_offset;
      unsigned line_compare;
      unsigned vertical_display_end;
//      u8 vga_memory[256 * 1024];
      u8 text_snapshot[32 * 1024]; // current text snapshot
      bool vga_tile_updated[BX_NUM_X_TILES][BX_NUM_Y_TILES];
      u8 *memory;
      u32 memsize;
      u8 last_bpp;
      u8 tile[X_TILESIZE * Y_TILESIZE * 4]; /**< Currently allocates the tile as large as needed. */
      unsigned x_tilesize;
      unsigned y_tilesize;

     struct {
       bool  flip_flop; /* 0 = address, 1 = data-write */
       unsigned address;  /* register number */
       bool  video_enabled;
       u8    palette_reg[16];
       u8    overscan_color;
       u8    color_plane_enable;
       u8    horiz_pel_panning;
       u8    color_select;
       struct {
         bool graphics_alpha;
         bool display_type;
         bool enable_line_graphics;
         bool blink_intensity;
         bool pixel_panning_compat;
         bool pixel_clock_select;
         bool internal_palette_size;
         } mode_ctrl;
       } attribute_ctrl;

    struct {
       bool color_emulation;  // 1=color emulation, base address = 3Dx
                                 // 0=mono emulation,  base address = 3Bx
       bool enable_ram;       // enable CPU access to video memory if set
       u8   clock_select;     // 0=25Mhz 1=28Mhz
       bool select_high_bank; // when in odd/even modes, select
                                 // high 64k bank if set
       bool horiz_sync_pol;   // bit6: negative if set
       bool vert_sync_pol;    // bit7: negative if set
                                 //   bit7,bit6 represent number of lines on display:
                                 //   0 = reserved
                                 //   1 = 400 lines
                                 //   2 = 350 lines
                                 //   3 - 480 lines
       } misc_output;
    
    struct {
       u8   index;
       u8   map_mask;
       bool map_mask_bit[4];
       bool reset1;
       bool reset2;
       u8   reg1;
       u8   char_map_select;
       bool extended_mem;
       bool odd_even;
       bool chain_four;
       } sequencer;

     struct {
       u8 write_data_register;
       u8 write_data_cycle; /* 0, 1, 2 */
       u8 read_data_register;
       u8 read_data_cycle; /* 0, 1, 2 */
       u8 dac_state;
       struct {
         u8 red;
         u8 green;
         u8 blue;
         } data[256];
       u8 mask;
       } pel;

     struct {
       u8   index;
       u8   set_reset;
       u8   enable_set_reset;
       u8   color_compare;
       u8   data_rotate;
       u8   raster_op;
       u8   read_map_select;
       u8   write_mode;
       bool read_mode;
       bool odd_even;
       bool chain_odd_even;
       u8   shift_reg;
       bool graphics_alpha;
       u8   memory_mapping; /* 0 = use A0000-BFFFF
                                * 1 = use A0000-AFFFF EGA/VGA graphics modes
                                * 2 = use B0000-B7FFF Monochrome modes
                                * 3 = use B8000-BFFFF CGA modes
                                */
       u8   color_dont_care;
       u8   bitmask;
       u8   latch[4];
       } graphics_ctrl;

     struct {
       u8   address;
       u8   reg[0x20];
       bool write_protect;
       } CRTC;
      // generic register range.  Basically, we've got to
      // cover 0x3c0 -> 0x3df or 32 addresses.
      u8 port_data[32];

      // this macro allows us to write things like
      // VGA_PORT(0x3c0) = data; 
      // without having to do the math every time.
#define VGA_PORT(id) state.port_data[id-VGA_BASE]

      // indexed ports
      u8 crtc_index;
      u64 crtc_data[CRTC_MAX];


      // pallette information 
      u8 pel_mode;
#define PEL_MODE_READ 3
#define PEL_MODE_WRITE 0
      u8 pel_data_count;
      u8 pel_data[256][3];

      // this is the attribute controller.
      u8 p3c0_mode;
#define P3C0_ADDRESS 0
#define P3C0_DATA 0
      u8 p3c0_index;
      u8 p3c0_data[20];      


      // sequencer
      u8 seq_data[8];

      // graphics
      u8 graphics_data[9];

      u8  video_mode;
#define MODE_TEXT 0

      u64 cursor_ttl;
#define BLINK_RATE 1

      u8 reg_3c2;

      u64 refresh_ttl;
#define REFRESH_RATE 100

//      SDL_Surface *screen;

    } state;
};

#endif // !defined(INCLUDED_S3Trio64_H_)

/*  ES40 emulator.
 *
 *  This file is based upon Bochs.
 *
 *  Copyright (C) 2002  MandrakeSoft S.A.
 *
 *    MandrakeSoft S.A.
 *    43, rue d'Aboukir
 *    75002 Paris - France
 *    http://www.linux-mandrake.com/
 *    http://www.mandrakesoft.com/
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

/**
 *
 * X-1.2        Camiel Vanderhoeven                             7-DEC-2007
 *      Code cleanup.
 *
 * X-1.1        Camiel Vanderhoeven                             6-DEC-2007
 *      Initial version for ES40 emulator.
 *
 **/

#ifndef BX_IODEV_VGA_H
#define BX_IODEV_VGA_H

// Make colour
#define MAKE_COLOUR(red, red_shiftfrom, red_shiftto, red_mask, \
                    green, green_shiftfrom, green_shiftto, green_mask, \
                    blue, blue_shiftfrom, blue_shiftto, blue_mask) \
( \
 ((((red_shiftto) > (red_shiftfrom)) ? \
  (red) << ((red_shiftto) - (red_shiftfrom)) : \
  (red) >> ((red_shiftfrom) - (red_shiftto))) & \
  (red_mask)) | \
 ((((green_shiftto) > (green_shiftfrom)) ? \
  (green) << ((green_shiftto) - (green_shiftfrom)) : \
  (green) >> ((green_shiftfrom) - (green_shiftto))) & \
  (green_mask)) | \
 ((((blue_shiftto) > (blue_shiftfrom)) ? \
  (blue) << ((blue_shiftto) - (blue_shiftfrom)) : \
  (blue) >> ((blue_shiftfrom) - (blue_shiftto))) & \
  (blue_mask)) \
)

#if BX_SUPPORT_VBE
  #define VBE_DISPI_TOTAL_VIDEO_MEMORY_MB 8
  #define VBE_DISPI_4BPP_PLANE_SHIFT      21

  #define VBE_DISPI_BANK_ADDRESS          0xA0000
  #define VBE_DISPI_BANK_SIZE_KB          64

  #define VBE_DISPI_MAX_XRES              1600
  #define VBE_DISPI_MAX_YRES              1200
  #define VBE_DISPI_MAX_BPP               32

  #define VBE_DISPI_IOPORT_INDEX          0x01CE
  #define VBE_DISPI_IOPORT_DATA           0x01CF

  #define VBE_DISPI_IOPORT_INDEX_OLD      0xFF80
  #define VBE_DISPI_IOPORT_DATA_OLD       0xFF81

  #define VBE_DISPI_INDEX_ID              0x0
  #define VBE_DISPI_INDEX_XRES            0x1
  #define VBE_DISPI_INDEX_YRES            0x2
  #define VBE_DISPI_INDEX_BPP             0x3
  #define VBE_DISPI_INDEX_ENABLE          0x4
  #define VBE_DISPI_INDEX_BANK            0x5
  #define VBE_DISPI_INDEX_VIRT_WIDTH      0x6
  #define VBE_DISPI_INDEX_VIRT_HEIGHT     0x7
  #define VBE_DISPI_INDEX_X_OFFSET        0x8
  #define VBE_DISPI_INDEX_Y_OFFSET        0x9

  #define VBE_DISPI_ID0                   0xB0C0
  #define VBE_DISPI_ID1                   0xB0C1
  #define VBE_DISPI_ID2                   0xB0C2
  #define VBE_DISPI_ID3                   0xB0C3
  #define VBE_DISPI_ID4                   0xB0C4

  #define VBE_DISPI_BPP_4                 0x04
  #define VBE_DISPI_BPP_8                 0x08
  #define VBE_DISPI_BPP_15                0x0F
  #define VBE_DISPI_BPP_16                0x10
  #define VBE_DISPI_BPP_24                0x18
  #define VBE_DISPI_BPP_32                0x20

  #define VBE_DISPI_DISABLED              0x00
  #define VBE_DISPI_ENABLED               0x01
  #define VBE_DISPI_GETCAPS               0x02
  #define VBE_DISPI_8BIT_DAC              0x20
  #define VBE_DISPI_LFB_ENABLED           0x40
  #define VBE_DISPI_NOCLEARMEM            0x80

  #define VBE_DISPI_LFB_PHYSICAL_ADDRESS  0xE0000000

  
#define VBE_DISPI_TOTAL_VIDEO_MEMORY_KB		(VBE_DISPI_TOTAL_VIDEO_MEMORY_MB * 1024)
#define VBE_DISPI_TOTAL_VIDEO_MEMORY_BYTES 	(VBE_DISPI_TOTAL_VIDEO_MEMORY_KB * 1024)

#define BX_MAX_XRES VBE_DISPI_MAX_XRES
#define BX_MAX_YRES VBE_DISPI_MAX_YRES

#elif BX_SUPPORT_CLGD54XX

#define BX_MAX_XRES 1280
#define BX_MAX_YRES 1024

#else

#define BX_MAX_XRES 800
#define BX_MAX_YRES 600

#endif //BX_SUPPORT_VBE

#define X_TILESIZE 16
#define Y_TILESIZE 24
#define BX_NUM_X_TILES (BX_MAX_XRES /X_TILESIZE)
#define BX_NUM_Y_TILES (BX_MAX_YRES /Y_TILESIZE)

// Support varying number of rows of text.  This used to
// be limited to only 25 lines.
#define BX_MAX_TEXT_LINES 100

#if BX_USE_VGA_SMF
#  define BX_VGA_SMF  static
#  define BX_VGA_THIS theVga->
#else
#  define BX_VGA_SMF
#  define BX_VGA_THIS this->
#endif

//class bx_vga_c : public bx_vga_stub_c {
//public:
//  bx_vga_c();
//  virtual ~bx_vga_c();
//  virtual void   init(void);
//  virtual void   reset(unsigned type);
//  BX_VGA_SMF bool mem_read_handler(unsigned long addr, unsigned long len, void *data, void *param);
//  BX_VGA_SMF bool mem_write_handler(unsigned long addr, unsigned long len, void *data, void *param);
//  virtual u8  mem_read(u32 addr);
//  virtual void   mem_write(u32 addr, u8 value);
//  virtual void   trigger_timer(void *this_ptr);
//  virtual void   dump_status(void);
//#if BX_SUPPORT_SAVE_RESTORE
//  virtual void   register_state(void);
//  virtual void   after_restore_state(void);
//#endif
//
//#if BX_SUPPORT_VBE
//  BX_VGA_SMF u8  vbe_mem_read(u32 addr) BX_CPP_AttrRegparmN(1);
//  BX_VGA_SMF void   vbe_mem_write(u32 addr, u8 value) BX_CPP_AttrRegparmN(2);
//#endif
//
//  virtual void   redraw_area(unsigned x0, unsigned y0,
//                             unsigned width, unsigned height);
//
//  virtual void   get_text_snapshot(u8 **text_snapshot, unsigned *txHeight,
//                                   unsigned *txWidth);
//  virtual u8  get_actl_palette_idx(u8 index);
//
//protected:
//  void init_iohandlers(bx_read_handler_t f_read, bx_write_handler_t f_write);
//  void init_systemtimer(bx_timer_handler_t f_timer, param_event_handler f_param);
//
//  static u32 read_handler(void *this_ptr, u32 address, unsigned io_len);
//  static void   write_handler(void *this_ptr, u32 address, u32 value, unsigned io_len);
//  static void   write_handler_no_log(void *this_ptr, u32 address, u32 value, unsigned io_len);
//
//#if BX_SUPPORT_VBE
//  static u32 vbe_read_handler(void *this_ptr, u32 address, unsigned io_len);
//  static void   vbe_write_handler(void *this_ptr, u32 address, u32 value, unsigned io_len);
//#endif
//
//  struct {
//    struct {
//      bool color_emulation;  // 1=color emulation, base address = 3Dx
//                                // 0=mono emulation,  base address = 3Bx
//      bool enable_ram;       // enable CPU access to video memory if set
//      u8   clock_select;     // 0=25Mhz 1=28Mhz
//      bool select_high_bank; // when in odd/even modes, select
//                                // high 64k bank if set
//      bool horiz_sync_pol;   // bit6: negative if set
//      bool vert_sync_pol;    // bit7: negative if set
//                                //   bit7,bit6 represent number of lines on display:
//                                //   0 = reserved
//                                //   1 = 400 lines
//                                //   2 = 350 lines
//                                //   3 - 480 lines
//    } misc_output;
//
//    struct {
//      u8   address;
//      u8   reg[0x19];
//      bool write_protect;
//    } CRTC;
//
//    struct {
//      bool  flip_flop; /* 0 = address, 1 = data-write */
//      unsigned address;  /* register number */
//      bool  video_enabled;
//      u8    palette_reg[16];
//      u8    overscan_color;
//      u8    color_plane_enable;
//      u8    horiz_pel_panning;
//      u8    color_select;
//      struct {
//        bool graphics_alpha;
//        bool display_type;
//        bool enable_line_graphics;
//        bool blink_intensity;
//        bool pixel_panning_compat;
//        bool pixel_clock_select;
//        bool internal_palette_size;
//      } mode_ctrl;
//    } attribute_ctrl;
//
//    struct {
//      u8 write_data_register;
//      u8 write_data_cycle; /* 0, 1, 2 */
//      u8 read_data_register;
//      u8 read_data_cycle; /* 0, 1, 2 */
//      u8 dac_state;
//      struct {
//        u8 red;
//        u8 green;
//        u8 blue;
//      } data[256];
//      u8 mask;
//    } pel;
//
//    struct {
//      u8   index;
//      u8   set_reset;
//      u8   enable_set_reset;
//      u8   color_compare;
//      u8   data_rotate;
//      u8   raster_op;
//      u8   read_map_select;
//      u8   write_mode;
//      bool read_mode;
//      bool odd_even;
//      bool chain_odd_even;
//      u8   shift_reg;
//      bool graphics_alpha;
//      u8   memory_mapping; /* 0 = use A0000-BFFFF
//                               * 1 = use A0000-AFFFF EGA/VGA graphics modes
//                               * 2 = use B0000-B7FFF Monochrome modes
//                               * 3 = use B8000-BFFFF CGA modes
//                               */
//      u8   color_dont_care;
//      u8   bitmask;
//      u8   latch[4];
//    } graphics_ctrl;
//
//    struct {
//      u8   index;
//      u8   map_mask;
//      bool reset1;
//      bool reset2;
//      u8   reg1;
//      u8   char_map_select;
//      bool extended_mem;
//      bool odd_even;
//      bool chain_four;
//    } sequencer;
//
//    bool  vga_enabled;
//    bool  vga_mem_updated;
//    unsigned x_tilesize;
//    unsigned y_tilesize;
//    unsigned line_offset;
//    unsigned line_compare;
//    unsigned vertical_display_end;
//    bool  vga_tile_updated[BX_NUM_X_TILES][BX_NUM_Y_TILES];
//    u8 *memory;
//    u32 memsize;
//    u8 text_snapshot[128 * 1024]; // current text snapshot
//    u8 tile[X_TILESIZE * Y_TILESIZE * 4]; /**< Currently allocates the tile as large as needed. */
//    u16 charmap_address;
//    bool x_dotclockdiv2;
//    bool y_doublescan;
//    u8 last_bpp;
//
//#if BX_SUPPORT_VBE    
//    u16  vbe_cur_dispi;
//    u16  vbe_xres;
//    u16  vbe_yres;
//    u16  vbe_bpp;
//    u16  vbe_max_xres;
//    u16  vbe_max_yres;
//    u16  vbe_max_bpp;
//    u16  vbe_bank;
//    bool vbe_enabled;
//    u16  vbe_curindex;
//    u32  vbe_visible_screen_size; /**< in bytes */
//    u16  vbe_offset_x;		 /**< Virtual screen x start (in pixels) */ 
//    u16  vbe_offset_y;		 /**< Virtual screen y start (in pixels) */
//    u16  vbe_virtual_xres;
//    u16  vbe_virtual_yres;
//    u32  vbe_virtual_start;   /**< For dealing with bpp>8, this is where the virtual screen starts. */
//    u8   vbe_bpp_multiplier;  /**< We have to save this b/c sometimes we need to recalculate stuff with it. */
//    bool vbe_lfb_enabled;
//    bool vbe_get_capabilities;
//    bool vbe_8bit_dac;
//#endif    
//  } s;  // state information
//
//
//#if !BX_USE_VGA_SMF
//  u32 read(u32 address, unsigned io_len);
//  void  write(u32 address, u32 value, unsigned io_len, bool no_log);
//#else
//  void write(u32 address, u32 value, unsigned io_len, bool no_log);
//#endif
//
//#if BX_SUPPORT_VBE
//
//#if !BX_USE_VGA_SMF
//  u32 vbe_read(u32 address, unsigned io_len);
//  void  vbe_write(u32 address, u32 value, unsigned io_len, bool no_log);
//#else
//  void vbe_write(u32 address, u32 value, unsigned io_len, bool no_log);
//#endif
//#endif
//
//  int timer_id;
//  bool extension_init;
//  bool extension_checked;
//
//  public:
//  static void     timer_handler(void *);
//  BX_VGA_SMF void timer(void);
//  static Bit64s   vga_param_handler(bx_param_c *param, int set, Bit64s val);
//
//  protected:
//  BX_VGA_SMF void update(void);
//  BX_VGA_SMF void determine_screen_dimensions(unsigned *piHeight,
//                                              unsigned *piWidth);
//};
//
//#if BX_SUPPORT_CLGD54XX
//void libvga_set_smf_pointer(bx_vga_c *theVga_ptr);
//#include "iodev/svga_cirrus.h"
//#endif

#endif

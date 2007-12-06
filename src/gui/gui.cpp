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
 * X-1.1        Camiel Vanderhoeven                             6-DEC-2007
 *      Initial version for ES40 emulator.
 *
 **/

#include "StdAfx.h"

#include <signal.h>

#include "gui.h"


//#include "bochs.h"
//#include "iodev.h"
//#include "gui/bitmaps/floppya.h"
//#include "gui/bitmaps/floppyb.h"
//#include "gui/bitmaps/mouse.h"
//#include "gui/bitmaps/reset.h"
//#include "gui/bitmaps/power.h"
//#include "gui/bitmaps/snapshot.h"
//#include "gui/bitmaps/copy.h"
//#include "gui/bitmaps/paste.h"
//#include "gui/bitmaps/configbutton.h"
//#include "gui/bitmaps/cdromd.h"
//#include "gui/bitmaps/userbutton.h"
#if BX_SUPPORT_SAVE_RESTORE
#include "gui/bitmaps/saverestore.h"
#endif

#if BX_WITH_MACOS
#  include <Disks.h>
#endif

bx_gui_c *bx_gui = NULL;

#define BX_GUI_THIS bx_gui->
#define LOG_THIS BX_GUI_THIS

#define BX_KEY_UNKNOWN 0x7fffffff
#define N_USER_KEYS 36

typedef struct {
  char *key;
  Bit32u symbol;
} user_key_t;

static user_key_t user_keys[N_USER_KEYS] =
{
  { "f1",    BX_KEY_F1 },
  { "f2",    BX_KEY_F2 },
  { "f3",    BX_KEY_F3 },
  { "f4",    BX_KEY_F4 },
  { "f5",    BX_KEY_F5 },
  { "f6",    BX_KEY_F6 },
  { "f7",    BX_KEY_F7 },
  { "f8",    BX_KEY_F8 },
  { "f9",    BX_KEY_F9 },
  { "f10",   BX_KEY_F10 },
  { "f11",   BX_KEY_F11 },
  { "f12",   BX_KEY_F12 },
  { "alt",   BX_KEY_ALT_L },
  { "bksl",  BX_KEY_BACKSLASH },
  { "bksp",  BX_KEY_BACKSPACE },
  { "ctrl",  BX_KEY_CTRL_L },
  { "del",   BX_KEY_DELETE },
  { "down",  BX_KEY_DOWN },
  { "end",   BX_KEY_END },
  { "enter", BX_KEY_ENTER },
  { "esc",   BX_KEY_ESC },
  { "home",  BX_KEY_HOME },
  { "ins",   BX_KEY_INSERT },
  { "left",  BX_KEY_LEFT },
  { "menu",  BX_KEY_MENU },
  { "minus", BX_KEY_MINUS },
  { "pgdwn", BX_KEY_PAGE_DOWN },
  { "pgup",  BX_KEY_PAGE_UP },
  { "plus",  BX_KEY_KP_ADD },
  { "right", BX_KEY_RIGHT },
  { "shift", BX_KEY_SHIFT_L },
  { "space", BX_KEY_SPACE },
  { "tab",   BX_KEY_TAB },
  { "up",    BX_KEY_UP },
  { "win",   BX_KEY_WIN_L },
  { "print", BX_KEY_PRINT }
};

bx_gui_c::bx_gui_c(void)
{
  framebuffer = NULL;
}

bx_gui_c::~bx_gui_c()
{
  if (framebuffer != NULL) {
    delete [] framebuffer;
  }
}

void bx_gui_c::init(int argc, char **argv, unsigned tilewidth, unsigned tileheight)
{
  BX_GUI_THIS new_gfx_api = 0;
  BX_GUI_THIS host_xres = 640;
  BX_GUI_THIS host_yres = 480;
  BX_GUI_THIS host_bpp = 8;
  //BX_GUI_THIS dialog_caps = BX_GUI_DLG_RUNTIME | BX_GUI_DLG_SAVE_RESTORE;

  //specific_init(argc, argv, tilewidth, tileheight, BX_HEADER_BAR_Y);

  // Define some bitmaps to use in the headerbar
  //BX_GUI_THIS floppyA_bmap_id = create_bitmap(bx_floppya_bmap,
  //                        BX_FLOPPYA_BMAP_X, BX_FLOPPYA_BMAP_Y);
  //BX_GUI_THIS floppyA_eject_bmap_id = create_bitmap(bx_floppya_eject_bmap,
  //                        BX_FLOPPYA_BMAP_X, BX_FLOPPYA_BMAP_Y);
  //BX_GUI_THIS floppyB_bmap_id = create_bitmap(bx_floppyb_bmap,
  //                        BX_FLOPPYB_BMAP_X, BX_FLOPPYB_BMAP_Y);
  //BX_GUI_THIS floppyB_eject_bmap_id = create_bitmap(bx_floppyb_eject_bmap,
  //                        BX_FLOPPYB_BMAP_X, BX_FLOPPYB_BMAP_Y);
  //BX_GUI_THIS cdromD_bmap_id = create_bitmap(bx_cdromd_bmap,
  //                        BX_CDROMD_BMAP_X, BX_CDROMD_BMAP_Y);
  //BX_GUI_THIS cdromD_eject_bmap_id = create_bitmap(bx_cdromd_eject_bmap,
  //                        BX_CDROMD_BMAP_X, BX_CDROMD_BMAP_Y);
  //BX_GUI_THIS mouse_bmap_id = create_bitmap(bx_mouse_bmap,
  //                        BX_MOUSE_BMAP_X, BX_MOUSE_BMAP_Y);
  //BX_GUI_THIS nomouse_bmap_id = create_bitmap(bx_nomouse_bmap,
  //                        BX_MOUSE_BMAP_X, BX_MOUSE_BMAP_Y);

  //BX_GUI_THIS power_bmap_id = create_bitmap(bx_power_bmap, BX_POWER_BMAP_X, BX_POWER_BMAP_Y);
  //BX_GUI_THIS reset_bmap_id = create_bitmap(bx_reset_bmap, BX_RESET_BMAP_X, BX_RESET_BMAP_Y);
  //BX_GUI_THIS snapshot_bmap_id = create_bitmap(bx_snapshot_bmap, BX_SNAPSHOT_BMAP_X, BX_SNAPSHOT_BMAP_Y);
  //BX_GUI_THIS copy_bmap_id = create_bitmap(bx_copy_bmap, BX_COPY_BMAP_X, BX_COPY_BMAP_Y);
  //BX_GUI_THIS paste_bmap_id = create_bitmap(bx_paste_bmap, BX_PASTE_BMAP_X, BX_PASTE_BMAP_Y);
  //BX_GUI_THIS config_bmap_id = create_bitmap(bx_config_bmap, BX_CONFIG_BMAP_X, BX_CONFIG_BMAP_Y);
  //BX_GUI_THIS user_bmap_id = create_bitmap(bx_user_bmap, BX_USER_BMAP_X, BX_USER_BMAP_Y);

//#if BX_SUPPORT_SAVE_RESTORE
//  BX_GUI_THIS save_restore_bmap_id = create_bitmap(bx_save_restore_bmap,
//                          BX_SAVE_RESTORE_BMAP_X, BX_SAVE_RESTORE_BMAP_Y);
//#endif
//
//  // Add the initial bitmaps to the headerbar, and enable callback routine, for use
//  // when that bitmap is clicked on
//
//  // Floppy A:
//  BX_GUI_THIS floppyA_status = DEV_floppy_get_media_status(0);
//  if (BX_GUI_THIS floppyA_status)
//    BX_GUI_THIS floppyA_hbar_id = headerbar_bitmap(BX_GUI_THIS floppyA_bmap_id,
//                          BX_GRAVITY_LEFT, floppyA_handler);
//  else
//    BX_GUI_THIS floppyA_hbar_id = headerbar_bitmap(BX_GUI_THIS floppyA_eject_bmap_id,
//                          BX_GRAVITY_LEFT, floppyA_handler);
//  BX_GUI_THIS set_tooltip(BX_GUI_THIS floppyA_hbar_id, "Change floppy A: media");
//
//  // Floppy B:
//  BX_GUI_THIS floppyB_status = DEV_floppy_get_media_status(1);
//  if (BX_GUI_THIS floppyB_status)
//    BX_GUI_THIS floppyB_hbar_id = headerbar_bitmap(BX_GUI_THIS floppyB_bmap_id,
//                          BX_GRAVITY_LEFT, floppyB_handler);
//  else
//    BX_GUI_THIS floppyB_hbar_id = headerbar_bitmap(BX_GUI_THIS floppyB_eject_bmap_id,
//                          BX_GRAVITY_LEFT, floppyB_handler);
//  BX_GUI_THIS set_tooltip(BX_GUI_THIS floppyB_hbar_id, "Change floppy B: media");
//
//  // CDROM, 
//  // the harddrive object is not initialised yet,
//  // so we just set the bitmap to ejected for now
//  BX_GUI_THIS cdromD_hbar_id = headerbar_bitmap(BX_GUI_THIS cdromD_eject_bmap_id,
//                          BX_GRAVITY_LEFT, cdromD_handler);
//  BX_GUI_THIS set_tooltip(BX_GUI_THIS cdromD_hbar_id, "Change first CDROM media");
//
//  // Mouse button
//  if (SIM->get_param_bool(BXPN_MOUSE_ENABLED)->get())
//    BX_GUI_THIS mouse_hbar_id = headerbar_bitmap(BX_GUI_THIS mouse_bmap_id,
//                          BX_GRAVITY_LEFT, toggle_mouse_enable);
//  else
//    BX_GUI_THIS mouse_hbar_id = headerbar_bitmap(BX_GUI_THIS nomouse_bmap_id,
//                          BX_GRAVITY_LEFT, toggle_mouse_enable);
//  BX_GUI_THIS set_tooltip(BX_GUI_THIS mouse_hbar_id, "Enable mouse capture");
//
//  // These are the buttons on the right side.  They are created in order
//  // of right to left.
//
//  // Power button
//  BX_GUI_THIS power_hbar_id = headerbar_bitmap(BX_GUI_THIS power_bmap_id,
//                          BX_GRAVITY_RIGHT, power_handler);
//  BX_GUI_THIS set_tooltip(BX_GUI_THIS power_hbar_id, "Turn power off");
//  // Save/Restore Button
//#if BX_SUPPORT_SAVE_RESTORE
//  BX_GUI_THIS save_restore_hbar_id = headerbar_bitmap(BX_GUI_THIS save_restore_bmap_id,
//                          BX_GRAVITY_RIGHT, save_restore_handler);
//  BX_GUI_THIS set_tooltip(BX_GUI_THIS save_restore_hbar_id, "Save simulation state");
//#endif
//  // Reset button
//  BX_GUI_THIS reset_hbar_id = headerbar_bitmap(BX_GUI_THIS reset_bmap_id,
//                          BX_GRAVITY_RIGHT, reset_handler);
//  BX_GUI_THIS set_tooltip(BX_GUI_THIS reset_hbar_id, "Reset the system");
//  // Configure button
//  BX_GUI_THIS config_hbar_id = headerbar_bitmap(BX_GUI_THIS config_bmap_id,
//                          BX_GRAVITY_RIGHT, config_handler);
//  BX_GUI_THIS set_tooltip(BX_GUI_THIS config_hbar_id, "Runtime config dialog");
//  // Snapshot button
//  BX_GUI_THIS snapshot_hbar_id = headerbar_bitmap(BX_GUI_THIS snapshot_bmap_id,
//                          BX_GRAVITY_RIGHT, snapshot_handler);
//  BX_GUI_THIS set_tooltip(BX_GUI_THIS snapshot_hbar_id, "Save snapshot of the text mode screen");
//  // Paste button
//  BX_GUI_THIS paste_hbar_id = headerbar_bitmap(BX_GUI_THIS paste_bmap_id,
//                          BX_GRAVITY_RIGHT, paste_handler);
//  BX_GUI_THIS set_tooltip(BX_GUI_THIS paste_hbar_id, "Paste clipboard text as emulated keystrokes");
//  // Copy button
//  BX_GUI_THIS copy_hbar_id = headerbar_bitmap(BX_GUI_THIS copy_bmap_id,
//                          BX_GRAVITY_RIGHT, copy_handler);
//  BX_GUI_THIS set_tooltip(BX_GUI_THIS copy_hbar_id, "Copy text mode screen to the clipboard");
//  // User button
//  BX_GUI_THIS user_hbar_id = headerbar_bitmap(BX_GUI_THIS user_bmap_id,
//                          BX_GRAVITY_RIGHT, userbutton_handler);
//  BX_GUI_THIS set_tooltip(BX_GUI_THIS user_hbar_id, "Send keyboard shortcut");
//
//  if (SIM->get_param_bool(BXPN_TEXT_SNAPSHOT_CHECK)->get()) {
//    bx_pc_system.register_timer(this, bx_gui_c::snapshot_checker, (unsigned) 1000000, 1, 1, "snap_chk");
//  }

  BX_GUI_THIS charmap_updated = 0;

  if (!BX_GUI_THIS new_gfx_api && (BX_GUI_THIS framebuffer == NULL)) {
    BX_GUI_THIS framebuffer = new Bit8u[BX_MAX_XRES * BX_MAX_YRES * 4];
  }
  //show_headerbar();
}

void bx_gui_c::cleanup(void)
{
}

Bit32u get_user_key(char *key)
{
  int i = 0;

  while (i < N_USER_KEYS) {
    if (!strcmp(key, user_keys[i].key))
      return user_keys[i].symbol;
    i++;
  }
  return BX_KEY_UNKNOWN;
}

void bx_gui_c::mouse_enabled_changed (bx_bool val)
{
  // This is only called when SIM->get_init_done is 1.  Note that VAL
  // is the new value of mouse_enabled, which may not match the old
  // value which is still in SIM->get_param_bool(BXPN_MOUSE_ENABLED)->get().
  //BX_DEBUG (("replacing the mouse bitmaps"));
  //if (val)
  //  BX_GUI_THIS replace_bitmap(BX_GUI_THIS mouse_hbar_id, BX_GUI_THIS mouse_bmap_id);
  //else
  //  BX_GUI_THIS replace_bitmap(BX_GUI_THIS mouse_hbar_id, BX_GUI_THIS nomouse_bmap_id);
  // give the GUI a chance to respond to the event.  Most guis will hide
  // the native mouse cursor and do something to trap the mouse inside the
  // bochs VGA display window.
  BX_GUI_THIS mouse_enabled_changed_specific (val);
}

void bx_gui_c::init_signal_handlers()
{
#if BX_GUI_SIGHANDLER
  if (bx_gui_sighandler) 
  {
    Bit32u mask = bx_gui->get_sighandler_mask ();
    for (Bit32u sig=0; sig<32; sig++)
    {
      if (mask & (1<<sig))
        signal (sig, bx_signal_handler);
    }
  }
#endif
}

void bx_gui_c::set_text_charmap(Bit8u *fbuffer)
{
  memcpy(& BX_GUI_THIS vga_charmap, fbuffer, 0x2000);
  for (unsigned i=0; i<256; i++) BX_GUI_THIS char_changed[i] = 1;
  BX_GUI_THIS charmap_updated = 1;
}

void bx_gui_c::set_text_charbyte(Bit16u address, Bit8u data)
{
  BX_GUI_THIS vga_charmap[address] = data;
  BX_GUI_THIS char_changed[address >> 5] = 1;
  BX_GUI_THIS charmap_updated = 1;
}
  
void bx_gui_c::beep_on(float frequency)
{
  BX_INFO(("GUI Beep ON (frequency=%.2f)", frequency));
}

void bx_gui_c::beep_off()
{
  BX_INFO(("GUI Beep OFF"));
}

void bx_gui_c::get_capabilities(Bit16u *xres, Bit16u *yres, Bit16u *bpp)
{
  *xres = 1024;
  *yres = 768;
  *bpp = 32;
}

bx_svga_tileinfo_t *bx_gui_c::graphics_tile_info(bx_svga_tileinfo_t *info)
{
  if (!info) {
    info = (bx_svga_tileinfo_t *)malloc(sizeof(bx_svga_tileinfo_t));
    if (!info) {
      return NULL;
    }
  }

  BX_GUI_THIS host_pitch = BX_GUI_THIS host_xres * ((BX_GUI_THIS host_bpp + 1) >> 3);

  info->bpp = BX_GUI_THIS host_bpp;
  info->pitch = BX_GUI_THIS host_pitch;
  switch (info->bpp) {
    case 15:
      info->red_shift = 15;
      info->green_shift = 10;
      info->blue_shift = 5;
      info->red_mask = 0x7c00;
      info->green_mask = 0x03e0;
      info->blue_mask = 0x001f;
      break;
    case 16:
      info->red_shift = 16;
      info->green_shift = 11;
      info->blue_shift = 5;
      info->red_mask = 0xf800;
      info->green_mask = 0x07e0;
      info->blue_mask = 0x001f;
      break;
    case 24:
    case 32:
      info->red_shift = 24;
      info->green_shift = 16;
      info->blue_shift = 8;
      info->red_mask = 0xff0000;
      info->green_mask = 0x00ff00;
      info->blue_mask = 0x0000ff;
      break;
  }
  info->is_indexed = (BX_GUI_THIS host_bpp == 8);
#ifdef BX_LITTLE_ENDIAN
  info->is_little_endian = 1;
#else
  info->is_little_endian = 0;
#endif

  return info;
}

Bit8u *bx_gui_c::graphics_tile_get(unsigned x0, unsigned y0,
                            unsigned *w, unsigned *h)
{
  if (x0+X_TILESIZE > BX_GUI_THIS host_xres) {
    *w = BX_GUI_THIS host_xres - x0;
  }
  else {
    *w = X_TILESIZE;
  }

  if (y0+Y_TILESIZE > BX_GUI_THIS host_yres) {
    *h = BX_GUI_THIS host_yres - y0;
  }
  else {
    *h = Y_TILESIZE;
  }

  return (Bit8u *)framebuffer + y0 * BX_GUI_THIS host_pitch +
                  x0 * ((BX_GUI_THIS host_bpp + 1) >> 3);
}

void bx_gui_c::graphics_tile_update_in_place(unsigned x0, unsigned y0,
                                        unsigned w, unsigned h)
{
  Bit8u tile[X_TILESIZE * Y_TILESIZE * 4];
  Bit8u *tile_ptr, *fb_ptr;
  Bit16u xc, yc, fb_pitch, tile_pitch;
  Bit8u r, diffx, diffy;

  diffx = (x0 % X_TILESIZE);
  diffy = (y0 % Y_TILESIZE);
  if (diffx > 0) {
    x0 -= diffx;
    w += diffx;
  }
  if (diffy > 0) {
    y0 -= diffy;
    h += diffy;
  }
  fb_pitch = BX_GUI_THIS host_pitch;
  tile_pitch = X_TILESIZE * ((BX_GUI_THIS host_bpp + 1) >> 3);
  for (yc=y0; yc<(y0+h); yc+=Y_TILESIZE) {
    for (xc=x0; xc<(x0+w); xc+=X_TILESIZE) {
      fb_ptr = BX_GUI_THIS framebuffer + (yc * fb_pitch + xc * ((BX_GUI_THIS host_bpp + 1) >> 3));
      tile_ptr = &tile[0];
      for (r=0; r<h; r++) {
        memcpy(tile_ptr, fb_ptr, tile_pitch);
        fb_ptr += fb_pitch;
        tile_ptr += tile_pitch;
      }
      BX_GUI_THIS graphics_tile_update(tile, xc, yc);
    }
  }
}

void bx_gui_c::show_ips(Bit32u ips_count)
{
#if BX_SHOW_IPS
  BX_INFO(("ips = %u", ips_count));
#endif
}

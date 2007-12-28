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
 * Contains the code for emulated S3 Trio 64 Video Card device.
 *
 * X-1.7        Camiel Vanderhoeven                             28-DEC-2007
 *      Keep the compiler happy.
 *
 * X-1.6        Camiel Vanderhoeven                             17-DEC-2007
 *      SaveState file format 2.1
 *
 * X-1.5        Brian Wheeler                                   10-DEC-2007
 *      Made refresh function name unique.
 *
 * X-1.4        Camiel Vanderhoeven                             10-DEC-2007
 *      Use new base class VGA.
 *
 * X-1.3        Camiel Vanderhoeven                             7-DEC-2007
 *      Code cleanup.
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

#include "StdAfx.h"
#include "S3Trio64.h"
#include "System.h"
#include "AliM1543C.h"
#include "gui/gui.h"

#ifndef _WIN32
#include "unistd.h"
#include "pthread.h"
#include "signal.h"
#endif

static unsigned old_iHeight = 0, old_iWidth = 0, old_MSL = 0;

static const u8 ccdat[16][4] = {
  { 0x00, 0x00, 0x00, 0x00 },
  { 0xff, 0x00, 0x00, 0x00 },
  { 0x00, 0xff, 0x00, 0x00 },
  { 0xff, 0xff, 0x00, 0x00 },
  { 0x00, 0x00, 0xff, 0x00 },
  { 0xff, 0x00, 0xff, 0x00 },
  { 0x00, 0xff, 0xff, 0x00 },
  { 0xff, 0xff, 0xff, 0x00 },
  { 0x00, 0x00, 0x00, 0xff },
  { 0xff, 0x00, 0x00, 0xff },
  { 0x00, 0xff, 0x00, 0xff },
  { 0xff, 0xff, 0x00, 0xff },
  { 0x00, 0x00, 0xff, 0xff },
  { 0xff, 0x00, 0xff, 0xff },
  { 0x00, 0xff, 0xff, 0xff },
  { 0xff, 0xff, 0xff, 0xff },
};

// Only reference the array if the tile numbers are within the bounds
// of the array.  If out of bounds, do nothing.
#define SET_TILE_UPDATED(xtile,ytile,value)                              \
  do {                                                                   \
    if (((xtile) < BX_NUM_X_TILES) && ((ytile) < BX_NUM_Y_TILES))        \
      state.vga_tile_updated[(xtile)][(ytile)] = value;          \
  } while (0)

// Only reference the array if the tile numbers are within the bounds
// of the array.  If out of bounds, return 0.
#define GET_TILE_UPDATED(xtile,ytile)                                    \
  ((((xtile) < BX_NUM_X_TILES) && ((ytile) < BX_NUM_Y_TILES))?           \
     state.vga_tile_updated[(xtile)][(ytile)]                    \
     : 0)

#if defined(_WIN32)
static HANDLE screen_refresh_handle_s3;
static DWORD WINAPI refresh_proc_s3(LPVOID lpParam)
#else
  pthread_t screen_refresh_handle_s3;
  static void *refresh_proc_s3(void *lpParam)
#endif
{
  CS3Trio64 *c = (CS3Trio64 *) lpParam;
  while(1) {
    //c->screenrefresh();
//    bx_gui->handle_events();
    c->update();
    bx_gui->flush();
    sleep_ms(100); // 10 fps
  }
  return 0;
}

static size_t rom_max;
static u8 option_rom[65536];


u32 s3_cfg_data[64] = {
/*00*/  0x88115333, // CFID: vendor + device
/*04*/  0x011f0000, // CFCS: command + status
/*08*/  0x03000002, // CFRV: class + revision
/*0c*/  0x00000000, // CFLT: latency timer + cache line size
/*10*/  0xf8000000, // BAR0: FB
/*14*/  0x00000000, // BAR1:
/*18*/  0x00000000, // BAR2: 
/*1c*/  0x00000000, // BAR3: 
/*20*/  0x00000000, // BAR4: 
/*24*/  0x00000000, // BAR5: 
/*28*/  0x00000000, // CCIC: CardBus
/*2c*/  0x00000000, // CSID: subsystem + vendor
/*30*/  0x00000000, // BAR6: expansion rom base
/*34*/  0x00000000, // CCAP: capabilities pointer
/*38*/  0x00000000,
/*3c*/  0x281401ff, // CFIT: interrupt configuration
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

u32 s3_cfg_mask[64] = {
/*00*/  0x00000000, // CFID: vendor + device
/*04*/  0x0000ffff, // CFCS: command + status
/*08*/  0x00000000, // CFRV: class + revision
/*0c*/  0x0000ffff, // CFLT: latency timer + cache line size
/*10*/  0xfc000000, // BAR0: FB
/*14*/  0x00000000, // BAR1:     
/*18*/  0x00000000, // BAR2: 
/*1c*/  0x00000000, // BAR3: 
/*20*/  0x00000000, // BAR4: 
/*24*/  0x00000000, // BAR5: 
/*28*/  0x00000000, // CCIC: CardBus
/*2c*/  0x00000000, // CSID: subsystem + vendor
/*30*/  0x00000000, // BAR6: expansion rom base
/*34*/  0x00000000, // CCAP: capabilities pointer
/*38*/  0x00000000,
/*3c*/  0x000000ff, // CFIT: interrupt configuration
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/**
 * Constructor.
 **/
CS3Trio64::CS3Trio64(CConfigurator * cfg, CSystem * c, int pcibus, int pcidev): CVGA(cfg,c,pcibus,pcidev)
{
  add_function(0,s3_cfg_data,s3_cfg_mask);

  int i;
  char *romfile;

    c->RegisterClock(this, true);

    /* the VGA I/O ports are at 3b0 -> 3df */
    add_legacy_io(2,0x3b0,48);

    /* we listen for messages from outer space (a.k.a. VGA bios) at port 500. */
    add_legacy_io(7,0x500,1);

    /* legacy video address space: A0000 -> bffff */
    add_legacy_mem(4,0xa0000,128*1024);

    ResetPCI();

    state.crtc_data[0x0a]=12;  // cursor size
    state.crtc_data[0x0b]=15;

    state.video_mode = MODE_TEXT;
    state.cursor_ttl = BLINK_RATE;

    bios_message_size = 0;
    bios_message[0] = '\0';

    // use a VGA rom from bochs
    romfile=myCfg->get_text_value("vga.rom","vgabios.bin");
    FILE *rom=fopen(romfile,"rb");
    if(!rom) {
      printf("%%VGA-F-ROM: Cannot open %s\n",romfile);
      exit(1);
    }
    rom_max=fread(option_rom,1,65536,rom);
    fclose(rom);
    printf("%%VGA-I-ROMSIZE: ROM is %d bytes.\n",rom_max);

    /* Option ROM address space: C0000  */
    add_legacy_mem(5,0xc0000,(u32)rom_max);


  state.vga_enabled = 1;
  state.misc_output.color_emulation  = 1;
  state.misc_output.enable_ram  = 1;
  state.misc_output.clock_select     = 0;
  state.misc_output.select_high_bank = 0;
  state.misc_output.horiz_sync_pol   = 1;
  state.misc_output.vert_sync_pol    = 1;

  state.attribute_ctrl.mode_ctrl.graphics_alpha = 0;
  state.attribute_ctrl.mode_ctrl.display_type = 0;
  state.attribute_ctrl.mode_ctrl.enable_line_graphics = 1;
  state.attribute_ctrl.mode_ctrl.blink_intensity = 0;
  state.attribute_ctrl.mode_ctrl.pixel_panning_compat = 0;
  state.attribute_ctrl.mode_ctrl.pixel_clock_select = 0;
  state.attribute_ctrl.mode_ctrl.internal_palette_size = 0;

  state.line_offset=80;
  state.line_compare=1023;
  state.vertical_display_end=399;

  for (i=0; i<=0x18; i++)
    state.CRTC.reg[i] = 0;
  state.CRTC.address = 0;
  state.CRTC.write_protect = 0;

  state.attribute_ctrl.flip_flop = 0;
  state.attribute_ctrl.address = 0;
  state.attribute_ctrl.video_enabled = 1;
  for (i=0; i<16; i++)
    state.attribute_ctrl.palette_reg[i] = 0;
  state.attribute_ctrl.overscan_color = 0;
  state.attribute_ctrl.color_plane_enable = 0x0f;
  state.attribute_ctrl.horiz_pel_panning = 0;
  state.attribute_ctrl.color_select = 0;

  for (i=0; i<256; i++) {
    state.pel.data[i].red = 0;
    state.pel.data[i].green = 0;
    state.pel.data[i].blue = 0;
  }
  state.pel.write_data_register = 0;
  state.pel.write_data_cycle = 0;
  state.pel.read_data_register = 0;
  state.pel.read_data_cycle = 0;
  state.pel.dac_state = 0x01;
  state.pel.mask = 0xff;

  state.graphics_ctrl.index = 0;
  state.graphics_ctrl.set_reset = 0;
  state.graphics_ctrl.enable_set_reset = 0;
  state.graphics_ctrl.color_compare = 0;
  state.graphics_ctrl.data_rotate = 0;
  state.graphics_ctrl.raster_op    = 0;
  state.graphics_ctrl.read_map_select = 0;
  state.graphics_ctrl.write_mode = 0;
  state.graphics_ctrl.read_mode  = 0;
  state.graphics_ctrl.odd_even = 0;
  state.graphics_ctrl.chain_odd_even = 0;
  state.graphics_ctrl.shift_reg = 0;
  state.graphics_ctrl.graphics_alpha = 0;
  state.graphics_ctrl.memory_mapping = 2; // monochrome text mode
  state.graphics_ctrl.color_dont_care = 0;
  state.graphics_ctrl.bitmask = 0;
  for (i=0; i<4; i++) {
    state.graphics_ctrl.latch[i] = 0;
  }

  state.sequencer.index = 0;
  state.sequencer.map_mask = 0;
  state.sequencer.reset1 = 1;
  state.sequencer.reset2 = 1;
  state.sequencer.reg1 = 0;
  state.sequencer.char_map_select = 0;
  state.sequencer.extended_mem = 1; // display mem greater than 64K
  state.sequencer.odd_even = 1; // use sequential addressing mode
  state.sequencer.chain_four = 0; // use map mask & read map select

  //extname = SIM->get_param_string(BXPN_VGA_EXTENSION)->getptr();
  //if ((strlen(extname) == 0) || (!strcmp(extname, "none"))) {
    state.memsize = 0x40000;
    state.memory = new u8[state.memsize];
    memset(state.memory, 0, state.memsize);
  //}

  state.vga_mem_updated = 0;
  for (unsigned y=0; y<480/Y_TILESIZE; y++)
    for (unsigned x=0; x<640/X_TILESIZE; x++)
      SET_TILE_UPDATED (x, y, 0);

  bx_gui->init(state.x_tilesize, state.y_tilesize);

#if !BX_SUPPORT_CLGD54XX
//  this->init_systemtimer(timer_handler, vga_param_handler);
#endif // !BX_SUPPORT_CLGD54XX

  state.charmap_address = 0;
  state.x_dotclockdiv2 = 0;
  state.y_doublescan = 0;
  state.last_bpp = 8;

#if BX_SUPPORT_VBE  
    // The following is for the vbe display extension

  state.vbe_enabled=0;
  state.vbe_8bit_dac=0;
  if (!strcmp(extname, "vbe")) {
    for (addr=VBE_DISPI_IOPORT_INDEX; addr<=VBE_DISPI_IOPORT_DATA; addr++) {
      DEV_register_ioread_handler(this, vbe_read_handler, addr, "vga video", 7);
      DEV_register_iowrite_handler(this, vbe_write_handler, addr, "vga video", 7);
    }    
    if (!BX_SUPPORT_PCIUSB || !SIM->get_param_bool(BXPN_USB1_ENABLED)->get()) {
      for (addr=VBE_DISPI_IOPORT_INDEX_OLD; addr<=VBE_DISPI_IOPORT_DATA_OLD; addr++) {
        DEV_register_ioread_handler(this, vbe_read_handler, addr, "vga video", 7);
        DEV_register_iowrite_handler(this, vbe_write_handler, addr, "vga video", 7);
      }    
    }
    DEV_register_memory_handlers(theVga, mem_read_handler, mem_write_handler,
                                 VBE_DISPI_LFB_PHYSICAL_ADDRESS,
                                 VBE_DISPI_LFB_PHYSICAL_ADDRESS + VBE_DISPI_TOTAL_VIDEO_MEMORY_BYTES - 1);

    if (state.memory == NULL)
      state.memory = new u8[VBE_DISPI_TOTAL_VIDEO_MEMORY_BYTES];
    memset(state.memory, 0, VBE_DISPI_TOTAL_VIDEO_MEMORY_BYTES);
    state.memsize = VBE_DISPI_TOTAL_VIDEO_MEMORY_BYTES;
    state.vbe_cur_dispi=VBE_DISPI_ID0;
    state.vbe_xres=640;
    state.vbe_yres=480;
    state.vbe_bpp=8;
    state.vbe_bank=0;
    state.vbe_curindex=0;
    state.vbe_offset_x=0;
    state.vbe_offset_y=0;
    state.vbe_virtual_xres=640;
    state.vbe_virtual_yres=480;
    state.vbe_bpp_multiplier=1;
    state.vbe_virtual_start=0;
    state.vbe_lfb_enabled=0;
    state.vbe_get_capabilities=0;
    bx_gui->get_capabilities(&max_xres, &max_yres,
                             &max_bpp);
    if (max_xres > VBE_DISPI_MAX_XRES) {
      state.vbe_max_xres=VBE_DISPI_MAX_XRES;
    } else {
      state.vbe_max_xres=max_xres;
    }
    if (max_yres > VBE_DISPI_MAX_YRES) {
      state.vbe_max_yres=VBE_DISPI_MAX_YRES;
    } else {
      state.vbe_max_yres=max_yres;
    }
    if (max_bpp > VBE_DISPI_MAX_BPP) {
      state.vbe_max_bpp=VBE_DISPI_MAX_BPP;
    } else {
      state.vbe_max_bpp=max_bpp;
    }
    this->extension_init = 1;
  
    BX_INFO(("VBE Bochs Display Extension Enabled"));
  }
#endif  

    state.CRTC.reg[0x09] = 16;
    state.graphics_ctrl.memory_mapping = 3; // color text mode

    state.vga_mem_updated = 1;

#if defined(_WIN32)
    screen_refresh_handle_s3 = CreateThread(NULL,0,refresh_proc_s3,this,0,NULL);
#else
    pthread_create(&screen_refresh_handle_s3,NULL,refresh_proc_s3,this);
#endif

    printf("%%VGA-I-INIT: S3 Trio 64 Initialized\n");
}

CS3Trio64::~CS3Trio64()
{
  printf("%%VGA-I-SHUTDOWN: vga console has shut down.\n");
}

u32 CS3Trio64::ReadMem_Legacy(int index, u32 address, int dsize)
{
  switch(index)
    {
    case 2: /* io ports */
      return io_read(address, dsize);
    case 4: /* legacy memory */
      return legacy_read(address, dsize);
    case 5: /* rom */
    case 6:
      return rom_read(address, dsize);
    }
  return 0;
}

void CS3Trio64::WriteMem_Legacy(int index, u32 address, int dsize, u32 data)
{
  switch(index)
    {
    case 2:  /* io port */
      io_write(address,dsize,data);
      return;
    case 4:  /* legacy memory */
      legacy_write(address,dsize,data);
      return;
    case 5:  /* rom */
    case 6:
      rom_write(address,dsize,data);
      return;
    case 7: /* bios message */
      bios_message[bios_message_size++] = (char) data & 0xff;
      if (((data & 0xff) == 0x0a) || ((data &0xff) == 0x0d)) 
      {
        if (bios_message_size>1)
        {
          bios_message[bios_message_size-1] = '\0';
          printf("%%VGA-I-BIOS: %s\n",bios_message);
        }
        bios_message_size = 0;
      }
      return;
    }
}

u32 CS3Trio64::ReadMem_Bar(int func, int bar, u32 address, int dsize)
{
  switch(bar)
    {
    case 0: /* pci memory */
      return mem_read(address, dsize);
    }
  return 0;
}

void CS3Trio64::WriteMem_Bar(int func, int bar, u32 address, int dsize, u32 data)
{
  switch(bar)
    {
    case 0:  /* pci memory */
      mem_write(address,dsize,data);
      return;
    }
}

/**
 * Redraw the screen.
 **/
int CS3Trio64::DoClock()
{
  return 0;
}

static u32 s3_magic1 = 0x53338811;
static u32 s3_magic2 = 0x88115333;

/**
 * Save state to a Virtual Machine State file.
 **/

int CS3Trio64::SaveState(FILE *f)
{
  long ss = sizeof(state);
  int res;

  if (res = CPCIDevice::SaveState(f))
    return res;

  fwrite(&s3_magic1,sizeof(u32),1,f);
  fwrite(&ss,sizeof(long),1,f);
  fwrite(&state,sizeof(state),1,f);
  fwrite(&s3_magic2,sizeof(u32),1,f);
  printf("%s: %d bytes saved.\n",devid_string,ss);
  return 0;
}

/**
 * Restore state from a Virtual Machine State file.
 **/

int CS3Trio64::RestoreState(FILE *f)
{
  long ss;
  u32 m1;
  u32 m2;
  int res;
  size_t r;

  if (res = CPCIDevice::RestoreState(f))
    return res;

  r = fread(&m1,sizeof(u32),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (m1 != s3_magic1)
  {
    printf("%s: MAGIC 1 does not match!\n",devid_string);
    return -1;
  }

  fread(&ss,sizeof(long),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (ss != sizeof(state))
  {
    printf("%s: STRUCT SIZE does not match!\n",devid_string);
    return -1;
  }

  fread(&state,sizeof(state),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }

  r = fread(&m2,sizeof(u32),1,f);
  if (r!=1)
  {
    printf("%s: unexpected end of file!\n",devid_string);
    return -1;
  }
  if (m2 != s3_magic2)
  {
    printf("%s: MAGIC 1 does not match!\n",devid_string);
    return -1;
  }

  printf("%s: %d bytes restored.\n",devid_string,ss);
  return 0;
}

/**
 * Read from Framebuffer
 */
u32 CS3Trio64::mem_read(u32 address, int dsize)
{
  u32 data = 0;
  //printf("S3 mem read: %" LL "x, %d, %" LL "x   \n", address, dsize, data);

  return data;
}

/**
 * Write to Framebuffer
 */
void CS3Trio64::mem_write(u32 address, int dsize, u32 data)
{

  //printf("S3 mem write: %" LL "x, %d, %" LL "x   \n", address, dsize, data);
  switch(dsize) {
  case 8:
  case 16:
  case 32:
    break;
  }
}

/**
 * Read from Legacy Framebuffer
 */
u32 CS3Trio64::legacy_read(u32 address, int dsize)
{
  u32 data = 0;    
  switch (dsize)
    {
    case 32:
      data |= (u64)vga_mem_read(address + 0xA0002) << 16;// (u64)(*((u8*)x))&0xff;
      data |= (u64)vga_mem_read(address + 0xA0003) << 24;// (u64)(*((u8*)x))&0xff;
    case 16:
      data |= (u64)vga_mem_read(address + 0xA0001) << 8;// (u64)(*((u8*)x))&0xff;
    case 8:
      data |= (u64)vga_mem_read(address + 0xA0000);// (u64)(*((u8*)x))&0xff;
    }
//  //printf("S3 legacy read: %" LL "x, %d, %" LL "x   \n", address, dsize, data);
  return data;
}

/**
 * Write to Legacy Framebuffer
 */
void CS3Trio64::legacy_write(u32 address, int dsize, u32 data)
{
//  //printf("S3 legacy write: %" LL "x, %d, %" LL "x   \n", address, dsize, data);
  switch(dsize) {
  case 32:
    vga_mem_write(address+0xA0002, (u8)(data>>16));
    vga_mem_write(address+0xA0003, (u8)(data>>24));
  case 16:
    vga_mem_write(address+0xA0001, (u8)(data>>8));
  case 8:
    vga_mem_write(address+0xA0000, (u8)(data));
  }
}


/**
 * Read from Option ROM
 */
u32 CS3Trio64::rom_read(u32 address, int dsize)
{
  u32 data = 0x00;  // make it easy for the checksummer.
  u8 *x=(u8 *)option_rom;
  if(address<= rom_max) {
    x+=address;
    switch (dsize)
      {
      case 8:
	data = (u64)(*((u8*)x))&0xff;
	break;
      case 16:
	data = (u64)(*((u16*)x))&0xffff;
	break;
      case 32:
	data = (u64)(*((u32*)x))&0xffffffff;
	break;
      }
    //printf("S3 rom read: %" LL "x, %d, %" LL "x\n", address, dsize,data);
  } else {
    //printf("S3 (BAD) rom read: %" LL "x, %d, %" LL "x\n", address, dsize,data);
  }
  return data;
}

/**
 * Write to Option ROM
 */
void CS3Trio64::rom_write(u32 address, int dsize, u32 data)
{
  //printf("S3 rom write: %" LL "x, %d, %" LL "x --", address, dsize, data);
}

/**
 * Read from I/O Port
 */
u32 CS3Trio64::io_read(u32 address, int dsize)
{
  u32 data = 0;
  if (dsize !=8)
  {
    printf("Unsupported dsize!\n");
    exit(1);
  }

  switch(address+VGA_BASE) {
    case 0x3c0:
        data = read_b_3c0();
        break;
    case 0x3c1:
        data = read_b_3c1();
        break;
    case 0x3c3:
      data = read_b_3c3();
      break;
    case 0x3c4:
      data = read_b_3c4();
      break;
    case 0x3c5:
      data = read_b_3c5();
      break;
    case 0x3c9:
      data = read_b_3c9();
      break;
   case 0x3cc:
        data = read_b_3cc();
        break;
   case 0x3cf:
        data = read_b_3cf();
        break;
    case 0x3b4:
    case 0x3d4:
        data = read_b_3d4();
        break;
    case 0x3b5:
    case 0x3d5:
        data = read_b_3d5();
        break;
    case 0x3ba:
    case 0x3da:
        data = read_b_3da();
        break;

  //case 0x3d5:
  //  switch(state.crtc_index) {
  //    /* registers that are just mirrors of the data. */
  //  case 0x0a: // cursor start line
  //  case 0x0b: // cursor end line
  //  case 0x38: // CR38 Register Lock 1
  //  case 0x39: // CR39 Register Lock 2
  //  case 0x46: 
  //  case 0x54: // Extended Memory Control 2 Register
  //  case 0x56: // External Sync Control 1 Register 
  //  case 0x57:
  //    data = state.crtc_data[state.crtc_index];
  //    break;

  //    /* constant or special registers */
  //  case 0x2e: //Extended Chip ID
  //    data = 0x11; // trio 64.
  //    break;
  //  case 0x2f: // Revision 
  //    data = 0x00;
  //    break;
  //  case 0x36: // CR36 Reset State Read 1
  //    // 000 1 10 10
  //    // ^   ^ ^  ^- PCI
  //    // |   | +---- Memory page mode select. EDO
  //    // |   +------ Display memory dedicated
  //    // +---------- Memory size.  4M
  //    data = 0x1A;
  //    break;

  //    /* catchall */
  //  default:
  //    printf("%%VGA-I-CRTC: read unknown crtc index: %x\n",state.crtc_index);
  //  }
  //  break;
  //case 0x3da: // input status #1 register
  //  // this tells if there's a retrace in progress.  Nope!
  //  data = 0x00;
  //  // also resets the 3c0 mode.
  //  state.reg_3c0_mode = 0;
  //  break;
  default:
    printf("%%VGA-W-PORT: Unhandled port %x read\n",address+VGA_BASE);
    exit(1);
  }

  //printf("S3 io read: %" LL "x, %d, %" LL "x   \n", address+VGA_BASE, dsize, data);

  return data;
}

/**
 * Write to I/O Port
 */
void CS3Trio64::io_write(u32 address, int dsize, u32 data)
{
//  printf("S3 io write: %" LL "x, %d, %" LL "x   \n", address+VGA_BASE, dsize, data);
  switch(dsize)
  {
  case 8:
    io_write_b(address,(u8)data);
    break;
  case 16:
    io_write_b(address    ,(u8) data    &0xff);
    io_write_b(address + 1,(u8)(data>>8)&0xff);
    break;
  default:
    //printf("Weird IO size! \n");
    exit(1);
  }
}

void CS3Trio64::io_write_b(u32 address, u8 data)
{
  switch(address+VGA_BASE) {

    case 0x3c0:
        write_b_3c0(data);
        break;
    case 0x3c2:
        write_b_3c2(data);
        break;
    case 0x3c4:
        write_b_3c4(data);
        break;
    case 0x3c5:
        write_b_3c5(data);
        break;
    case 0x3c6:
        write_b_3c6(data);
        break;
    case 0x3c7:
        write_b_3c7(data);
        break;
    case 0x3c8:
        write_b_3c8(data);
        break;
    case 0x3c9:
        write_b_3c9(data);
        break;
    case 0x3ce:
        write_b_3ce(data);
        break;
    case 0x3cf:
        write_b_3cf(data);
        break;
    case 0x3b4:
    case 0x3d4:
      write_b_3d4(data);
      break;
    case 0x3b5:
    case 0x3d5:
      write_b_3d5(data);
      break;

  default:
    printf("%%VGA-W-PORT: Unhandled port %x write\n",address+VGA_BASE);
    exit(1);
  }
}

void CS3Trio64::write_b_3c0(u8 value)
{
  bool prev_video_enabled, prev_line_graphics, prev_int_pal_size;

   /* Attribute Controller */
       if (state.attribute_ctrl.flip_flop == 0) 
       { /* address mode */
         prev_video_enabled = state.attribute_ctrl.video_enabled;
         state.attribute_ctrl.video_enabled = (value >> 5) & 0x01;
#if defined(DEBUG_VGA)
         printf("io write 3c0: video_enabled = %u   \n", (unsigned) state.attribute_ctrl.video_enabled);
#endif
         if (state.attribute_ctrl.video_enabled == 0)
         {
           bx_gui->clear_screen();
         }
         else if (!prev_video_enabled) {
#if defined(DEBUG_VGA)
           printf("found enable transition   \n");
#endif
             redraw_area(0, 0, old_iWidth, old_iHeight);
         }
         value &= 0x1f; /* address = bits 0..4 */
         state.attribute_ctrl.address = value;
         switch (value) {
           case 0x00: case 0x01: case 0x02: case 0x03:
           case 0x04: case 0x05: case 0x06: case 0x07:
           case 0x08: case 0x09: case 0x0a: case 0x0b:
           case 0x0c: case 0x0d: case 0x0e: case 0x0f:
             break;
 
#if defined(DEBUG_VGA)
           default:
             printf("io write 3c0: address mode reg=%u   \n",(unsigned) value);
#endif
           }
         }
       else { /* data-write mode */
         switch (state.attribute_ctrl.address) {
           case 0x00: case 0x01: case 0x02: case 0x03:
           case 0x04: case 0x05: case 0x06: case 0x07:
           case 0x08: case 0x09: case 0x0a: case 0x0b:
           case 0x0c: case 0x0d: case 0x0e: case 0x0f:
             if (value != state.attribute_ctrl.palette_reg[state.attribute_ctrl.address]) {
               state.attribute_ctrl.palette_reg[state.attribute_ctrl.address] = value;
               redraw_area(0, 0, old_iWidth, old_iHeight);
             }
             break;
           case 0x10: // mode control register
             prev_line_graphics = state.attribute_ctrl.mode_ctrl.enable_line_graphics;
             prev_int_pal_size = state.attribute_ctrl.mode_ctrl.internal_palette_size;
             state.attribute_ctrl.mode_ctrl.graphics_alpha =
               (value >> 0) & 0x01;
             state.attribute_ctrl.mode_ctrl.display_type =
               (value >> 1) & 0x01;
             state.attribute_ctrl.mode_ctrl.enable_line_graphics =
               (value >> 2) & 0x01;
             state.attribute_ctrl.mode_ctrl.blink_intensity =
               (value >> 3) & 0x01;
             state.attribute_ctrl.mode_ctrl.pixel_panning_compat =
               (value >> 5) & 0x01;
             state.attribute_ctrl.mode_ctrl.pixel_clock_select =
               (value >> 6) & 0x01;
             state.attribute_ctrl.mode_ctrl.internal_palette_size =
               (value >> 7) & 0x01;
             if (((value >> 2) & 0x01) != prev_line_graphics) {
               bx_gui->set_text_charmap( & state.memory[0x20000 + state.charmap_address]);
               state.vga_mem_updated = 1;
             }
             if (((value >> 7) & 0x01) != prev_int_pal_size) {
             redraw_area(0, 0, old_iWidth, old_iHeight);
             }
#if defined(DEBUG_VGA)
             printf("io write 3c0: mode control: %02x h   \n", (unsigned) value);
#endif
             break;
           case 0x11: // Overscan Color Register
             state.attribute_ctrl.overscan_color = (value & 0x3f);
#if defined(DEBUG_VGA)
             printf("io write 3c0: overscan color = %02x   \n",
                         (unsigned) value);
#endif
             break;
           case 0x12: // Color Plane Enable Register
             state.attribute_ctrl.color_plane_enable = (value & 0x0f);
             redraw_area(0, 0, old_iWidth, old_iHeight);
#if defined(DEBUG_VGA)
             printf("io write 3c0: color plane enable = %02x   \n",
                         (unsigned) value);
#endif
             break;
           case 0x13: // Horizontal Pixel Panning Register
             state.attribute_ctrl.horiz_pel_panning = (value & 0x0f);
             redraw_area(0, 0, old_iWidth, old_iHeight);
#if defined(DEBUG_VGA)
             printf("io write 3c0: horiz pel panning = %02x   \n",
                         (unsigned) value);
#endif
             break;
           case 0x14: // Color Select Register
             state.attribute_ctrl.color_select = (value & 0x0f);
             redraw_area(0, 0, old_iWidth, old_iHeight);
#if defined(DEBUG_VGA)
             printf("io write 3c0: color select = %02x   \n",
                         (unsigned) state.attribute_ctrl.color_select);
#endif
             break;
           default:
             printf("io write 3c0: data-write mode %02x h  \n",(unsigned) state.attribute_ctrl.address);
             exit(1);
           }
         }
       state.attribute_ctrl.flip_flop = !state.attribute_ctrl.flip_flop;
}

void CS3Trio64::write_b_3c2(u8 value)
{
  /* Miscellaneous Output Register */
      state.misc_output.color_emulation  = (value >> 0) & 0x01;
      state.misc_output.enable_ram       = (value >> 1) & 0x01;
      state.misc_output.clock_select     = (value >> 2) & 0x03;
      state.misc_output.select_high_bank = (value >> 5) & 0x01;
      state.misc_output.horiz_sync_pol   = (value >> 6) & 0x01;
      state.misc_output.vert_sync_pol    = (value >> 7) & 0x01;
#if defined(DEBUG_VGA)
         printf("io write 3c2:   \n");
         printf("  color_emulation = %u   \n", (unsigned) state.misc_output.color_emulation);
         printf("  enable_ram = %u   \n",(unsigned) state.misc_output.enable_ram);
         printf("  clock_select = %u   \n", (unsigned) state.misc_output.clock_select);
         printf("  select_high_bank = %u   \n", (unsigned) state.misc_output.select_high_bank);
         printf("  horiz_sync_pol = %u   \n", (unsigned) state.misc_output.horiz_sync_pol);
         printf("  vert_sync_pol = %u   \n", (unsigned) state.misc_output.vert_sync_pol);
#endif
}

void CS3Trio64::write_b_3c4(u8 value)
{
     /* Sequencer Index Register */
       state.sequencer.index = value;
}

void CS3Trio64::write_b_3c5(u8 value)
{
  unsigned i;
  u8 charmap1, charmap2;

    /* Sequencer Registers 00..04 */
       switch (state.sequencer.index) {
         case 0: /* sequencer: reset */
 #if defined(DEBUG_VGA)
           printf("write 0x3c5: sequencer reset: value=0x%02x   \n", (unsigned) value);
 #endif
           if (state.sequencer.reset1 && ((value & 0x01) == 0)) 
           {
             state.sequencer.char_map_select = 0;
             state.charmap_address = 0;
             bx_gui->set_text_charmap(& state.memory[0x20000 + state.charmap_address]);
             state.vga_mem_updated = 1;
           }
          state.sequencer.reset1 = (value >> 0) & 0x01;
           state.sequencer.reset2 = (value >> 1) & 0x01;
           break;
         case 1: /* sequencer: clocking mode */
#if defined(DEBUG_VGA)
           printf("io write 3c5=%02x: clocking mode reg: ignoring   \n", (unsigned) value);
#endif
           state.sequencer.reg1 = value & 0x3f;
           state.x_dotclockdiv2 = ((value & 0x08) > 0);
           break;
         case 2: /* sequencer: map mask register */
           state.sequencer.map_mask = (value & 0x0f);
           for (i=0; i<4; i++)
             state.sequencer.map_mask_bit[i] = (value >> i) & 0x01;
           break;
         case 3: /* sequencer: character map select register */
           state.sequencer.char_map_select = value;
           charmap1 = value & 0x13;
           if (charmap1 > 3) charmap1 = (charmap1 & 3) + 4;
           charmap2 = (value & 0x2C) >> 2;
           if (charmap2 > 3) charmap2 = (charmap2 & 3) + 4;
           if (state.CRTC.reg[0x09] > 0) {
             state.charmap_address = (charmap1 << 13);
             bx_gui->set_text_charmap(& state.memory[0x20000 + state.charmap_address]);
             state.vga_mem_updated = 1;
             }
           if (charmap2 != charmap1)
             printf("char map select: #2=%d (unused)   \n", charmap2);
           break;
         case 4: /* sequencer: memory mode register */
           state.sequencer.extended_mem   = (value >> 1) & 0x01;
           state.sequencer.odd_even       = (value >> 2) & 0x01;
           state.sequencer.chain_four     = (value >> 3) & 0x01;
 
 #if defined(DEBUG_VGA)
           printf("io write 3c5: index 4:   \n");
           printf("  extended_mem %u   \n", (unsigned) state.sequencer.extended_mem);
           printf("  odd_even %u   \n", (unsigned) state.sequencer.odd_even);
           printf("  chain_four %u   \n", (unsigned) state.sequencer.chain_four);
 #endif
           break;
         default:
           printf("io write 3c5: index %u unhandled   \n", (unsigned) state.sequencer.index);
           exit(1);
         }
}

void CS3Trio64::write_b_3c6(u8 value)
{
/* PEL mask */
       state.pel.mask = value;
#if defined(DEBUG_VGA)
       if (state.pel.mask != 0xff)
         printf("io write 3c6: PEL mask=0x%02x != 0xFF   \n", value);
#endif
       // state.pel.mask should be and'd with final value before
       // indexing into color register state.pel.data[]
}

void CS3Trio64::write_b_3c7(u8 value)
{
// PEL address, read mode
       state.pel.read_data_register = value;
       state.pel.read_data_cycle    = 0;
       state.pel.dac_state = 0x03;
}

void CS3Trio64::write_b_3c8(u8 value)
{
/* PEL address write mode */
       state.pel.write_data_register = value;
       state.pel.write_data_cycle    = 0;
       state.pel.dac_state = 0x00;
}

void CS3Trio64::write_b_3c9(u8 value)
{
  /* PEL Data Register, colors 00..FF */
  switch (state.pel.write_data_cycle) {
         case 0:
           state.pel.data[state.pel.write_data_register].red = value;
           break;
         case 1:
           state.pel.data[state.pel.write_data_register].green = value;
           break;
         case 2:
           state.pel.data[state.pel.write_data_register].blue = value;
           if (bx_gui->palette_change(state.pel.write_data_register,
             state.pel.data[state.pel.write_data_register].red<<2,
             state.pel.data[state.pel.write_data_register].green<<2,
             state.pel.data[state.pel.write_data_register].blue<<2))
             redraw_area(0, 0, old_iWidth, old_iHeight);
           break;
         }
 
       state.pel.write_data_cycle++;
       if (state.pel.write_data_cycle >= 3) {
         //BX_INFO(("state.pel.data[%u] {r=%u, g=%u, b=%u}",
         //  (unsigned) state.pel.write_data_register,
         //  (unsigned) state.pel.data[state.pel.write_data_register].red,
         //  (unsigned) state.pel.data[state.pel.write_data_register].green,
         //  (unsigned) state.pel.data[state.pel.write_data_register].blue);
         state.pel.write_data_cycle = 0;
         state.pel.write_data_register++;
          }
}

void CS3Trio64::write_b_3ce(u8 value)
{
  /* Graphics Controller Index Register */
#if defined(DEBUG_VGA)
  if (value > 0x08) /* ??? */
    printf("io write: 3ce: value > 8   \n");
#endif
  state.graphics_ctrl.index = value;
}

void CS3Trio64::write_b_3cf(u8 value)
{
  u8 prev_memory_mapping;
  bool prev_graphics_alpha, prev_chain_odd_even;
/* Graphics Controller Registers 00..08 */
       switch (state.graphics_ctrl.index) {
         case 0: /* Set/Reset */
           state.graphics_ctrl.set_reset = value & 0x0f;
           break;
         case 1: /* Enable Set/Reset */
           state.graphics_ctrl.enable_set_reset = value & 0x0f;
           break;
         case 2: /* Color Compare */
           state.graphics_ctrl.color_compare = value & 0x0f;
           break;
         case 3: /* Data Rotate */
           state.graphics_ctrl.data_rotate = value & 0x07;
           /* ??? is this bits 3..4 or 4..5 */
           state.graphics_ctrl.raster_op    = (value >> 3) & 0x03; /* ??? */
           break;
         case 4: /* Read Map Select */
           state.graphics_ctrl.read_map_select = value & 0x03;
#if defined(DEBUG_VGA)
           printf("io write to 03cf = %02x (RMS)   \n", (unsigned) value);
#endif
           break;
         case 5: /* Mode */
           state.graphics_ctrl.write_mode        = value & 0x03;
           state.graphics_ctrl.read_mode         = (value >> 3) & 0x01;
           state.graphics_ctrl.odd_even   = (value >> 4) & 0x01;
           state.graphics_ctrl.shift_reg         = (value >> 5) & 0x03;
 
#if defined(DEBUG_VGA)
           if (state.graphics_ctrl.odd_even)
             printf("io write: 3cf: reg 05: value = %02xh   \n",
               (unsigned) value);
           if (state.graphics_ctrl.shift_reg)
             printf("io write: 3cf: reg 05: value = %02xh   \n",
               (unsigned) value);
#endif
           break;
         case 6: /* Miscellaneous */
           prev_graphics_alpha = state.graphics_ctrl.graphics_alpha;
           prev_chain_odd_even = state.graphics_ctrl.chain_odd_even;
           prev_memory_mapping = state.graphics_ctrl.memory_mapping;
 
           state.graphics_ctrl.graphics_alpha = value & 0x01;
           state.graphics_ctrl.chain_odd_even = (value >> 1) & 0x01;
           state.graphics_ctrl.memory_mapping = (value >> 2) & 0x03;
#if defined(DEBUG_VGA)
           printf("memory_mapping set to %u   \n",
               (unsigned) state.graphics_ctrl.memory_mapping);
           printf("graphics mode set to %u   \n",
               (unsigned) state.graphics_ctrl.graphics_alpha);
           printf("odd_even mode set to %u   \n",
               (unsigned) state.graphics_ctrl.odd_even);
           printf("io write: 3cf: reg 06: value = %02xh   \n",
                 (unsigned) value);
#endif
           if (prev_memory_mapping != state.graphics_ctrl.memory_mapping)
           {
             redraw_area(0, 0, old_iWidth, old_iHeight);
           }
           if (prev_graphics_alpha != state.graphics_ctrl.graphics_alpha) {
             redraw_area(0, 0, old_iWidth, old_iHeight);
             old_iHeight = 0;
           }
           break;
         case 7: /* Color Don't Care */
           state.graphics_ctrl.color_dont_care = value & 0x0f;
           break;
         case 8: /* Bit Mask */
           state.graphics_ctrl.bitmask = value;
           break;
         default:
           /* ??? */
           printf("io write: 3cf: index %u unhandled   \n", (unsigned) state.graphics_ctrl.index);
           exit(1);
         }
}

void CS3Trio64::write_b_3d4(u8 value)
{
  state.CRTC.address = value & 0x7f;
#if defined(DEBUG_VGA)
       if (state.CRTC.address > 0x18)
         printf("write: invalid CRTC register 0x%02x selected",
           (unsigned) state.CRTC.address);
#endif
}

void CS3Trio64::write_b_3d5(u8 value)
{
  /* CRTC Registers */
  if (state.CRTC.address > 0x18) {
#if defined(DEBUG_VGA)
         printf("write: invalid CRTC register 0x%02x ignored",
           (unsigned) state.CRTC.address);
#endif
         return;
       }
       if (state.CRTC.write_protect && (state.CRTC.address < 0x08)) {
        if (state.CRTC.address == 0x07) {
          state.CRTC.reg[state.CRTC.address] &= ~0x10;
          state.CRTC.reg[state.CRTC.address] |= (value & 0x10);
          state.line_compare &= 0x2ff;
          if (state.CRTC.reg[0x07] & 0x10) state.line_compare |= 0x100;
          redraw_area(0, 0, old_iWidth, old_iHeight);
          return;
        } else {
          return;
        }
      }

       if (value != state.CRTC.reg[state.CRTC.address]) {
         state.CRTC.reg[state.CRTC.address] = value;
         switch (state.CRTC.address) {
           case 0x07:
             state.vertical_display_end &= 0xff;
             if (state.CRTC.reg[0x07] & 0x02) state.vertical_display_end |= 0x100;
             if (state.CRTC.reg[0x07] & 0x40) state.vertical_display_end |= 0x200;
             state.line_compare &= 0x2ff;
             if (state.CRTC.reg[0x07] & 0x10) state.line_compare |= 0x100;
             redraw_area(0, 0, old_iWidth, old_iHeight);
             break;
           case 0x08:
             // Vertical pel panning change
             redraw_area(0, 0, old_iWidth, old_iHeight);
             break;
           case 0x09:
             state.y_doublescan = ((value & 0x9f) > 0);
             state.line_compare &= 0x1ff;
             if (state.CRTC.reg[0x09] & 0x40) state.line_compare |= 0x200;
               redraw_area(0, 0, old_iWidth, old_iHeight);
             break;
           case 0x0A:
           case 0x0B:
           case 0x0E:
           case 0x0F:
             // Cursor size / location change
             state.vga_mem_updated = 1;
             break;
           case 0x0C:
           case 0x0D:
             // Start address change
             if (state.graphics_ctrl.graphics_alpha) {
             redraw_area(0, 0, old_iWidth, old_iHeight);
             } else {
               state.vga_mem_updated = 1;
             }
             break;
           case 0x12:
             state.vertical_display_end &= 0x300;
             state.vertical_display_end |= state.CRTC.reg[0x12];
             break;
           case 0x13:
           case 0x14:
           case 0x17:
             // Line offset change
             state.line_offset = state.CRTC.reg[0x13] << 1;
             if (state.CRTC.reg[0x14] & 0x40) state.line_offset <<= 2;
             else if ((state.CRTC.reg[0x17] & 0x40) == 0) state.line_offset <<= 1;
             redraw_area(0, 0, old_iWidth, old_iHeight);
             break;
           case 0x18:
             state.line_compare &= 0x300;
             state.line_compare |= state.CRTC.reg[0x18];
             redraw_area(0, 0, old_iWidth, old_iHeight);
             break;
         }
       }
}

u8 CS3Trio64::read_b_3c0()
{
  if (state.attribute_ctrl.flip_flop == 0) {
    //BX_INFO(("io read: 0x3c0: flip_flop = 0"));
    return (state.attribute_ctrl.video_enabled << 5) |
            state.attribute_ctrl.address;
  } else {
    printf("io read: 0x3c0: flip_flop != 0   \n");
    exit(1);
  }
}

u8 CS3Trio64::read_b_3c1()
{
  u8 retval;
       switch (state.attribute_ctrl.address) {
         case 0x00: case 0x01: case 0x02: case 0x03:
         case 0x04: case 0x05: case 0x06: case 0x07:
         case 0x08: case 0x09: case 0x0a: case 0x0b:
         case 0x0c: case 0x0d: case 0x0e: case 0x0f:
           retval = state.attribute_ctrl.palette_reg[state.attribute_ctrl.address];
           return (retval);
           break;
         case 0x10: /* mode control register */
           retval =
             (state.attribute_ctrl.mode_ctrl.graphics_alpha << 0) |
             (state.attribute_ctrl.mode_ctrl.display_type << 1) |
             (state.attribute_ctrl.mode_ctrl.enable_line_graphics << 2) |
             (state.attribute_ctrl.mode_ctrl.blink_intensity << 3) |
             (state.attribute_ctrl.mode_ctrl.pixel_panning_compat << 5) |
             (state.attribute_ctrl.mode_ctrl.pixel_clock_select << 6) |
             (state.attribute_ctrl.mode_ctrl.internal_palette_size << 7);
           return (retval);
           break;
         case 0x11: /* overscan color register */
           return (state.attribute_ctrl.overscan_color);
           break;
         case 0x12: /* color plane enable */
           return (state.attribute_ctrl.color_plane_enable);
           break;
         case 0x13: /* horizontal PEL panning register */
           return (state.attribute_ctrl.horiz_pel_panning);
           break;
         case 0x14: /* color select register */
           return (state.attribute_ctrl.color_select);
           break;
         default:
           printf("io read: 0x3c1: unknown register 0x%02x   \n",
             (unsigned) state.attribute_ctrl.address);
           exit(1);
         }
}

u8 CS3Trio64::read_b_3c3()
{
      return state.vga_enabled;

}

u8 CS3Trio64::read_b_3c4()
{
  return state.sequencer.index;
}

u8 CS3Trio64::read_b_3c5()
{
      switch (state.sequencer.index) {
        case 0: /* sequencer: reset */
#if defined(DEBUG_VGA)
          BX_DEBUG(("io read 0x3c5: sequencer reset"));
#endif
          return (state.sequencer.reset1?1:0) | (state.sequencer.reset2?2:0);
          break;
        case 1: /* sequencer: clocking mode */
 #if defined(DEBUG_VGA)
         BX_DEBUG(("io read 0x3c5: sequencer clocking mode"));
#endif
          return state.sequencer.reg1;
          break;
        case 2: /* sequencer: map mask register */
          return state.sequencer.map_mask;
          break;
        case 3: /* sequencer: character map select register */
          return state.sequencer.char_map_select;
          break;
        case 4: /* sequencer: memory mode register */
          return
            (state.sequencer.extended_mem   << 1) |
            (state.sequencer.odd_even       << 2) |
            (state.sequencer.chain_four     << 3);
          break;
           default:
          BX_DEBUG(("io read 0x3c5: index %u unhandled",
            (unsigned) state.sequencer.index));
          exit(1);
          return 0;
        }
}

u8 CS3Trio64::read_b_3c9()
{
  u8 retval;
      if (state.pel.dac_state == 0x03) {
        switch (state.pel.read_data_cycle) {
          case 0:
            retval = state.pel.data[state.pel.read_data_register].red;
            break;
          case 1:
            retval = state.pel.data[state.pel.read_data_register].green;
            break;
          case 2:
            retval = state.pel.data[state.pel.read_data_register].blue;
            break;
          default:
            retval = 0; // keep compiler happy
        }
        state.pel.read_data_cycle++;
        if (state.pel.read_data_cycle >= 3) {
          state.pel.read_data_cycle = 0;
          state.pel.read_data_register++;
        }
      }
      else {
        retval = 0x3f;
      }
      return retval;

}

u8 CS3Trio64::read_b_3cc()
{
    /* Miscellaneous Output / Graphics 1 Position ??? */
       return
         ((state.misc_output.color_emulation  & 0x01) << 0) |
         ((state.misc_output.enable_ram       & 0x01) << 1) |
         ((state.misc_output.clock_select     & 0x03) << 2) |
         ((state.misc_output.select_high_bank & 0x01) << 5) |
         ((state.misc_output.horiz_sync_pol   & 0x01) << 6) |
         ((state.misc_output.vert_sync_pol    & 0x01) << 7);
}

u8 CS3Trio64::read_b_3cf()
{
  u8 retval;
      switch (state.graphics_ctrl.index) {
        case 0: /* Set/Reset */
          return (state.graphics_ctrl.set_reset);
          break;
        case 1: /* Enable Set/Reset */
          return (state.graphics_ctrl.enable_set_reset);
          break;
        case 2: /* Color Compare */
          return (state.graphics_ctrl.color_compare);
          break;
        case 3: /* Data Rotate */
          retval =
            ((state.graphics_ctrl.raster_op & 0x03) << 3) |
            ((state.graphics_ctrl.data_rotate & 0x07) << 0);
          return (retval);
          break;
        case 4: /* Read Map Select */
          return (state.graphics_ctrl.read_map_select);
          break;
        case 5: /* Mode */
          retval =
            ((state.graphics_ctrl.shift_reg & 0x03) << 5) |
            ((state.graphics_ctrl.odd_even & 0x01 ) << 4) |
            ((state.graphics_ctrl.read_mode & 0x01) << 3) |
            ((state.graphics_ctrl.write_mode & 0x03) << 0);

#if defined(DEBUG_VGA)
          if (state.graphics_ctrl.odd_even ||
              state.graphics_ctrl.shift_reg)
            BX_DEBUG(("io read 0x3cf: reg 05 = 0x%02x", (unsigned) retval));
#endif
          return (retval);
          break;
        case 6: /* Miscellaneous */
          return
            ((state.graphics_ctrl.memory_mapping & 0x03 ) << 2) |
            ((state.graphics_ctrl.odd_even & 0x01) << 1) |
            ((state.graphics_ctrl.graphics_alpha & 0x01) << 0);
          break;
        case 7: /* Color Don't Care */
          return (state.graphics_ctrl.color_dont_care);
          break;
        case 8: /* Bit Mask */
          return (state.graphics_ctrl.bitmask);
          break;
        default:
          /* ??? */
          BX_DEBUG(("io read: 0x3cf: index %u unhandled",
            (unsigned) state.graphics_ctrl.index));
          exit(1);
          return (0);
      }

}

u8 CS3Trio64::read_b_3d4()
{
    return state.CRTC.address;
}

u8 CS3Trio64::read_b_3d5()
{
    if (state.CRTC.address > 0x18) {
        printf("io read: invalid CRTC register 0x%02x   \n",
          (unsigned) state.CRTC.address);
        exit(1);
        return 0;
      }
      return state.CRTC.reg[state.CRTC.address];
  
}

u8 CS3Trio64::read_b_3da()
{
/* Input Status 1 (color emulation modes) */

       u8 retval = 0;
  
       // bit3: Vertical Retrace
       //       0 = display is in the display mode
       //       1 = display is in the vertical retrace mode
       // bit0: Display Enable
       //       0 = display is in the display mode
       //       1 = display is not in the display mode; either the
       //           horizontal or vertical retrace period is active
 
       // using 72 Hz vertical frequency

       /*** TO DO ??? ***
       usec = bx_pc_system.time_usec();
       switch ( ( state.misc_output.vert_sync_pol << 1) | state.misc_output.horiz_sync_pol )
       {
         case 0: vertres = 200; break;
         case 1: vertres = 400; break;
         case 2: vertres = 350; break;
         default: vertres = 480; break;
       }
       if ((usec % 13888) < 70) {
         vert_retrace = 1;
       }
       if ((usec % (13888 / vertres)) == 0) {
         horiz_retrace = 1;
       }
 
       if (horiz_retrace || vert_retrace)
         retval = 0x01;
       if (vert_retrace)
         retval |= 0x08;
 
       *** TO DO ??? ***/

       /* reading this port resets the flip-flop to address mode */
       state.attribute_ctrl.flip_flop = 0;
       return retval;
}

u8 CS3Trio64::get_actl_palette_idx(u8 index)
{
  return state.attribute_ctrl.palette_reg[index];
}

void CS3Trio64::redraw_area(unsigned x0, unsigned y0, unsigned width, unsigned height)
{
  unsigned xti, yti, xt0, xt1, yt0, yt1, xmax, ymax;

  if ((width == 0) || (height == 0)) {
    return;
  }

  state.vga_mem_updated = 1;

#if BX_SUPPORT_VBE
  if (state.graphics_ctrl.graphics_alpha || state.vbe_enabled) {
#else
  if (state.graphics_ctrl.graphics_alpha) {
#endif
    // graphics mode
    xmax = old_iWidth;
    ymax = old_iHeight;
#if BX_SUPPORT_VBE
    if (state.vbe_enabled) {
      xmax = state.vbe_xres;
      ymax = state.vbe_yres;
    }
#endif
    xt0 = x0 / X_TILESIZE;
    yt0 = y0 / Y_TILESIZE;
    if (x0 < xmax) {
      xt1 = (x0 + width  - 1) / X_TILESIZE;
    } else {
      xt1 = (xmax - 1) / X_TILESIZE;
    }
    if (y0 < ymax) {
      yt1 = (y0 + height - 1) / Y_TILESIZE;
    } else {
      yt1 = (ymax - 1) / Y_TILESIZE;
    }
    for (yti=yt0; yti<=yt1; yti++) {
      for (xti=xt0; xti<=xt1; xti++) {
        SET_TILE_UPDATED (xti, yti, 1);
      }
    }

  } else {
    // text mode
    memset(state.text_snapshot, 0,
           sizeof(state.text_snapshot));
  }
}

void CS3Trio64::update(void)
{
  unsigned iHeight, iWidth;

  /* no screen update necessary */
  if (state.vga_mem_updated==0)
    return;

  /* skip screen update when vga/video is disabled or the sequencer is in reset mode */

  if (!state.vga_enabled || !state.attribute_ctrl.video_enabled
      || !state.sequencer.reset2 || !state.sequencer.reset1)
    return;

  /* skip screen update if the vertical retrace is in progress
     (using 72 Hz vertical frequency) */
//  if ((bx_pc_system.time_usec() % 13888) < 70)
//    return;

#if BX_SUPPORT_VBE  
  if ((state.vbe_enabled) && (state.vbe_bpp != VBE_DISPI_BPP_4))
  {
    // specific VBE code display update code
    unsigned pitch;
    unsigned xc, yc, xti, yti;
    unsigned r, c, w, h;
    int i;
    unsigned long red, green, blue, colour;
    u8 * vid_ptr, * vid_ptr2;
    u8 * tile_ptr, * tile_ptr2;
    bx_svga_tileinfo_t info;
    u8 dac_size = state.vbe_8bit_dac ? 8 : 6;

    iWidth=state.vbe_xres;
    iHeight=state.vbe_yres;
    pitch = state.line_offset;
    u8 *disp_ptr = &state.memory[state.vbe_virtual_start]; 

    if (bx_gui->graphics_tile_info(&info)) {
      if (info.is_indexed) {
        switch (state.vbe_bpp) {
          case 4:
          case 15:
          case 16:
          case 24:
          case 32:
            BX_ERROR(("current guest pixel format is unsupported on indexed colour host displays"));
            break;
          case 8:
            for (yc=0, yti = 0; yc<iHeight; yc+=Y_TILESIZE, yti++) {
              for (xc=0, xti = 0; xc<iWidth; xc+=X_TILESIZE, xti++) {
                if (GET_TILE_UPDATED (xti, yti)) {
                  vid_ptr = disp_ptr + (yc * pitch + xc);
                  tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                  for (r=0; r<h; r++) {
                    vid_ptr2  = vid_ptr;
                    tile_ptr2 = tile_ptr;
                    for (c=0; c<w; c++) {
                      colour = 0;
                      for (i=0; i<(int)state.vbe_bpp; i+=8) {
                        colour |= *(vid_ptr2++) << i;
                      }
                      if (info.is_little_endian) {
                        for (i=0; i<info.bpp; i+=8) {
                          *(tile_ptr2++) = (u8)(colour >> i);
                        }
                      }
                      else {
                        for (i=info.bpp-8; i>-8; i-=8) {
                          *(tile_ptr2++) = (u8)(colour >> i);
                        }
                      }
                    }
                    vid_ptr  += pitch;
                    tile_ptr += info.pitch;
                  }
                  bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                  SET_TILE_UPDATED (xti, yti, 0);
                }
              }
            }
            break;
        }
      }
      else {
        switch (state.vbe_bpp) {
          case 4:
            BX_ERROR(("cannot draw 4bpp SVGA"));
            break;
          case 8:
            for (yc=0, yti = 0; yc<iHeight; yc+=Y_TILESIZE, yti++) {
              for (xc=0, xti = 0; xc<iWidth; xc+=X_TILESIZE, xti++) {
                if (GET_TILE_UPDATED (xti, yti)) {
                  vid_ptr = disp_ptr + (yc * pitch + xc);
                  tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                  for (r=0; r<h; r++) {
                    vid_ptr2  = vid_ptr;
                    tile_ptr2 = tile_ptr;
                    for (c=0; c<w; c++) {
                      colour = *(vid_ptr2++);
                      colour = MAKE_COLOUR(
                        state.pel.data[colour].red, dac_size, info.red_shift, info.red_mask,
                        state.pel.data[colour].green, dac_size, info.green_shift, info.green_mask,
                        state.pel.data[colour].blue, dac_size, info.blue_shift, info.blue_mask);
                      if (info.is_little_endian) {
                        for (i=0; i<info.bpp; i+=8) {
                          *(tile_ptr2++) = (u8)(colour >> i);
                        }
                      }
                      else {
                        for (i=info.bpp-8; i>-8; i-=8) {
                          *(tile_ptr2++) = (u8)(colour >> i);
                        }
                      }
                    }
                    vid_ptr  += pitch;
                    tile_ptr += info.pitch;
                  }
                  bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                  SET_TILE_UPDATED (xti, yti, 0);
                }
              }
            }
            break;
          case 15:
            for (yc=0, yti = 0; yc<iHeight; yc+=Y_TILESIZE, yti++) {
              for (xc=0, xti = 0; xc<iWidth; xc+=X_TILESIZE, xti++) {
                if (GET_TILE_UPDATED (xti, yti)) {
                  vid_ptr = disp_ptr + (yc * pitch + (xc<<1));
                  tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                  for (r=0; r<h; r++) {
                    vid_ptr2  = vid_ptr;
                    tile_ptr2 = tile_ptr;
                    for (c=0; c<w; c++) {
                      colour = *(vid_ptr2++);
                      colour |= *(vid_ptr2++) << 8;
                      colour = MAKE_COLOUR(
                        colour & 0x001f, 5, info.blue_shift, info.blue_mask,
                        colour & 0x03e0, 10, info.green_shift, info.green_mask,
                        colour & 0x7c00, 15, info.red_shift, info.red_mask);
                      if (info.is_little_endian) {
                        for (i=0; i<info.bpp; i+=8) {
                          *(tile_ptr2++) = (u8)(colour >> i);
                        }
                      }
                      else {
                        for (i=info.bpp-8; i>-8; i-=8) {
                          *(tile_ptr2++) = (u8)(colour >> i);
                        }
                      }
                    }
                    vid_ptr  += pitch;
                    tile_ptr += info.pitch; 
                  }
                  bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                  SET_TILE_UPDATED (xti, yti, 0);
                }
              }
            }
            break;
          case 16:
            for (yc=0, yti = 0; yc<iHeight; yc+=Y_TILESIZE, yti++) {
              for (xc=0, xti = 0; xc<iWidth; xc+=X_TILESIZE, xti++) {
                if (GET_TILE_UPDATED (xti, yti)) {
                  vid_ptr = disp_ptr + (yc * pitch + (xc<<1));
                  tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                  for (r=0; r<h; r++) {
                    vid_ptr2  = vid_ptr;
                    tile_ptr2 = tile_ptr;
                    for (c=0; c<w; c++) {
                      colour = *(vid_ptr2++);
                      colour |= *(vid_ptr2++) << 8;
                      colour = MAKE_COLOUR(
                        colour & 0x001f, 5, info.blue_shift, info.blue_mask,
                        colour & 0x07e0, 11, info.green_shift, info.green_mask,
                        colour & 0xf800, 16, info.red_shift, info.red_mask);
                      if (info.is_little_endian) {
                        for (i=0; i<info.bpp; i+=8) {
                          *(tile_ptr2++) = (u8)(colour >> i);
                        }
                      }
                      else {
                        for (i=info.bpp-8; i>-8; i-=8) {
                          *(tile_ptr2++) = (u8)(colour >> i);
                        }
                      }
                    }
                    vid_ptr  += pitch;
                    tile_ptr += info.pitch; 
                  }
                  bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                  SET_TILE_UPDATED (xti, yti, 0);
                }
              }
            }
            break;
          case 24:
            for (yc=0, yti = 0; yc<iHeight; yc+=Y_TILESIZE, yti++) {
              for (xc=0, xti = 0; xc<iWidth; xc+=X_TILESIZE, xti++) {
                if (GET_TILE_UPDATED (xti, yti)) {
                  vid_ptr = disp_ptr + (yc * pitch + 3*xc);
                  tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                  for (r=0; r<h; r++) {
                    vid_ptr2  = vid_ptr;
                    tile_ptr2 = tile_ptr;
                    for (c=0; c<w; c++) {
                      blue = *(vid_ptr2++);
                      green = *(vid_ptr2++);
                      red = *(vid_ptr2++);
                      colour = MAKE_COLOUR(
                        red, 8, info.red_shift, info.red_mask,
                        green, 8, info.green_shift, info.green_mask,
                        blue, 8, info.blue_shift, info.blue_mask);
                      if (info.is_little_endian) {
                        for (i=0; i<info.bpp; i+=8) {
                          *(tile_ptr2++) = (u8)(colour >> i);
                        }
                      }
                      else {
                        for (i=info.bpp-8; i>-8; i-=8) {
                          *(tile_ptr2++) = (u8)(colour >> i);
                        }
                      }
                    }
                    vid_ptr  += pitch;
                    tile_ptr += info.pitch; 
                  }
                  bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                  SET_TILE_UPDATED (xti, yti, 0);
                }
              }
            }
            break;
          case 32:
            for (yc=0, yti = 0; yc<iHeight; yc+=Y_TILESIZE, yti++) {
              for (xc=0, xti = 0; xc<iWidth; xc+=X_TILESIZE, xti++) {
                if (GET_TILE_UPDATED (xti, yti)) {
                  vid_ptr = disp_ptr + (yc * pitch + (xc<<2));
                  tile_ptr = bx_gui->graphics_tile_get(xc, yc, &w, &h);
                  for (r=0; r<h; r++) {
                    vid_ptr2  = vid_ptr;
                    tile_ptr2 = tile_ptr;
                    for (c=0; c<w; c++) {
                      blue = *(vid_ptr2++);
                      green = *(vid_ptr2++);
                      red = *(vid_ptr2++);
                      vid_ptr2++;
                      colour = MAKE_COLOUR(
                        red, 8, info.red_shift, info.red_mask,
                        green, 8, info.green_shift, info.green_mask,
                        blue, 8, info.blue_shift, info.blue_mask);
                      if (info.is_little_endian) {
                        for (i=0; i<info.bpp; i+=8) {
                          *(tile_ptr2++) = (u8)(colour >> i);
                        }
                      }
                      else {
                        for (i=info.bpp-8; i>-8; i-=8) {
                          *(tile_ptr2++) = (u8)(colour >> i);
                        }
                      }
                    }
                    vid_ptr  += pitch;
                    tile_ptr += info.pitch;
                  }
                  bx_gui->graphics_tile_update_in_place(xc, yc, w, h);
                  SET_TILE_UPDATED (xti, yti, 0);
                }
              }
            }
            break;
        }
      }
      old_iWidth = iWidth;
      old_iHeight = iHeight;
      state.vga_mem_updated = 0;
    }
    else {
      BX_PANIC(("cannot get svga tile info"));
    }

    // after a vbe display update, don't try to do any 'normal vga' updates anymore
    return;
  }
#endif  
  // fields that effect the way video memory is serialized into screen output:
  // GRAPHICS CONTROLLER:
  //   state.graphics_ctrl.shift_reg:
  //     0: output data in standard VGA format or CGA-compatible 640x200 2 color
  //        graphics mode (mode 6)
  //     1: output data in CGA-compatible 320x200 4 color graphics mode
  //        (modes 4 & 5)
  //     2: output data 8 bits at a time from the 4 bit planes
  //        (mode 13 and variants like modeX)

  // if (state.vga_mem_updated==0 || state.attribute_ctrl.video_enabled == 0)

  if (state.graphics_ctrl.graphics_alpha) {
    u8 color;
    unsigned bit_no, r, c, x, y;
    unsigned long byte_offset, start_addr;
    unsigned xc, yc, xti, yti;

    start_addr = (state.CRTC.reg[0x0c] << 8) | state.CRTC.reg[0x0d];

//BX_DEBUG(("update: shiftreg=%u, chain4=%u, mapping=%u",
//  (unsigned) state.graphics_ctrl.shift_reg,
//  (unsigned) state.sequencer.chain_four,
//  (unsigned) state.graphics_ctrl.memory_mapping);

    determine_screen_dimensions(&iHeight, &iWidth);
    if((iWidth != old_iWidth) || (iHeight != old_iHeight) ||
        (state.last_bpp > 8))
    {
      bx_gui->dimension_update(iWidth, iHeight);
      old_iWidth = iWidth;
      old_iHeight = iHeight;
      state.last_bpp = 8;
    }

    switch ( state.graphics_ctrl.shift_reg ) {

      case 0:
        u8 attribute, palette_reg_val, DAC_regno;
        unsigned long line_compare;
        u8 *plane0, *plane1, *plane2, *plane3;

        if (state.graphics_ctrl.memory_mapping == 3) { // CGA 640x200x2

          for (yc=0, yti=0; yc<iHeight; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti=0; xc<iWidth; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                for (r=0; r<Y_TILESIZE; r++) {
                  y = yc + r;
                  if (state.y_doublescan) y >>= 1;
                  for (c=0; c<X_TILESIZE; c++) {

                    x = xc + c;
                    /* 0 or 0x2000 */
                    byte_offset = start_addr + ((y & 1) << 13);
                    /* to the start of the line */
                    byte_offset += (320 / 4) * (y / 2);
                    /* to the byte start */
                    byte_offset += (x / 8);

                    bit_no = 7 - (x % 8);
                    palette_reg_val = (((state.memory[byte_offset]) >> bit_no) & 1);
                    DAC_regno = state.attribute_ctrl.palette_reg[palette_reg_val];
                    state.tile[r*X_TILESIZE + c] = DAC_regno;
                  }
                }
                SET_TILE_UPDATED (xti, yti, 0);
                bx_gui->graphics_tile_update(state.tile, xc, yc);
              }
            }
          }
        } else { // output data in serial fashion with each display plane
                 // output on its associated serial output.  Standard EGA/VGA format

#if BX_SUPPORT_VBE  
          if (state.vbe_enabled)
          {
            plane0 = &state.memory[0<<VBE_DISPI_4BPP_PLANE_SHIFT];
            plane1 = &state.memory[1<<VBE_DISPI_4BPP_PLANE_SHIFT];
            plane2 = &state.memory[2<<VBE_DISPI_4BPP_PLANE_SHIFT];
            plane3 = &state.memory[3<<VBE_DISPI_4BPP_PLANE_SHIFT];
            start_addr = state.vbe_virtual_start;
            line_compare = 0xffff;
          }
          else
#endif
          {
            plane0 = &state.memory[0<<16];
            plane1 = &state.memory[1<<16];
            plane2 = &state.memory[2<<16];
            plane3 = &state.memory[3<<16];
            line_compare = state.line_compare;
            if (state.y_doublescan) line_compare >>= 1;
          }

          for (yc=0, yti=0; yc<iHeight; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti=0; xc<iWidth; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                for (r=0; r<Y_TILESIZE; r++) {
                  y = yc + r;
                  if (state.y_doublescan) y >>= 1;
                  for (c=0; c<X_TILESIZE; c++) {
                    x = xc + c;
                    if (state.x_dotclockdiv2) x >>= 1;
                    bit_no = 7 - (x % 8);
                    if (y > line_compare) {
                      byte_offset = x / 8 +
                        ((y - line_compare - 1) * state.line_offset);
                    } else {
                      byte_offset = start_addr + x / 8 +
                        (y * state.line_offset);
                    }
                    attribute =
                      (((plane0[byte_offset] >> bit_no) & 0x01) << 0) |
                      (((plane1[byte_offset] >> bit_no) & 0x01) << 1) |
                      (((plane2[byte_offset] >> bit_no) & 0x01) << 2) |
                      (((plane3[byte_offset] >> bit_no) & 0x01) << 3);

                    attribute &= state.attribute_ctrl.color_plane_enable;
                    // undocumented feature ???: colors 0..7 high intensity, colors 8..15 blinking
                    // using low/high intensity. Blinking is not implemented yet.
                    if (state.attribute_ctrl.mode_ctrl.blink_intensity) attribute ^= 0x08;
                    palette_reg_val = state.attribute_ctrl.palette_reg[attribute];
                    if (state.attribute_ctrl.mode_ctrl.internal_palette_size) {
                      // use 4 lower bits from palette register
                      // use 4 higher bits from color select register
                      // 16 banks of 16-color registers
                      DAC_regno = (palette_reg_val & 0x0f) |
                                  (state.attribute_ctrl.color_select << 4);
                      }
                    else {
                      // use 6 lower bits from palette register
                      // use 2 higher bits from color select register
                      // 4 banks of 64-color registers
                      DAC_regno = (palette_reg_val & 0x3f) |
                                  ((state.attribute_ctrl.color_select & 0x0c) << 4);
                      }
                    // DAC_regno &= video DAC mask register ???

                    state.tile[r*X_TILESIZE + c] = DAC_regno;
                    }
                  }
                SET_TILE_UPDATED (xti, yti, 0);
                bx_gui->graphics_tile_update(state.tile, xc, yc);
                }
              }
            }
          }
        break; // case 0

      case 1: // output the data in a CGA-compatible 320x200 4 color graphics
              // mode.  (modes 4 & 5)

        /* CGA 320x200x4 start */

        for (yc=0, yti=0; yc<iHeight; yc+=Y_TILESIZE, yti++) {
          for (xc=0, xti=0; xc<iWidth; xc+=X_TILESIZE, xti++) {
            if (GET_TILE_UPDATED (xti, yti)) {
              for (r=0; r<Y_TILESIZE; r++) {
                y = yc + r;
                if (state.y_doublescan) y >>= 1;
                for (c=0; c<X_TILESIZE; c++) {

                  x = xc + c;
                  if (state.x_dotclockdiv2) x >>= 1;
                  /* 0 or 0x2000 */
                  byte_offset = start_addr + ((y & 1) << 13);
                  /* to the start of the line */
                  byte_offset += (320 / 4) * (y / 2);
                  /* to the byte start */
                  byte_offset += (x / 4);

                  attribute = 6 - 2*(x % 4);
                  palette_reg_val = (state.memory[byte_offset]) >> attribute;
                  palette_reg_val &= 3;
                  DAC_regno = state.attribute_ctrl.palette_reg[palette_reg_val];
                  state.tile[r*X_TILESIZE + c] = DAC_regno;
                }
              }
              SET_TILE_UPDATED (xti, yti, 0);
              bx_gui->graphics_tile_update(state.tile, xc, yc);
            }
          }
        }
        /* CGA 320x200x4 end */

        break; // case 1

      case 2: // output the data eight bits at a time from the 4 bit plane
              // (format for VGA mode 13 hex)
      case 3: // FIXME: is this really the same ???

        if ( state.sequencer.chain_four ) {
          unsigned long pixely, pixelx, plane;

          if (state.misc_output.select_high_bank != 1)
          {
            printf("update: select_high_bank != 1   \n");
            exit(1);
          }

          for (yc=0, yti=0; yc<iHeight; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti=0; xc<iWidth; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                for (r=0; r<Y_TILESIZE; r++) {
                  pixely = yc + r;
                  if (state.y_doublescan) pixely >>= 1;
                  for (c=0; c<X_TILESIZE; c++) {
                    pixelx = (xc + c) >> 1;
                    plane  = (pixelx % 4);
                    byte_offset = start_addr + (plane * 65536) +
                                  (pixely * state.line_offset) + (pixelx & ~0x03);
                    color = state.memory[byte_offset];
                    state.tile[r*X_TILESIZE + c] = color;
                    }
                  }
                SET_TILE_UPDATED (xti, yti, 0);
                bx_gui->graphics_tile_update(state.tile, xc, yc);
                }
              }
            }
          }

        else { // chain_four == 0, modeX
          unsigned long pixely, pixelx, plane;

          for (yc=0, yti=0; yc<iHeight; yc+=Y_TILESIZE, yti++) {
            for (xc=0, xti=0; xc<iWidth; xc+=X_TILESIZE, xti++) {
              if (GET_TILE_UPDATED (xti, yti)) {
                for (r=0; r<Y_TILESIZE; r++) {
                  pixely = yc + r;
                  if (state.y_doublescan) pixely >>= 1;
                  for (c=0; c<X_TILESIZE; c++) {
                    pixelx = (xc + c) >> 1;
                    plane  = (pixelx % 4);
                    byte_offset = (plane * 65536) +
                                  (pixely * state.line_offset)
                                  + (pixelx >> 2);
                    color = state.memory[start_addr + byte_offset];
                    state.tile[r*X_TILESIZE + c] = color;
                    }
                  }
                SET_TILE_UPDATED (xti, yti, 0);
                bx_gui->graphics_tile_update(state.tile, xc, yc);
                }
	      }
	    }
          }
        break; // case 2

      default:
        printf("update: shift_reg == %u   \n", (unsigned)
          state.graphics_ctrl.shift_reg );
        exit(1);
      }

    state.vga_mem_updated = 0;
    return;
    }

  else { // text mode
    unsigned long start_address;
    unsigned long cursor_address, cursor_x, cursor_y;
    bx_vga_tminfo_t tm_info;
    unsigned VDE, MSL, cols, rows, cWidth;

    tm_info.start_address = 2*((state.CRTC.reg[12] << 8) +
                            state.CRTC.reg[13]);
    tm_info.cs_start = state.CRTC.reg[0x0a] & 0x3f;
    tm_info.cs_end = state.CRTC.reg[0x0b] & 0x1f;
    tm_info.line_offset = state.CRTC.reg[0x13] << 2;
    tm_info.line_compare = state.line_compare;
    tm_info.h_panning = state.attribute_ctrl.horiz_pel_panning & 0x0f;
    tm_info.v_panning = state.CRTC.reg[0x08] & 0x1f;
    tm_info.line_graphics = state.attribute_ctrl.mode_ctrl.enable_line_graphics;
    tm_info.split_hpanning =  state.attribute_ctrl.mode_ctrl.pixel_panning_compat;
    if ((state.sequencer.reg1 & 0x01) == 0) {
      if (tm_info.h_panning >= 8)
        tm_info.h_panning = 0;
      else
        tm_info.h_panning++;
    } else {
      tm_info.h_panning &= 0x07;
    }

    // Verticle Display End: find out how many lines are displayed
    VDE = state.vertical_display_end;
    // Maximum Scan Line: height of character cell
    MSL = state.CRTC.reg[0x09] & 0x1f;
    if (MSL == 0) {
#if defined(DEBUG_VGA)
      BX_ERROR(("character height = 1, skipping text update"));
#endif
      return;
    }
    cols = state.CRTC.reg[1] + 1;
    if ((MSL == 1) && (VDE == 399)) {
      // emulated CGA graphics mode 160x100x16 colors
      MSL = 3;
    }
    rows = (VDE+1)/(MSL+1);
    if (rows > BX_MAX_TEXT_LINES) {
      BX_PANIC(("text rows>%d: %d",BX_MAX_TEXT_LINES,rows));
      return;
    }
    cWidth = ((state.sequencer.reg1 & 0x01) == 1) ? 8 : 9;
    iWidth = cWidth * cols;
    iHeight = VDE+1;
    if ((iWidth != old_iWidth) || (iHeight != old_iHeight) || (MSL != old_MSL) ||
        (state.last_bpp > 8))
    {
      bx_gui->dimension_update(iWidth, iHeight, MSL+1, cWidth);
      old_iWidth = iWidth;
      old_iHeight = iHeight;
      old_MSL = MSL;
      state.last_bpp = 8;
    }
    // pass old text snapshot & new VGA memory contents
    start_address = 2*((state.CRTC.reg[12] << 8) +
                    state.CRTC.reg[13]);
    cursor_address = 2*((state.CRTC.reg[0x0e] << 8) +
                     state.CRTC.reg[0x0f]);
    if (cursor_address < start_address) {
      cursor_x = 0xffff;
      cursor_y = 0xffff;
    } else {
      cursor_x = ((cursor_address - start_address)/2) % (iWidth/cWidth);
      cursor_y = ((cursor_address - start_address)/2) / (iWidth/cWidth);
    }
    bx_gui->text_update(state.text_snapshot,
                        &state.memory[start_address],
                        cursor_x, cursor_y, tm_info, rows);
    // screen updated, copy new VGA memory contents into text snapshot
    memcpy(state.text_snapshot,
           &state.memory[start_address],
           2*cols*rows);
    state.vga_mem_updated = 0;
  }
}

void CS3Trio64::determine_screen_dimensions(unsigned *piHeight, unsigned *piWidth)
{
  int ai[0x20];
  int i,h,v;
  for ( i = 0 ; i < 0x20 ; i++ )
   ai[i] = state.CRTC.reg[i];

  h = (ai[1] + 1) * 8;
  v = (ai[18] | ((ai[7] & 0x02) << 7) | ((ai[7] & 0x40) << 3)) + 1;

  if (state.graphics_ctrl.shift_reg == 0)
  {
    *piWidth = 640;
    *piHeight = 480;

    if ( state.CRTC.reg[6] == 0xBF )
    {
      if (state.CRTC.reg[23] == 0xA3 &&
         state.CRTC.reg[20] == 0x40 &&
         state.CRTC.reg[9] == 0x41)
      {
        *piWidth = 320;
        *piHeight = 240;
      }
      else {
        if (state.x_dotclockdiv2) h <<= 1;
        *piWidth = h;
        *piHeight = v;
      }
    }
    else if ((h >= 640) && (v >= 480)) {
      *piWidth = h;
      *piHeight = v;
    }
  }
  else if (state.graphics_ctrl.shift_reg == 2)
    {

    if (state.sequencer.chain_four)
    {
      *piWidth = h;
      *piHeight = v;
    }
    else
    {
      *piWidth = h;
      *piHeight = v;
    }
  }
  else
  {
    if (state.x_dotclockdiv2) h <<= 1;
    *piWidth = h;
    *piHeight = v;
  }
}

u8 CS3Trio64::vga_mem_read(u32 addr)
{
  u32 offset;
  u8 *plane0, *plane1, *plane2, *plane3;
  u8 retval = 0;

#if BX_SUPPORT_VBE  
  // if in a vbe enabled mode, read from the vbe_memory
  if ((state.vbe_enabled) && (state.vbe_bpp != VBE_DISPI_BPP_4))
  {
        return vbe_mem_read(addr);
  }
  else if (addr >= VBE_DISPI_LFB_PHYSICAL_ADDRESS)
  {
        return 0xff;
  }
#endif  

#if defined(VGA_TRACE_FEATURE)
//  BX_DEBUG(("8-bit memory read from 0x%08x", addr));
#endif

  switch (state.graphics_ctrl.memory_mapping) {
    case 1: // 0xA0000 .. 0xAFFFF
      if (addr > 0xAFFFF) return 0xff;
      offset = addr & 0xFFFF;
      break;
    case 2: // 0xB0000 .. 0xB7FFF
      if ((addr < 0xB0000) || (addr > 0xB7FFF)) return 0xff;
      offset = addr & 0x7FFF;
      break;
    case 3: // 0xB8000 .. 0xBFFFF
      if (addr < 0xB8000) return 0xff;
      offset = addr & 0x7FFF;
      break;
    default: // 0xA0000 .. 0xBFFFF
      offset = addr & 0x1FFFF;
    }

  if ( state.sequencer.chain_four ) {
    // Mode 13h: 320 x 200 256 color mode: chained pixel representation
    return state.memory[(offset & ~0x03) + (offset % 4)*65536];
  }

#if BX_SUPPORT_VBE  
  if (state.vbe_enabled)
  {
    plane0 = &state.memory[(0<<VBE_DISPI_4BPP_PLANE_SHIFT) + (state.vbe_bank<<16)];
    plane1 = &state.memory[(1<<VBE_DISPI_4BPP_PLANE_SHIFT) + (state.vbe_bank<<16)];
    plane2 = &state.memory[(2<<VBE_DISPI_4BPP_PLANE_SHIFT) + (state.vbe_bank<<16)];
    plane3 = &state.memory[(3<<VBE_DISPI_4BPP_PLANE_SHIFT) + (state.vbe_bank<<16)];
  }
  else
#endif
  {
    plane0 = &state.memory[0<<16];
    plane1 = &state.memory[1<<16];
    plane2 = &state.memory[2<<16];
    plane3 = &state.memory[3<<16];
  }

  /* addr between 0xA0000 and 0xAFFFF */
  switch (state.graphics_ctrl.read_mode) {
    case 0: /* read mode 0 */
      state.graphics_ctrl.latch[0] = plane0[offset];
      state.graphics_ctrl.latch[1] = plane1[offset];
      state.graphics_ctrl.latch[2] = plane2[offset];
      state.graphics_ctrl.latch[3] = plane3[offset];
      retval = state.graphics_ctrl.latch[state.graphics_ctrl.read_map_select];
      break;

    case 1: /* read mode 1 */
      {
      u8 color_compare, color_dont_care;
      u8 latch0, latch1, latch2, latch3;

      color_compare   = state.graphics_ctrl.color_compare & 0x0f;
      color_dont_care = state.graphics_ctrl.color_dont_care & 0x0f;
      latch0 = state.graphics_ctrl.latch[0] = plane0[offset];
      latch1 = state.graphics_ctrl.latch[1] = plane1[offset];
      latch2 = state.graphics_ctrl.latch[2] = plane2[offset];
      latch3 = state.graphics_ctrl.latch[3] = plane3[offset];

      latch0 ^= ccdat[color_compare][0];
      latch1 ^= ccdat[color_compare][1];
      latch2 ^= ccdat[color_compare][2];
      latch3 ^= ccdat[color_compare][3];

      latch0 &= ccdat[color_dont_care][0];
      latch1 &= ccdat[color_dont_care][1];
      latch2 &= ccdat[color_dont_care][2];
      latch3 &= ccdat[color_dont_care][3];

      retval = ~(latch0 | latch1 | latch2 | latch3);
      }
      break;
  }
  return retval;
}

void CS3Trio64::vga_mem_write(u32 addr, u8 value)
{
  u32 offset;
  u8 new_val[4];
  unsigned start_addr;
  u8 *plane0, *plane1, *plane2, *plane3;

#if BX_SUPPORT_VBE
  // if in a vbe enabled mode, write to the vbe_memory
  if ((state.vbe_enabled) && (state.vbe_bpp != VBE_DISPI_BPP_4))
  {
    vbe_mem_write(addr,value);
    return;
  }
  else if (addr >= VBE_DISPI_LFB_PHYSICAL_ADDRESS)
  {
    return;
  }
#endif

#if defined(VGA_TRACE_FEATURE)
//	BX_DEBUG(("8-bit memory write to %08x = %02x", addr, value));
#endif

  switch (state.graphics_ctrl.memory_mapping) {
    case 1: // 0xA0000 .. 0xAFFFF
      if (addr > 0xAFFFF)
        return;
      offset = addr - 0xA0000;
      break;
    case 2: // 0xB0000 .. 0xB7FFF
      if ((addr < 0xB0000) || (addr > 0xB7FFF))
        return;
      offset = addr - 0xB0000;
      break;
    case 3: // 0xB8000 .. 0xBFFFF
      if (addr < 0xB8000)
        return;
      offset = addr - 0xB8000;
      break;
    default: // 0xA0000 .. 0xBFFFF
      offset = addr - 0xA0000;
    }


  start_addr = (state.CRTC.reg[0x0c] << 8) | state.CRTC.reg[0x0d];

  if (state.graphics_ctrl.graphics_alpha) {
    if (state.graphics_ctrl.memory_mapping == 3) { // 0xB8000 .. 0xBFFFF
      unsigned x_tileno, x_tileno2, y_tileno;

      /* CGA 320x200x4 / 640x200x2 start */
      state.memory[offset] = value;
      offset -= start_addr;
      if (offset>=0x2000) {
        y_tileno = offset - 0x2000;
        y_tileno /= (320/4);
        y_tileno <<= 1; //2 * y_tileno;
        y_tileno++;
        x_tileno = (offset - 0x2000) % (320/4);
        x_tileno <<= 2; //*= 4;
      } else {
        y_tileno = offset / (320/4);
        y_tileno <<= 1; //2 * y_tileno;
        x_tileno = offset % (320/4);
        x_tileno <<= 2; //*=4;
      }
      x_tileno2=x_tileno;
      if (state.graphics_ctrl.shift_reg==0) {
        x_tileno*=2;
        x_tileno2+=7;
      } else {
        x_tileno2+=3;
      }
      if (state.x_dotclockdiv2) {
        x_tileno/=(X_TILESIZE/2);
        x_tileno2/=(X_TILESIZE/2);
      } else {
        x_tileno/=X_TILESIZE;
        x_tileno2/=X_TILESIZE;
      }
      if (state.y_doublescan) {
        y_tileno/=(Y_TILESIZE/2);
      } else {
        y_tileno/=Y_TILESIZE;
      }
      state.vga_mem_updated = 1;
      SET_TILE_UPDATED (x_tileno, y_tileno, 1);
      if (x_tileno2!=x_tileno) {
        SET_TILE_UPDATED (x_tileno2, y_tileno, 1);
      }
      return;
      /* CGA 320x200x4 / 640x200x2 end */
      }
    else if (state.graphics_ctrl.memory_mapping != 1) {

      printf("mem_write: graphics: mapping = %u  \n",
               (unsigned) state.graphics_ctrl.memory_mapping);
      exit(1);
      return;
      }

    if ( state.sequencer.chain_four ) {
      unsigned x_tileno, y_tileno;

      // 320 x 200 256 color mode: chained pixel representation
      state.memory[(offset & ~0x03) + (offset % 4)*65536] = value;
      if (state.line_offset > 0) {
        offset -= start_addr;
        x_tileno = (offset % state.line_offset) / (X_TILESIZE/2);
        if (state.y_doublescan) {
          y_tileno = (offset / state.line_offset) / (Y_TILESIZE/2);
        } else {
          y_tileno = (offset / state.line_offset) / Y_TILESIZE;
        }
        state.vga_mem_updated = 1;
        SET_TILE_UPDATED (x_tileno, y_tileno, 1);
      }
      return;
    }
  }

  /* addr between 0xA0000 and 0xAFFFF */

#if BX_SUPPORT_VBE
  if (state.vbe_enabled)
  {
    plane0 = &state.memory[(0<<VBE_DISPI_4BPP_PLANE_SHIFT) + (state.vbe_bank<<16)];
    plane1 = &state.memory[(1<<VBE_DISPI_4BPP_PLANE_SHIFT) + (state.vbe_bank<<16)];
    plane2 = &state.memory[(2<<VBE_DISPI_4BPP_PLANE_SHIFT) + (state.vbe_bank<<16)];
    plane3 = &state.memory[(3<<VBE_DISPI_4BPP_PLANE_SHIFT) + (state.vbe_bank<<16)];
  }
  else
#endif
  {
    plane0 = &state.memory[0<<16];
    plane1 = &state.memory[1<<16];
    plane2 = &state.memory[2<<16];
    plane3 = &state.memory[3<<16];
  }

  switch (state.graphics_ctrl.write_mode) {
    unsigned i;

    case 0: /* write mode 0 */
      {
        const u8 bitmask = state.graphics_ctrl.bitmask;
        const u8 set_reset = state.graphics_ctrl.set_reset;
        const u8 enable_set_reset = state.graphics_ctrl.enable_set_reset;
        /* perform rotate on CPU data in case its needed */
        if (state.graphics_ctrl.data_rotate) {
          value = (value >> state.graphics_ctrl.data_rotate) |
                  (value << (8 - state.graphics_ctrl.data_rotate));
        }
        new_val[0] = state.graphics_ctrl.latch[0] & ~bitmask;
        new_val[1] = state.graphics_ctrl.latch[1] & ~bitmask;
        new_val[2] = state.graphics_ctrl.latch[2] & ~bitmask;
        new_val[3] = state.graphics_ctrl.latch[3] & ~bitmask;
        switch (state.graphics_ctrl.raster_op) {
          case 0: // replace
            new_val[0] |= ((enable_set_reset & 1)
                           ? ((set_reset & 1) ? bitmask : 0)
                           : (value & bitmask));
            new_val[1] |= ((enable_set_reset & 2)
                           ? ((set_reset & 2) ? bitmask : 0)
                           : (value & bitmask));
            new_val[2] |= ((enable_set_reset & 4)
                           ? ((set_reset & 4) ? bitmask : 0)
                           : (value & bitmask));
            new_val[3] |= ((enable_set_reset & 8)
                           ? ((set_reset & 8) ? bitmask : 0)
                           : (value & bitmask));
            break;
          case 1: // AND
            new_val[0] |= ((enable_set_reset & 1)
                           ? ((set_reset & 1)
                              ? (state.graphics_ctrl.latch[0] & bitmask)
                              : 0)
                           : (value & state.graphics_ctrl.latch[0]) & bitmask);
            new_val[1] |= ((enable_set_reset & 2)
                           ? ((set_reset & 2)
                              ? (state.graphics_ctrl.latch[1] & bitmask)
                              : 0)
                           : (value & state.graphics_ctrl.latch[1]) & bitmask);
            new_val[2] |= ((enable_set_reset & 4)
                           ? ((set_reset & 4)
                              ? (state.graphics_ctrl.latch[2] & bitmask)
                              : 0)
                           : (value & state.graphics_ctrl.latch[2]) & bitmask);
            new_val[3] |= ((enable_set_reset & 8)
                           ? ((set_reset & 8)
                              ? (state.graphics_ctrl.latch[3] & bitmask)
                              : 0)
                           : (value & state.graphics_ctrl.latch[3]) & bitmask);
            break;
          case 2: // OR
            new_val[0]
              |= ((enable_set_reset & 1)
                  ? ((set_reset & 1)
                     ? bitmask
                     : (state.graphics_ctrl.latch[0] & bitmask))
                  : ((value | state.graphics_ctrl.latch[0]) & bitmask));
            new_val[1]
              |= ((enable_set_reset & 2)
                  ? ((set_reset & 2)
                     ? bitmask
                     : (state.graphics_ctrl.latch[1] & bitmask))
                  : ((value | state.graphics_ctrl.latch[1]) & bitmask));
            new_val[2]
              |= ((enable_set_reset & 4)
                  ? ((set_reset & 4)
                     ? bitmask
                     : (state.graphics_ctrl.latch[2] & bitmask))
                  : ((value | state.graphics_ctrl.latch[2]) & bitmask));
            new_val[3]
              |= ((enable_set_reset & 8)
                  ? ((set_reset & 8)
                     ? bitmask
                     : (state.graphics_ctrl.latch[3] & bitmask))
                  : ((value | state.graphics_ctrl.latch[3]) & bitmask));
            break;
          case 3: // XOR
            new_val[0]
              |= ((enable_set_reset & 1)
                 ? ((set_reset & 1)
                    ? (~state.graphics_ctrl.latch[0] & bitmask)
                    : (state.graphics_ctrl.latch[0] & bitmask))
                 : (value ^ state.graphics_ctrl.latch[0]) & bitmask);
            new_val[1]
              |= ((enable_set_reset & 2)
                 ? ((set_reset & 2)
                    ? (~state.graphics_ctrl.latch[1] & bitmask)
                    : (state.graphics_ctrl.latch[1] & bitmask))
                 : (value ^ state.graphics_ctrl.latch[1]) & bitmask);
            new_val[2]
              |= ((enable_set_reset & 4)
                 ? ((set_reset & 4)
                    ? (~state.graphics_ctrl.latch[2] & bitmask)
                    : (state.graphics_ctrl.latch[2] & bitmask))
                 : (value ^ state.graphics_ctrl.latch[2]) & bitmask);
            new_val[3]
              |= ((enable_set_reset & 8)
                 ? ((set_reset & 8)
                    ? (~state.graphics_ctrl.latch[3] & bitmask)
                    : (state.graphics_ctrl.latch[3] & bitmask))
                 : (value ^ state.graphics_ctrl.latch[3]) & bitmask);
            break;
          default:
            printf("vga_mem_write: write mode 0: op = %u",
                      (unsigned) state.graphics_ctrl.raster_op);
            exit(1);
        }
      }
      break;

    case 1: /* write mode 1 */
      for (i=0; i<4; i++ ) {
        new_val[i] = state.graphics_ctrl.latch[i];
      }
      break;

    case 2: /* write mode 2 */
      {
        const u8 bitmask = state.graphics_ctrl.bitmask;

        new_val[0] = state.graphics_ctrl.latch[0] & ~bitmask;
        new_val[1] = state.graphics_ctrl.latch[1] & ~bitmask;
        new_val[2] = state.graphics_ctrl.latch[2] & ~bitmask;
        new_val[3] = state.graphics_ctrl.latch[3] & ~bitmask;
        switch (state.graphics_ctrl.raster_op) {
          case 0: // write
            new_val[0] |= (value & 1) ? bitmask : 0;
            new_val[1] |= (value & 2) ? bitmask : 0;
            new_val[2] |= (value & 4) ? bitmask : 0;
            new_val[3] |= (value & 8) ? bitmask : 0;
            break;
          case 1: // AND
            new_val[0] |= (value & 1)
              ? (state.graphics_ctrl.latch[0] & bitmask)
              : 0;
            new_val[1] |= (value & 2)
              ? (state.graphics_ctrl.latch[1] & bitmask)
              : 0;
            new_val[2] |= (value & 4)
              ? (state.graphics_ctrl.latch[2] & bitmask)
              : 0;
            new_val[3] |= (value & 8)
              ? (state.graphics_ctrl.latch[3] & bitmask)
              : 0;
            break;
          case 2: // OR
            new_val[0] |= (value & 1)
              ? bitmask
              : (state.graphics_ctrl.latch[0] & bitmask);
            new_val[1] |= (value & 2)
              ? bitmask
              : (state.graphics_ctrl.latch[1] & bitmask);
            new_val[2] |= (value & 4)
              ? bitmask
              : (state.graphics_ctrl.latch[2] & bitmask);
            new_val[3] |= (value & 8)
              ? bitmask
              : (state.graphics_ctrl.latch[3] & bitmask);
            break;
          case 3: // XOR
            new_val[0] |= (value & 1)
              ? (~state.graphics_ctrl.latch[0] & bitmask)
              : (state.graphics_ctrl.latch[0] & bitmask);
            new_val[1] |= (value & 2)
              ? (~state.graphics_ctrl.latch[1] & bitmask)
              : (state.graphics_ctrl.latch[1] & bitmask);
            new_val[2] |= (value & 4)
              ? (~state.graphics_ctrl.latch[2] & bitmask)
              : (state.graphics_ctrl.latch[2] & bitmask);
            new_val[3] |= (value & 8)
              ? (~state.graphics_ctrl.latch[3] & bitmask)
              : (state.graphics_ctrl.latch[3] & bitmask);
            break;
        }
      }
      break;

    case 3: /* write mode 3 */
      {
        const u8 bitmask = state.graphics_ctrl.bitmask & value;
        const u8 set_reset = state.graphics_ctrl.set_reset;

        /* perform rotate on CPU data */
        if (state.graphics_ctrl.data_rotate) {
          value = (value >> state.graphics_ctrl.data_rotate) |
                  (value << (8 - state.graphics_ctrl.data_rotate));
        }
        new_val[0] = state.graphics_ctrl.latch[0] & ~bitmask;
        new_val[1] = state.graphics_ctrl.latch[1] & ~bitmask;
        new_val[2] = state.graphics_ctrl.latch[2] & ~bitmask;
        new_val[3] = state.graphics_ctrl.latch[3] & ~bitmask;

        value &= bitmask;

        switch (state.graphics_ctrl.raster_op) {
          case 0: // write
            new_val[0] |= (set_reset & 1) ? value : 0;
            new_val[1] |= (set_reset & 2) ? value : 0;
            new_val[2] |= (set_reset & 4) ? value : 0;
            new_val[3] |= (set_reset & 8) ? value : 0;
            break;
          case 1: // AND
            new_val[0] |= ((set_reset & 1) ? value : 0)
              & state.graphics_ctrl.latch[0];
            new_val[1] |= ((set_reset & 2) ? value : 0)
              & state.graphics_ctrl.latch[1];
            new_val[2] |= ((set_reset & 4) ? value : 0)
              & state.graphics_ctrl.latch[2];
            new_val[3] |= ((set_reset & 8) ? value : 0)
              & state.graphics_ctrl.latch[3];
            break;
          case 2: // OR
            new_val[0] |= ((set_reset & 1) ? value : 0)
              | state.graphics_ctrl.latch[0];
            new_val[1] |= ((set_reset & 2) ? value : 0)
              | state.graphics_ctrl.latch[1];
            new_val[2] |= ((set_reset & 4) ? value : 0)
              | state.graphics_ctrl.latch[2];
            new_val[3] |= ((set_reset & 8) ? value : 0)
              | state.graphics_ctrl.latch[3];
            break;
          case 3: // XOR
            new_val[0] |= ((set_reset & 1) ? value : 0)
              ^ state.graphics_ctrl.latch[0];
            new_val[1] |= ((set_reset & 2) ? value : 0)
              ^ state.graphics_ctrl.latch[1];
            new_val[2] |= ((set_reset & 4) ? value : 0)
              ^ state.graphics_ctrl.latch[2];
            new_val[3] |= ((set_reset & 8) ? value : 0)
              ^ state.graphics_ctrl.latch[3];
            break;
        }
      }
      break;

    default:
      printf("vga_mem_write: write mode %u ?",
        (unsigned) state.graphics_ctrl.write_mode);
      exit(1);
  }

  if (state.sequencer.map_mask & 0x0f) {
    state.vga_mem_updated = 1;
    if (state.sequencer.map_mask & 0x01)
      plane0[offset] = new_val[0];
    if (state.sequencer.map_mask & 0x02)
      plane1[offset] = new_val[1];
    if (state.sequencer.map_mask & 0x04) {
      if ((offset & 0xe000) == state.charmap_address) {
        //printf("Updating character map %04x with %02x...\n  ", (offset & 0x1fff), new_val[2]);
        bx_gui->set_text_charbyte((offset & 0x1fff), new_val[2]);
      }
      plane2[offset] = new_val[2];
    }
    if (state.sequencer.map_mask & 0x08)
      plane3[offset] = new_val[3];

    unsigned x_tileno, y_tileno;

    if (state.graphics_ctrl.shift_reg == 2) {
      offset -= start_addr;
      x_tileno = (offset % state.line_offset) * 4 / (X_TILESIZE / 2);
      if (state.y_doublescan) {
        y_tileno = (offset / state.line_offset) / (Y_TILESIZE / 2);
      } else {
        y_tileno = (offset / state.line_offset) / Y_TILESIZE;
      }
      SET_TILE_UPDATED (x_tileno, y_tileno, 1);
    } else {
      if (state.line_compare < state.vertical_display_end) {
        if (state.line_offset > 0) {
          if (state.x_dotclockdiv2) {
            x_tileno = (offset % state.line_offset) / (X_TILESIZE / 16);
          } else {
            x_tileno = (offset % state.line_offset) / (X_TILESIZE / 8);
          }
          if (state.y_doublescan) {
            y_tileno = ((offset / state.line_offset) * 2 + state.line_compare + 1) / Y_TILESIZE;
          } else {
            y_tileno = ((offset / state.line_offset) + state.line_compare + 1) / Y_TILESIZE;
          }
          SET_TILE_UPDATED (x_tileno, y_tileno, 1);
        }
      }
      if (offset >= start_addr) {
        offset -= start_addr;
        if (state.line_offset > 0) {
          if (state.x_dotclockdiv2) {
            x_tileno = (offset % state.line_offset) / (X_TILESIZE / 16);
          } else {
            x_tileno = (offset % state.line_offset) / (X_TILESIZE / 8);
          }
          if (state.y_doublescan) {
            y_tileno = (offset / state.line_offset) / (Y_TILESIZE / 2);
          } else {
            y_tileno = (offset / state.line_offset) / Y_TILESIZE;
          }
          SET_TILE_UPDATED (x_tileno, y_tileno, 1);
        }
      }
    }
  }
}

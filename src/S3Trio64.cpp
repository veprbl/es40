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
#include "vgafont.h"
#include "AliM1543C.h"

extern CAliM1543C * ali;


#define VGA_BASE 0x3c0

unsigned int rom_max;

u8 option_rom[] = {
  0x55, // signature
  0xaa, // signature
  0x40, /* bios length in blocks. Normally the video bios is 32K (64 blocks)
	   but we really don't need that much, so we're going to fill it
	   with zeros beyond our bogus startup code. */
  // code starts here....
  0xcb, // retf right now!
  'H','e','r','e',' ','l','i','v','e','s',' ','t','h','e',' ','b','o','g',
  'u','s',' ','S','3',' ','R','O','M','.',' ',' ','C','o','p','y','r','i',
  'g','h','t',' ','2','0','0','7',
  0xff  // the checksum/end of data placeholder.
};



/**
 * Constructor.
 **/
CS3Trio64::CS3Trio64(CSystem * c): CSystemComponent(c)
{
    c->RegisterClock(this, true);

    /* the VGA I/O ports are at 3b0 -> 3df */
    cSystem->RegisterMemory(this,2,X64(00000801fc0003c0),48);

    /* legacy video address space: A0000 -> bffff */
    cSystem->RegisterMemory(this,4,X64(800000a0000),131072);

    /* Option ROM address space: C0000  */
    cSystem->RegisterMemory(this,5,X64(800000c0000),32768);

    ResetPCI();

    state.video_mode = MODE_TEXT;
    state.cursor_x = state.cursor_y = 0;
    state.cursor_ttl = 2*BLINK_RATE;
    state.screen = NULL;

    // generate the checksum for the option rom.  We also need to know the
    // boundary between the "real" data and the bogus empty data.
    int j = 0;
    int sum = 0;
    while(option_rom[j] != 0xff) {
      sum+=option_rom[j];
      j++;
    }
    rom_max=j;
    sum &= 0xff;
    option_rom[j]=0x100 - sum;

    // put a welcome message into the legacy text-mode framebuffer
    // so we know its actually working.
    u8 message[] = {'T', 0x4f, 'h', 0x4f, 'i', 0x4f, 's', 0x4f, ' ', 0x4f,
		    'i', 0x4f, 's', 0x4f, ' ', 0x4f,
		    't', 0x4f, 'h', 0x4f, 'e', 0x4f, ' ', 0x4f,
		    'E', 0x4f, 'S', 0x4f, '4', 0x4f, '0', 0x4f, ' ', 0x4f,
		    'f', 0x4f, 'r', 0x4f, 'a', 0x4f, 'm', 0x4f, 'e', 0x4f,
		    'b', 0x4f, 'u', 0x4f, 'f', 0x4f, 'f', 0x4f, 'e', 0x4f,
		    'r', 0x4f, '.', 0x4f, 0};
    int i = 0;
    u8 *fb = &state.legacybuffer[0x18000];
    while(message[i] != 0) {
      *fb++ = message[i++];
    }
    setmode();
    screenrefresh();
    printf("%%VGA-I-INIT: S3 Trio 64 Initialized\n");
}

CS3Trio64::~CS3Trio64()
{
}

u64 CS3Trio64::ReadMem(int index, u64 address, int dsize)
{
  int channel = 0;
  switch(index)
    {
    case 1: /* pci config space */
      return endian_bits(config_read(address, dsize), dsize);
    case 2: /* io ports */
      return endian_bits(io_read(address, dsize), dsize);
    case 3: /* pci memory */
      return endian_bits(mem_read(address, dsize), dsize);
    case 4: /* legacy memory */
      return endian_bits(legacy_read(address, dsize), dsize);
    case 5: /* rom */
      return endian_bits(rom_read(address, dsize), dsize);
    }

  return 0;
}

void CS3Trio64::WriteMem(int index, u64 address, int dsize, u64 data)
{
  int channel = 0;
  switch(index)
    {
    case 1:  /* pci config space */
      config_write(address, dsize, endian_bits(data, dsize));
      return;
    case 2:  /* io port */
      io_write(address,dsize,endian_bits(data,dsize));
      return;
    case 3:  /* pci memory */
      mem_write(address,dsize,endian_bits(data,dsize));
      return;
    case 4:  /* legacy memory */
      legacy_write(address,dsize,endian_bits(data,dsize));
      return;
    case 5:  /* rom */
      rom_write(address,dsize,endian_bits(data,dsize));
      return;
    }
}

/**
 * Redraw the screen.
 **/
int CS3Trio64::DoClock()
{
  if(state.refresh_ttl-- == 0) {
    screenrefresh();
    state.refresh_ttl=REFRESH_RATE;
    if(state.config_data[0x3c] != 0xff) {
      cSystem->interrupt(state.config_data[0x3c],true);
    }
  }
  return 0;
}

/**
 * Read from the PCI configuration space.
 **/

u64 CS3Trio64::config_read(u64 address, int dsize)
{
    
  u64 data;
  void * x;

  x = &(state.config_data[address]);


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
    default:
      data = (u64)(*((u64*)x));
      break;
    }
  //printf("S3 Config read: %" LL "x, %d, %" LL "x\n", address, dsize,data);
  return data;
}

/**
 * Write to the PCI configuration space.
 **/

void CS3Trio64::config_write(u64 address, int dsize, u64 data)
{
  void * x;
  void * y;
  u64 t;

  x = &(state.config_data[address]);
  y = &(state.config_mask[address]);

  //printf("S3 Config write: %" LL "x, %d, %" LL "x\n", address, dsize, data);

  //else 
    switch (dsize)
      {
      case 8:
	*((u8*)x) = (*((u8*)x) & ~*((u8*)y)) | ((u8)data & *((u8*)y));
	break;
      case 16:
	*((u16*)x) = (*((u16*)x) & ~*((u16*)y)) | ((u16)data & *((u16*)y));
	break;
      case 32:
	*((u32*)x) = (*((u32*)x) & ~*((u32*)y)) | ((u32)data & *((u32*)y));
	break;
      case 64:
	*((u64*)x) = (*((u64*)x) & ~*((u64*)y)) | ((u64)data & *((u64*)y));
	break;
      }

  /* handle the case where PCI resources are moved */
  if(dsize==32 &&
     ((data & 0xffffffff) != 0xffffffff) &&
     ((data & 0xffffffff) != 0)) 
    {
      switch(address) 
	{
	case 0x10: /* linear frame buffer*/
	  cSystem->RegisterMemory(this,3,t=X64(0000080000000000)+(endian_32(data)&0xfffffffe),VIDEO_RAM_SIZE);
	  printf("%%VGA-I-FB: S3 frame buffer at %" LL "x\n",t);
	  state.video_base=data;
      break;
      }

  }


}


void CS3Trio64::ResetPCI()
{
  int i;

  /* PCI config slots are at:
     0x801fe000000 + slot*0x800
   */

  // put us into bus 0, slot 2.
  cSystem->RegisterMemory(this, 1, X64(00000801fe001000),0x100);

  for (i=0;i<256;i++) 
    {
      state.config_data[i] = 0;
      state.config_mask[i] = 0;
    }

  state.config_data[0x00] = 0x33; // vendor id 5333h
  state.config_data[0x01] = 0x53;
  state.config_data[0x02] = 0x11; // device id 8811h
  state.config_data[0x03] = 0x88;
  state.config_data[0x04] = 0x00; // flags
  state.config_mask[0x04] = 0xff; // this gets written with 7. IO/MEM/MASTER
  state.config_data[0x05] = 0x00; // flags upper.
  state.config_data[0x06] = 0x1f;
  state.config_data[0x07] = 0x01;
  state.config_data[0x08] = 0x02; // revision: trio64.
  state.config_data[0x09] = 0x00;
  state.config_data[0x0a] = 0x00;
  state.config_data[0x0b] = 0x03;
  state.config_data[0x0c] = 0x00; // cacheline size
  state.config_mask[0x0c] = 0xff; // written with ff or 10?
  state.config_data[0x0d] = 0x00; // latency timer
  state.config_mask[0x0d] = 0xff; // written with 10 or ff?
  state.config_data[0x0e] = 0x00;
  state.config_data[0x0f] = 0x00;

  /* 10 - 24 are memory bases. */
  state.config_data[0x10] = 0x00;
  state.config_data[0x11] = 0x00;
  state.config_data[0x12] = 0x00;
  state.config_data[0x13] = 0xf8; // linear framebuffer at f8000000 

  state.config_mask[0x10] = 0x00;
  state.config_mask[0x11] = 0x00;
  state.config_mask[0x12] = 0x00;
  state.config_mask[0x13] = 0xfc; // lfb is changable (64 MB window).

  /* 28 - 2c are for subsystem things */
  /* 30 - expansion rom address */

  /* 35 - 38 are reserved */
  /* 3c = intr line(rw), 3d = intr pin, 3e = min gnt, 3f = max lat */
  state.config_data[0x3c] = 0xff; state.config_mask[0x3c] = 0xff;
  state.config_data[0x3d] = 0x01;
  state.config_data[0x3e] = 0x14;
  state.config_data[0x3f] = 0x28;


}



/**
 * Save state to a Virtual Machine State file.
 **/

void CS3Trio64::SaveState(FILE *f)
{
  fwrite(&state,sizeof(state),1,f);
}

/**
 * Restore state from a Virtual Machine State file.
 **/

void CS3Trio64::RestoreState(FILE *f)
{
  fread(&state,sizeof(state),1,f);
}


/**
 * Read from Framebuffer
 */
u64 CS3Trio64::mem_read(u64 address, int dsize)
{
  u64 data = 0;
  printf("S3 mem read: %" LL "x, %d, %" LL "x\n", address, dsize, data);

  return data;
}

/**
 * Write to Framebuffer
 */
void CS3Trio64::mem_write(u64 address, int dsize, u64 data)
{

  printf("S3 mem write: %" LL "x, %d, %" LL "x\n", address, dsize, data);
  switch(dsize) {
  case 8:
  case 16:
  case 32:
  case 64:
    break;
  }
}

/**
 * Read from I/O Port
 */
u64 CS3Trio64::io_read(u64 address, int dsize)
{
  u64 data = 0;
  printf("S3 io read: %" LL "x, %d, %" LL "x\n", address+VGA_BASE, dsize, data);

  return data;
}

/**
 * Write to I/O Port
 */
void CS3Trio64::io_write(u64 address, int dsize, u64 data)
{
  switch(address+VGA_BASE) {
  case 0x3d4: /* CRTC Controller index (color) */
    data &= 0xff;
    if(data > CRTC_MAX)
      printf("%%VGA-W-RANGE: crtc index %d is out of range.\n",(int)data);
    else
      state.crtc_index= (u8)data;
    break;
  case 0x3d5: /* CRTC Controller data (color) */
    if(state.crtc_index < CRTC_MAX) {
      state.crtc_data[state.crtc_index]=data;
    }
    break;
  default:
    printf("S3 io write: %" LL "x, %d, %" LL "x\n", address+VGA_BASE, dsize, data);
  }

}

/**
 * Read from Legacy Framebuffer
 */
u64 CS3Trio64::legacy_read(u64 address, int dsize)
{
  u64 data = 0;    
  void *x=&(state.legacybuffer[address]);
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
    default:
      data = (u64)(*((u64*)x));
      break;
    }
  //printf("S3 legacy read: %" LL "x, %d, %" LL "x\n", address, dsize, data);
  return data;
}

/**
 * Write to Legacy Framebuffer
 */
void CS3Trio64::legacy_write(u64 address, int dsize, u64 data)
{
  void *x=&(state.legacybuffer[address]);
  //printf("S3 legacy write: %" LL "x, %d, %" LL "x\n", address, dsize, data);
  switch(dsize) {
  case 8:
    *((u8*)x) = (u8)(data & 0xff);
    break;
  case 16:
    *((u16*)x) = (u16)(data & 0xffff);
    break;
  case 32:
    *((u32*)x) = (u32)(data & 0xffffffffff);
    break;
  case 64:
    *((u64*)x) = (u64)data;
    break;
  }
}


/**
 * Read from Option ROM
 */
u64 CS3Trio64::rom_read(u64 address, int dsize)
{
  u64 data = 0x00;  // make it easy for the checksummer.
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
      default:
	data = (u64)(*((u64*)x));
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
void CS3Trio64::rom_write(u64 address, int dsize, u64 data)
{

  //printf("S3 rom write: %" LL "x, %d, %" LL "x --", address, dsize, data);
  switch(dsize) {
  case 8:
  case 16:
  case 32:
  case 64:
    break;
  }
}


/* create a new surface for the new video mode, and destroy the old one
   if necessary */

void CS3Trio64::setmode()
{
  if(state.screen != NULL) {
    // destroy old one?

  }
  switch(state.video_mode) {
  case MODE_TEXT:
    state.screen = SDL_SetVideoMode(640,400,32,SDL_SWSURFACE);
    if(state.screen == NULL) {
      printf("%%VGA-F-VIDMODE: Cannot set text mode.\n");
      exit(1);
    }
    SDL_WM_SetCaption("ES40 text mode console",NULL);
    screenrefresh();

    break;
  default:
    printf("%%VGA-W-VIDMODE: Unknown video mode %d, reset to text.\n",state.video_mode);
    state.video_mode=MODE_TEXT;
    setmode();
  }
}


/* look at the screen mode and the framebuffer and output the data as
   represented in the data. */
void CS3Trio64::screenrefresh()
{
  u8 *buffer;
  int x,y;
  Uint32 *bufp;
  uint8_t *font;

  switch(state.video_mode) {
  case MODE_TEXT:
    if ( SDL_MUSTLOCK(state.screen) ) {
        if ( SDL_LockSurface(state.screen) < 0 ) {
            return;
        }
    }

    buffer = &state.legacybuffer[0x18000];
    for(int i=0; i< 4000; i+=2) {
      u8 c = *(buffer + i);
      u8 attr = *(buffer + i + 1);
      Uint32 bgcolor = SDL_MapRGB(state.screen->format, 
				  ((attr & 0x10)<<3),
				  ((attr & 0x20)<<2),
				  ((attr & 0x40)<<1));
      Uint32 fgcolor = SDL_MapRGB(state.screen->format,
				  ((attr & 0x1)?(attr & 0x8?0xff:0x80):0x00),
				  ((attr & 0x2)?(attr & 0x8?0xff:0x80):0x00),
				  ((attr & 0x4)?(attr & 0x8?0xff:0x80):0x00));
      // draw the letters + attributes.
      font = &(vgafont16[c*16]);
      x = ((i/2) % 80) * 8;
      y = ((i/2) / 80) * 16;
      for(int j=0; j<16; j++) {
	uint8_t row = *font;
	for(int k=0; k<8; k++) {	  
	    bufp = ((Uint32 *)(state.screen->pixels) +
		    (y+j)*(state.screen->pitch/4) + 
		    (x+k)
		    );
	  if(row & 0x80) {
	    *bufp=fgcolor;
	  } else {
	    *bufp=bgcolor;
	  }
	  row <<= 1;
	}
	font++;
      }
    }
    if ( SDL_MUSTLOCK(state.screen) ) {
        SDL_UnlockSurface(state.screen);
    }
    SDL_UpdateRect(state.screen,0,0,640,400);
    break;
  }
}

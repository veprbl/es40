/* ES40 emulator.
 * Copyright (C) 2007 by Camiel Vanderhoeven
 *
 * Website: www.camicom.com
 * E-mail : camiel@camicom.com
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
 * Contains the code for the emulated Dual Port Ram and RMC devices.
 *
 * X-1.10       Camiel Vanderhoeven                             31-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.9        Camiel Vanderhoeven                             16-FEB-2007
 *      Added functions SaveStateF and RestoreStateF.
 *
 * X-1.8	Brian Wheeler					13-FEB-2007
 *	Formatting.
 *
 * X-1.7 	Camiel Vanderhoeven				12-FEB-2007
 *	Added comments.
 *
 * X-1.6        Camiel Vanderhoeven                             9-FEB-2007
 *      Added comments.
 *
 * X-1.5	Camiel Vanderhoeven				7-FEB-2007
 *	Calls to trace_dev now use the TRC_DEVx macro's.
 *
 * X-1.4        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.3        Brian Wheeler                                   3-FEB-2007
 *      64-bit literals made compatible with Linux/GCC/glibc.
 *      
 * X-1.2        Brian Wheeler                                   3-FEB-2007
 *      Includes are now case-correct (necessary on Linux)
 *
 * X-1.1        Camiel Vanderhoeven                             19-JAN-2007
 *      Initial version in CVS.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#include "StdAfx.h"
#include "DPR.h"
#include "System.h"
#include "Serial.h"

extern CSerial * srl[2];

/**
 * Constructor.
 **/

CDPR::CDPR(CSystem * c) : CSystemComponent(c)
{
  u8 i;

  c->RegisterMemory(this, 0, X64(0000080110000000),0x100000); // 16KB
  printf("%%DPR-I-INIT: Dual-Port RAM emulator initialized.\n");
  memset(ram,0,16*1024);
  //
  ram[0x3401] = 1;	// SROM valid

  ram[0] = 1;		// EV6 BIST
  ram[1] = 0x80;	// SROM status
  ram[2] = 1;		// STR status
  ram[3] = 1;		// CSC status
  ram[4] = 1;		// Pchip0 status
  ram[5] = 1;		// Pchip1 status
  ram[6] = 1;		// DIMx status
  ram[7] = 1;		// TIG bus status
  ram[8] = 0xdd;	// DPR test started
  ram[9] = 1;		// DPR status
  ram[10] = 0xff;	// CPU speed status
  ram[11] = 833%256;	//speed
  ram[12] = 833/256;	//speed
  // powerup time BCD:
  ram[16] = 0x08; // uur
  ram[17] = 0x30; // min
  ram[18] = 0x00; // sec
  ram[19] = 0x01; // dag
  ram[20] = 0x02; // maand
  ram[21] = 0x07; // jaar
  ram[0x16] = 0;	// no error
  ram[0x1e] = 0x80; // CPU SROM sync moet 0x80 zijn; anders --> cpu0 startup failure
  ram[0x1f] = 8;	// cach size in MB

  ram[0xda] = 0xaa; // TIG load

  // DIMM config
  ram[0x80] = 0xf0;	// twice-split 8 dimms array 0
  ram[0x81] = 0x01;	// 64 MB
  //	ram[0x82] = 0xf1;	// twice-split 8 dimms array 1
  //	ram[0x83] = 0x01;	// 64 MB
  //	ram[0x84] = 0xf2;	// twice-split 8 dimms array 2
  //	ram[0x85] = 0x01;	// 64 MB
  //	ram[0x86] = 0xf3;	// twice-split 8 dimms array 3
  //	ram[0x87] = 0x01;	// 64 MB

  // powerup failure bits
  ram[0x88] = 0; // each bit is one DIMM on MMB0
  ram[0x89] = 0x00; // MMB1
  ram[0x8a] = 0x00; // MMB2
  ram[0x8b] = 0x00; // MMB3
  // misconfigured DIMM bits
  ram[0x8c] = 0; // each bit is one DIMM on MMB0
  ram[0x8d] = 0; // MMB1
  ram[0x8e] = 0; // MMB2
  ram[0x8f] = 0; // MMB3

  ram[0x90] = 0xff; // psu / vterm present
  ram[0x91] = 0xff; // psu ok bits
  ram[0x92] = 0x07; // ac inputs valid
  ram[0x93] = 0x50; // cpu 0 temp in BCD
  ram[0x94] = 0x50; // cpu 1 temp in BCD
  ram[0x95] = 0x50; // cpu 2 temp in BCD
  ram[0x96] = 0x50; // cpu 3 temp in BCD
  ram[0x97] = 0x36; // pci 0 temp in BCD
  ram[0x98] = 0x36; // pci 1 temp in BCD
  ram[0x99] = 0x36; // pci 2 temp in BCD
  ram[0x9a] = 0x8b; // fan 0 speed
  ram[0x9b] = 0x8b; // fan 1 speed
  ram[0x9c] = 0x8b; // fan 2 speed
  ram[0x9d] = 0x8b; // fan 3 speed
  ram[0x9e] = 0x8b; // fan 4 speed
  ram[0x9f] = 0x8b; // fan 5 speed
	
  // vector 680 info (various faults)
  for (i=0xa0;i<0xaa;i++)
    ram[i] = 0;

  ram[0xaa] = 0x00;	// fans good
  // RMC read failure DIMM bits
  ram[0xab] = 0; // each bit is one DIMM on MMB0
  ram[0xac] = 0xff; // MMB1
  ram[0xad] = 0xff; // MMB2
  ram[0xae] = 0xff; // MMB3

  ram[0xaf] = 0x0e; // all MMB I2C's read + CPU 0
  ram[0xb0] = 0x00; // PCI i2c read
  ram[0xb1] = 0x00; // mainboard i2c read
  ram[0xb2] = 0x00; // psu's and scsi backplanes i2c read

  ram[0xba] = 0xba; // i2c finished

  ram[0xbb] = 0x00;	// rmc error
  ram[0xbc] = 0x00;	//rmc flash update error status

  // 680 fatal registers
  ram[0xbd] = 0x07;	// ac inputs valid
  ram[0xbe] = 0;		// faults
  ram[0xbf] = 0;		// faults

  ram[0xda] = 0xaa;	// tig load success

  // Power-supplies
  ram[0xdb] = 0xf4;	// PS0 id
  ram[0xdc] = 0x45;	// 3.3v current
  ram[0xdd] = 0x51;	// 5.0v current
  ram[0xde] = 0x37;	// 12v current
  ram[0xdf] = 0x8b;	// fan speed
  ram[0xe0] = 0xd6;	// ac voltage (230v)
  ram[0xe1] = 0x49;	// internal temp. (56 C)
  ram[0xe2] = 0x4b;	// inlet temp. (20 C)
	
  ram[0xe4] = 0xf5;	// PS1 id
  ram[0xe5] = 0x45;	// 3.3v current
  ram[0xe6] = 0x51;	// 5.0v current
  ram[0xe7] = 0x37;	// 12v current
  ram[0xe8] = 0x8b;	// fan speed
  ram[0xe9] = 0xd6;	// ac voltage (230v)
  ram[0xea] = 0x49;	// internal temp. (56 C)
  ram[0xeb] = 0x4b;	// inlet temp. (20 C)
	
  ram[0xed] = 0xf6;	// PS2 id
  ram[0xee] = 0x45;	// 3.3v current
  ram[0xef] = 0x51;	// 5.0v current
  ram[0xf0] = 0x37;	// 12v current
  ram[0xf1] = 0x8b;	// fan speed
  ram[0xf2] = 0xd6;	// ac voltage (230v)
  ram[0xf3] = 0x49;	// internal temp. (56 C)
  ram[0xf4] = 0x4b;	// inlet temp. (20 C)

  // EEROMs
  /*
    100: MMB0 DIMM 2
    200: MMB0 DIMM 3
    300: MMB0 DIMM 4
    400: MMB0 DIMM 5
    500: MMB0 DIMM 6
    600: MMB0 DIMM 7
    700: MMB0 DIMM 8
    800: MMB0 DIMM 1
    900: MMB1 DIMM 2
    a00: MMB1 DIMM 3
    b00: MMB1 DIMM 4
    c00: MMB1 DIMM 5
    d00: MMB1 DIMM 6
    e00: MMB1 DIMM 7
    f00: MMB1 DIMM 8
    1000: MMB1 DIMM 1
    1100: MMB2 DIMM 2
    1200: MMB2 DIMM 3
    1300: MMB2 DIMM 4
    1400: MMB2 DIMM 5
    1500: MMB2 DIMM 6
    1600: MMB2 DIMM 7
    1700: MMB2 DIMM 8
    1800: MMB2 DIMM 1
    1900: MMB3 DIMM 2
    1a00: MMB3 DIMM 3
    1b00: MMB3 DIMM 4
    1c00: MMB3 DIMM 5
    1d00: MMB3 DIMM 6
    1e00: MMB3 DIMM 7
    1f00: MMB3 DIMM 8
    2000: MMB3 DIMM 1
    2100: CPU0
    2200: CPU1
    2300: CPU2
    2400: CPU3
    2500: MMB0
    2600: MMB1
    2700: MMB2
    2800: MMB3
    2900: CPB (PCI backplane)
    2a00: CSB (motherboard)
    3100: PSU0 cont @ 3d00
    3200: PSU1 cont @ 3e00
    3300: PSU2 cont @ 3f00
    3b00: SCSI0 (backplane)
    3c00: SCSI1

    2B00:2BFF	RMC Last EV6 Correctable Error 
    ASCII character string that indicates correctable error occurred, type, FRU, and so on.
    2C00:2CFF	RMC Last Redundant Failure
    ASCII character string that indicates redundant failure occurred, type, FRU, and so on.
    2D00:2DFF	RMC Last System Failure
    ASCII character string that indicates system failure occurred, type, FRU, and so on.
    2E00:2FFF	RMC Uncorrectable machine logout frame (512 bytes)
  */

  //	3000:3008	SROM Version (ASCII string)
  ram[0x3000] = 'V';
  ram[0x3001] = '2';
  ram[0x3002] = '.';
  ram[0x3003] = '2';
  ram[0x3004] = '2';
  ram[0x3005] = 'G';
  ram[0x3006] = 0;
  ram[0x3007] = 0;
  ram[0x3008] = 0;
  //	3009:300B	RMC Rev Level of RMC first byte is letter Rev [x/t/v] second 2 bytes are major/minor.
  //				This is the rev level of the RMC on-chip code.
  ram[0x3009] = 'V';
  ram[0x300a] = 0x01;
  ram[0x300b] = 0x00;
  //	300C:300E	RMC Rev Level of RMC first byte is letter Rev [x/t/v] second 2 bytes are major/minor.
  //				This is the rev level of the RMC flash code.
  ram[0x300c] = 'V';
  ram[0x300d] = 0x01;
  ram[0x300e] = 0x00;
  //	300F:3010 300F RMC Revision Field of the DPR Structure

  //	3400 SROM Size of Bcache in MB
  ram[0x3400] = 8;
  //3401 SROM Flash SROM is valid flag; 8 = valid,0 = invalid
  ram[0x3401] = 1;
  //3402 SROM System's errors determined by SROM
  ram[0x3402] = 0;
  //3410:3417 SROM/SRM Jump to address for CPU0
  //3418 SROM/SRM Waiting to jump to flag for CPU0
  //3419 SROM Shadow of value written to EV6 DC_CTL register.
  //341A:341E SROM Shadow of most recent writes to EV6 CBOX "Write-many" chain.
  //34A0:34A7 SROM Array 0 to DIMM ID translation
  //										Bits<4:0>
  //		Bits<7:5>
  //		0 = Exists, No Error			Bits <2:0> =
  //		1 = Expected Missing DIMM			+ 1 (1-8)
  //		2 = Error - Missing DIMM(s)		Bits <4:3> =
  //		4 = Error - Illegal MMB			(0-3) DIMM(s)
  //		6 = Error - Incompatible DIMM(s)
  //	34A8:34AF SROM Repeat for Array 1 of Array 0 34A0:34A7
  //	34B0:34B7 SROM Repeat for Array 2 of Array 0 34A0:34A7
  //	34B8:34CF SROM Repeat for Array 3 of Array 0 34A0:34A7
  for (i=0;i<0x20;i++)
    ram[0x34a0+i] = i;

  //	34C0:34FF	Used as scratch area for SROM
  //	3500:35FF	Used as the dedicated buffer in which SRM writes OCP or FRU EEROM data. 
  //				Firmware will write this data, RMC will only read this data.
  //	3600:36FF 3600 SRM Reserved
  //	3700:37FF SRM Reserved
  //	3800:3AFF RMC RMC scratch space

}

/**
 * Destructor.
 **/

CDPR::~CDPR()
{

}

u64 CDPR::ReadMem(int index, u64 address, int dsize)
{
  u64 data = 0;
  int a = (int)(address>>6);

  data = ram[a];

  TRC_DEV3("%%DPR-I-READ: Dual-Port RAM read @ 0x%08x: 0x%02x\n",a,(u32)(data&0xff));
  return data;
}

void CDPR::WriteMem(int index, u64 address, int dsize, u64 data)
{
  int i;
  int a = (int)(address>>6);
  char trcbuffer[1000];

  TRC_DEV3("%%DPR-I-WRITE: Dual-Port RAM write 0x%08x 0x%02x:\n",a,(u32)(data&0xff));

  // FOR COMMANDS:
  //
  // 0xf9:	buffer size
  // 0xfb:fa	qualifier / address
  // 0xfc:	completion code (0 = ok, 80 = error, 81 = invalid code, 82 = invalid qualifier)
  // 0xfd:	rmc command id for response
  // 0xfe:	command code
  // 0xff:	rmc command id for command
  // COMMANDS:
  // 01:	update EEPROM
  // 02:	update baud rate
  // 03:	write to OCP
  // F0:	update RMC flash
	
  ram[a] = (char)data;
  switch (a)
    {
    case 0xff:
      // command
      ram[0xfd] = ram[0xff];
      switch (ram[0xfe])
	{
	case 1:
	  /*
	    100: MMB0 DIMM 2
	    200: MMB0 DIMM 3
	    300: MMB0 DIMM 4
	    400: MMB0 DIMM 5
	    500: MMB0 DIMM 6
	    600: MMB0 DIMM 7
	    700: MMB0 DIMM 8
	    800: MMB0 DIMM 1
	    900: MMB1 DIMM 2
	    a00: MMB1 DIMM 3
	    b00: MMB1 DIMM 4
	    c00: MMB1 DIMM 5
	    d00: MMB1 DIMM 6
	    e00: MMB1 DIMM 7
	    f00: MMB1 DIMM 8
	    1000: MMB1 DIMM 1
	    1100: MMB2 DIMM 2
	    1200: MMB2 DIMM 3
	    1300: MMB2 DIMM 4
	    1400: MMB2 DIMM 5
	    1500: MMB2 DIMM 6
	    1600: MMB2 DIMM 7
	    1700: MMB2 DIMM 8
	    1800: MMB2 DIMM 1
	    1900: MMB3 DIMM 2
	    1a00: MMB3 DIMM 3
	    1b00: MMB3 DIMM 4
	    1c00: MMB3 DIMM 5
	    1d00: MMB3 DIMM 6
	    1e00: MMB3 DIMM 7
	    1f00: MMB3 DIMM 8
	    2000: MMB3 DIMM 1
	    2100: CPU0
	    2200: CPU1
	    2300: CPU2
	    2400: CPU3
	    2500: MMB0
	    2600: MMB1
	    2700: MMB2
	    2800: MMB3
	    2900: CPB (PCI backplane)
	    2a00: CSB (motherboard)
	    3100: PSU0 cont @ 3d00
	    3200: PSU1 cont @ 3e00
	    3300: PSU2 cont @ 3f00
	    3b00: SCSI0 (backplane)
	    3c00: SCSI1 */
			
	  // FRU-Write
	  switch(ram[0xfb])
	    {
	    case 1:
	    case 2:
	    case 3:
	    case 4:
	    case 5:
	    case 6:
	    case 7:
	    case 8:
	    case 0x21:
	    case 0x25:
	    case 0x26:
	    case 0x27:
	    case 0x28:
	    case 0x29:
	    case 0x2a:
	    case 0x31:
	    case 0x32:
	    case 0x33:
	    case 0x3b:
	    case 0x3c:
	    case 0x3d:
	    case 0x3e:
	    case 0x3f:
	      for(i=0;i<ram[0xf9]+1;i++)
		{
		  ram[ram[0xfb]*0x100 + ram[0xfa] + i] = ram[0x3500 + ram[0xfa] + i];			
		  TRC_DEV4("%%DPR-I-FRU: FRU data %02x @ FRU %02x set to %02x\n",ram[0xfa]+i,ram[0xfb],ram[0x3500 + ram[0xfa]+i]);
		}
	      ram[0xfc] = 0;
	      break;
	    default:
	      TRC_DEV2("%%DPR-I-RMC: RMC Command given: %02x\r\n",ram[0xfe]);
	      TRC_DEV4("%%DPR-I-RMC: f9:%02x fb-fa:%02x%02x\r\n",ram[0xf9],ram[0xfb],ram[0xfa]);
	      ram[0xfc] = 0x80;
	    }
	  break;
	case 2:
	  ram[0xfc] = 0;
	  break;
	case 3:
	  // OCP-Write
	  sprintf(trcbuffer,"%%%%DPR-I-OCP: OCP Text set to \"0123456789abcdef\"\r\n");
	  memcpy(trcbuffer+29,&(ram[0x3500]),16);
	  //			srl[0]->write(trcbuffer);
	  TRC_DEV(trcbuffer);
	  ram[0xfc] = 0;
	  break;
	case 0xf0:
	  ram[0xfc] = 0;
	default:
	  TRC_DEV2("%%DPR-I-RMC: RMC Command given: %02x\r\n",ram[0xfe]);
	  TRC_DEV4("%%DPR-I-RMC: f9:%02x fb-fa:%02x%02x\r\n",ram[0xf9],ram[0xfb],ram[0xfa]);
	  ram[0xfc] = 0x81;
	}
      break;
    case 0xfd:
      // end of command
      ram[0xff] = ram[0xfd];
    }

  return;
}


/**
 * Save state to a Virtual Machine State file.
 **/

void CDPR::SaveState(FILE * f)
{
  fwrite(ram,16*1024,1,f);
}

/**
 * Save state to a DPR rom file.
 **/

void CDPR::SaveStateF(char * fn)
{
  FILE * ff;
  ff = fopen(fn,"wb");
  if (ff)
    {
      SaveState(ff);
      fclose(ff);
      printf("%%DPR-I-SAVEST: DPR state saved to %s\n",fn);
    }
  else
  {
    printf("%%DPR-F-NOSAVE: DPR could not be saved to %s\n",fn);
  }
}

/**
 * Restore state from a DPR rom file.
 **/

void CDPR::RestoreStateF(char * fn)
{
  FILE * ff;
  ff = fopen(fn,"rb");
  if (ff)
    {
      RestoreState(ff);
      fclose(ff);
      printf("%%DPR-I-RESTST: DPR state restored from %s\n",fn);
    }
  else
  {
    printf("%%DPR-F-NOREST: DPR could not be restored from %s\n",fn);
  }
}

/**
 * Restore state from a Virtual Machine State file.
 **/

void CDPR::RestoreState(FILE * f)
{
  fread(ram,16*1024,1,f);
}

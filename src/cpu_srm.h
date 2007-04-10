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
 * Contains code macros for the SRM-replacement pseudo instructions.
 *
 * X-1.2	Camiel Vanderhoeven				10-APR-2007
 *	Unintentional version number increase.
 *
 * X-1.1        Camiel Vanderhoeven                             10-APR-2007
 *      File created. Contains code previously found in AlphaCPU.cpp:
 *	New mechanism for SRM replacements. Where these need to be executed,
 *	CSystem::LoadROM() puts a special opcode (a CALL_PAL instruction
 *	with an otherwise illegal operand of 0x01234xx) in memory. 
 *	CAlphaCPU::DoClock() recognizes these opcodes and performs the SRM
 *	action.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/


#define QQQ(a) cSystem->ReadMem(a,64)
#define LLL(a) cSystem->ReadMem(a,32)
#define WWW(a) cSystem->ReadMem(a,16)
#define BBB(a) cSystem->ReadMem(a,8)

#define DO_SRM_WRITE_SERIAL							\
	temp_32 = (u32)LLL(LLL(LLL(r[16] + 0x68) + 0x34) + 0x2c);               \
        /* temp_32 now contains serial port number */                           \
	temp_char2[0] = ' ';                                                    \
        temp_char2[1] = 0;                                                      \
	if (temp_32<2)                                                          \
	{                                                                       \
	  for (temp_64 = r[19]; temp_64 < r[19]+(r[17]*r[18]); temp_64++)       \
	  {                                                                     \
	    temp_char2[0] = (char)BBB(temp_64);                                 \
	    if (temp_char2[0]=='\n')                                             \
              srl[temp_32]->write("\r");                                        \
	    srl[temp_32]->write(temp_char2);                                    \
	    TRC_DEV4("%%SRM-I-WRITSRL : Write character %02x (%c) on serial port %d.\n",        \
                     temp_char2[0],printable(temp_char2[0]),temp_32);              \
	  }                                                                     \
  	  r[0] = r[17] * r[18];                                                 \
	}

#define DO_SRM_READ_IDE_DISK                                                        \
        temp_64 = QQQ(LLL(r[16] + 0x6c));/* file position */                    \
        temp_32 = (int)LLL(LLL(LLL(LLL(r[16] + 0x68) + 0x34) + 0x14) + 0xac); /* drive */       \
        temp_32_1 = (LLL(LLL(LLL(LLL(LLL(r[16] + 0x68) + 0x34) + 0x14)) + 0x21c)&0x80)?0:1; /* controller */        \
	fseek(ali->get_ide_disk(temp_32_1,temp_32),(long)temp_64,0);            \
	r[0] = fread(cSystem->PtrToMem(r[19]),(size_t)r[17],(size_t)r[18],ali->get_ide_disk(temp_32_1,temp_32)) * r[17];        \
	cSystem->WriteMem(LLL(r[16] + 0x6c),64,ftell(ali->get_ide_disk(temp_32_1,temp_32)));    \
	TRC_DEV5("%%SRM-I-READIDE : Read  %3" LL "d sectors @ IDE %d.%d @ LBA %8d\n",r[18]*r[17]/512,tmp_ctl,tmp_drv,(long)(tmp_fps/512));


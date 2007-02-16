/** ES40 emulator.
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
 * Contains the definitions for the emulated Typhoon Chipset devices.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / www.camicom.com)
 **/

#include "datatypes.h"
#include "SystemComponent.h"
#include "TraceEngine.h"

#if !defined(INCLUDED_SYSTEM_H)
#define INCLUDED_SYSTEM_H

#define MAX_COMPONENTS 100

 /**
 * Structure used for mapping memory ranges to devices.
 **/

struct SMemoryUser {
  CSystemComponent * component;	/**< Device that occupies this range. */
  int index;			/**< Index within the device. 
                                 *   Used by devices that occupy more than one range.
                                 **/
  u64 base;			/**< Address of first byte. */
  u64 length;			/**< Number of bytes in range. */
};

/**
 * Structure used for information about available ROM images.
 **/

struct SROM_data
{
  char * filename;
  u32 header_size;
  u32 cksum;
  u32 image_size;
  u32 compression;
  u64 load_at;
  u32 romsize;
  u32 hdrck;
  u8 id;
  char revision[9];
  char tp[4];
};

/**
 * Structure used for configuration values.
 **/

struct SConfig {
  char *key;		/**< Name of the value. */
  char *value;		/**< Value of the value. */
};

/**
 * Emulated Typhoonchipset.
 **/

class CSystem  
{
 public:
  void DumpMemory(unsigned int filenum);
  char * PtrToMem(u64 address);
  unsigned int get_memory_bits();
  void RestoreState(char * fn);
  void SaveState(char * fn);
  u64 PCI_ReadMem(int pcibus, u32 address, int dsize);
  void PCI_WriteMem(int pcibus, u32 address, int dsize, u64 data);
  void interrupt(int number, bool assert);
  u64 Select_ROM();
  int load_ROM(char * filename);
  int load_ROM2(char* filename, int start_at, u64 load_at, u8 type);
  u64 ReadMem(u64 address, int dsize);
  void WriteMem(u64 address, int dsize, u64 data);
  int Run();
#if defined(IDB)
  int SingleStep();
#endif

  int RegisterMemory(CSystemComponent * component, int index, u64 base, u64 length);
  int RegisterClock(CSystemComponent * component, bool slow);
  int RegisterComponent(CSystemComponent * component);
  int RegisterCPU(class CAlphaCPU * cpu);
	
  CSystem(char *filename);
  void ResetMem(unsigned int membits);

  virtual ~CSystem();
  unsigned int iNumMemoryBits;

  char *FindConfig();
  void LoadConfig(char *filename);
  char *GetConfig(char *key);
  char *GetConfig(char *key, char *defval);


 private:
  int iNumCPUs;
  u8  tig$FwWrite;
  u8  tig$HaltA;
  u8  tig$HaltB;
  u64 p$PLAT[2];
  u64 p$PERRMASK[2];
  u64 p$PCTL[2];
  u64 c$DIM[4];
  u64 c$DRIR;
  u64 c$MISC;
  u64 c$CSC;
  u64 c$TRR;
  u64 c$TDR;
  u64 p$WSBA[2][4];
  u64 p$WSM[2][4];
  u64 p$TBA[2][4];
  u64 d$STR;
  void * memory;
  //	void * memmap;

  int iNumComponents;
  CSystemComponent * acComponents[MAX_COMPONENTS];
  int iNumFastClocks;
  CSystemComponent * acFastClocks[MAX_COMPONENTS];
  int iNumSlowClocks;
  CSystemComponent * acSlowClocks[MAX_COMPONENTS];
  int iNumMemories;
  struct SMemoryUser * asMemories[MAX_COMPONENTS];

  struct SROM_data * asROMs[10];
  int iNumROMs;
  class CAlphaCPU * acCPUs[4];

  struct SConfig *asConfig[30];
  int iNumConfig;

#if defined(IDB)
  int iSingleStep;
#endif
};

#endif // !defined(INCLUDED_SYSTEM_H)

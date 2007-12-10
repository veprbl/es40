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
 * X-1.18       Camiel Vanderhoeven                             10-DEC-2007
 *      Use configurator.
 *
 * X-1.17       Camiel Vanderhoeven                             2-DEC-2007
 *      Added support for code profiling, and for direct operations on the
 *      Tsunami/Typhoon's interrupt registers.
 *
 * X-1.16       Brian Wheeler                                   1-DEC-2007
 *      Added panic.
 *
 * X-1.15       Camiel Vanderhoeven                             16-NOV-2007
 *      Replaced PCI_ReadMem and PCI_WriteMem with PCI_Phys.
 *
 * X-1.14       Camiel Vanderhoeven                             18-APR-2007
 *      Faster lockstep mechanism (send info 50 cpu cycles at a time)
 *
 * X-1.13       Camiel Vanderhoeven                             11-APR-2007
 *      Moved all data that should be saved to a state file to a structure
 *      "state".
 *
 * X-1.12       Camiel Vanderhoeven                             10-APR-2007
 *      Replaced LoadROM(...) and SelectROM() calls with a single LoadROM()
 *      call. (See System.cpp, X-1.23).
 *
 * X-1.11       Camiel Vanderhoeven                             10-APR-2007
 *      Removed obsolete ROM-handling code.
 *
 * X-1.10       Camiel Vanderhoeven                             30-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.9        Camiel Vanderhoeven                             1-MAR-2007
 *      Removes $-sign from variable names. The Sun C-compiler can't handle
 *      these.
 *
 * X-1.8        Camiel Vanderhoeven                             18-FEB-2007
 *      Added iSSCycles variable to handle cycle-counting/slow-clocking in 
 *      single-step mode.
 *
 * X-1.7        Camiel Vanderhoeven                             16-FEB-2007
 *   a) Replaced DoClock with run and single_step.
 *   b) Added support for slow-clocked devices.
 *
 * X-1.6        Camiel Vanderhoeven                             12-FEB-2007
 *      Added comments.
 *
 * X-1.5        Brian Wheeler                                   7-FEB-2007
 *      CSystem constructor takes filename of configuration file as an 
 *      argument.
 *
 * X-1.4        Camiel Vanderhoeven                             7-FEB-2007
 *      Added trace (pointer to CTraceEngine)
 *
 * X-1.3        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.2        Brian Wheeler                                   3-FEB-2007
 *      Add support for configuration file.
 *
 * X-1.1        Camiel Vanderhoeven                             19-JAN-2007
 *      Initial version in CVS.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#include "datatypes.h"
#include "SystemComponent.h"
#include "TraceEngine.h"
#include "Configurator.h"

#if !defined(INCLUDED_SYSTEM_H)
#define INCLUDED_SYSTEM_H

#define MAX_COMPONENTS 100


//#define PROFILE 1

#if defined(PROFILE)
#define PROFILE_FROM      X64(8000)
#define PROFILE_TO        X64(1a81c0)
#define PROFILE_AFTER     X64(200000)
#define PROFILE_BUCKSIZE  16
#define PROFILE_LENGTH    (PROFILE_TO - PROFILE_FROM)
#define PROFILE_INSTS     (PROFILE_LENGTH / 4)
#define PROFILE_BUCKETS   (PROFILE_INSTS / PROFILE_BUCKSIZE)
#define PROFILE_YN(a)     ((a >= PROFILE_FROM) && (a < PROFILE_TO) && profile_started)
#define PROFILE_BUCKET(a) profile_buckets[(a-PROFILE_FROM)/4/PROFILE_BUCKSIZE]
#define PROFILE_DO(a)     if ((a&(~X64(3)))>=PROFILE_AFTER) profile_started = true; if (PROFILE_YN(a)) { PROFILE_BUCKET(a)++; profiled_insts++; } 

extern u64 profile_buckets[PROFILE_BUCKETS];
extern u64 profiled_insts;
extern bool profile_started;
#endif


#if defined(LS_MASTER) || defined(LS_SLAVE)
extern char * dbg_strptr;
#endif

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
  u64 PCI_Phys(int pcibus, u64 address);
  void interrupt(int number, bool assert);
  int LoadROM();
  u64 ReadMem(u64 address, int dsize);
  void WriteMem(u64 address, int dsize, u64 data);
  int Run();
  int SingleStep();

  int RegisterMemory(CSystemComponent * component, int index, u64 base, u64 length);
  int RegisterClock(CSystemComponent * component, bool slow);
  int RegisterComponent(CSystemComponent * component);
  int RegisterCPU(class CAlphaCPU * cpu);
	
  CSystem(CConfigurator * cfg);
  void ResetMem(unsigned int membits);

  virtual ~CSystem();
  unsigned int iNumMemoryBits;

  void panic(char *message, int flags);

#define PANIC_NOSHUTDOWN 0
#define PANIC_SHUTDOWN 1
#define PANIC_ASKSHUTDOWN 2
#define PANIC_LISTING 4

  void clear_clock_int(int ProcNum);
  u64 get_c_misc();
  u64 get_c_dir(int ProcNum);
  u64 get_c_dim(int ProcNum);
  void set_c_dim(int ProcNum,u64 value);

private:
  int iNumCPUs;

  // The state structure contains all elements that need to be saved to the statefile.
  struct SSystemState{
    u8  tig_FwWrite;
    u8  tig_HaltA;
    u8  tig_HaltB;
    u64 p_PLAT[2];
    u64 p_PERRMASK[2];
    u64 p_PCTL[2];
    u64 c_DIM[4];
    u64 c_DRIR;
    u64 c_MISC;
    u64 c_CSC;
    u64 c_TRR;
    u64 c_TDR;
    u64 p_WSBA[2][4];
    u64 p_WSM[2][4];
    u64 p_TBA[2][4];
    u64 d_STR;
  } state;
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

  class CAlphaCPU * acCPUs[4];

  CConfigurator *myCfg;

  int iSingleStep;

#if defined(IDB)
  int iSSCycles;
#endif
};

inline u64 CSystem::get_c_misc()
{
  return state.c_MISC;
}

inline u64 CSystem::get_c_dir(int ProcNum)
{
  return state.c_DRIR & state.c_DIM[ProcNum];
}

inline u64 CSystem::get_c_dim(int ProcNum)
{
  return state.c_DIM[ProcNum];
}

inline void CSystem::set_c_dim(int ProcNum,u64 value)
{
  state.c_DIM[ProcNum] = value;
}

extern CSystem * theSystem;

#endif // !defined(INCLUDED_SYSTEM_H)

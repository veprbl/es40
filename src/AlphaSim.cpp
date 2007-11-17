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
 * Defines the entry point for the application.
 *
 * X-1.27       Camiel Vanderhoeven                             17-NOV-2007
 *      Use FAILURE.
 *
 * X-1.26       Camiel Vanderhoeven                             17-NOV-2007
 *      Version updated to 0.16. Made it possible to disable the network
 *      by defining NO_NETWORK.
 *
 * X-1.25       Camiel Vanderhoeven                             14-NOV-2007
 *      Added network.
 *
 * X-1.24       Camiel Vanderhoeven                             08-NOV-2007
 *      Version updated to 0.15.
 *
 * X-1.23       Camiel Vanderhoeven                             02-NOV-2007
 *      Version updated to 0.14.
 *
 * X-1.22       Camiel Vanderhoeven                             10-APR-2007
 *      Replaced LoadROM(...) and SelectROM() calls with a single LoadROM()
 *      call. (See System.cpp, X-1.23).
 *
 * X-1.21       Camiel Vanderhoeven                             10-APR-2007
 *      Calls to LoadROM and SelectROM changed to fit X-1.22 of System.cpp.
 *
 * X-1.20       Camiel Vanderhoeven                             31-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.19	Camiel Vanderhoeven				28-FEB-2007
 *	Call lockstep_init() if needed.
 *
 * X-1.18	Camiel Vanderhoeven				16-FEB-2007
 *	No longer load CSV files, this can be done with the LOAD CSV command
 *	in the Interactive De-Bugger.
 *
 * X-1.17	Camiel Vanderhoeven				16-FEB-2007
 *   a)	Removed most of the code from this file. The main function no longer
 *	controls every clocktick, but rather transfers control directly to
 *	CSystem::Run(), or to the Interactive Debugger.
 *   b)	In the IDB-version, the name of a script to execute can be specified
 *	on the command line (es40_idb @script).
 *
 * X-1.16	Camiel Vanderhoeven				13-FEB-2007
 *	The define DISASM_START can be set to the number of processor cycles
 *	after which the disassembly will start.
 *
 * X-1.15	Camiel Vanderhoeven				13-FEB-2007
 *	The define DO_SETPC can be set to the address of the first 
 *	instruction to be executed.
 *
 * X-1.14	Camiel Vanderhoeven				12-FEB-2007
 *	Use methods provided by CCPU rather than directly accessing its 
 *	member variables.
 *
 * X-1.13	Camiel Vanderhoeven				12-FEB-2007
 *	Added comments.
 *
 * X-1.12	Camiel Vanderhoeven				9-FEB-2007
 *	Removed an obscure hack.
 *
 * X-1.11       Camiel Vanderhoeven                             9-FEB-2007
 *      If DO_DISASM is set, disassembly will be output.
 *
 * X-1.10	Brian Wheeler					7-FEB-2007
 *	Configuration file may be specified on the command line.
 *
 * X-1.9	Camiel Vanderhoeven				7-FEB-2007
 *	Add instantiation of trace engine.
 *
 * X-1.8	Camiel Vanderhoeven				3-FEB-2007
 *	RUN_LT can be set to provide a breakpoint (end if pc < defined value)
 *
 * X-1.7        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.6	Brian Wheeler					3-FEB-2007
 *	Support for execution speed timing on Linux
 *
 * X-1.5	Brian Wheeler					3-FEB-2007
 *	Support for configuration file.
 *
 * X-1.4	Brian Wheeler					3-FEB-2007
 *	Made execution speed timing dependent on _WIN32 (since this code
 *	uses a lot of Windows-specific features, and won't compile on
 *	Linux.
 *
 * X-1.3        Brian Wheeler                                   3-FEB-2007
 *      Scanf calls made compatible with Linux/GCC/glibc.
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
#include "System.h"
#include "memory.h"
#include "AlphaCPU.h"
#include "Serial.h"
#include "Port80.h"
#include "FloppyController.h"
#include "Flash.h"
#include "AliM1543C.h"
#if !defined(NO_NETWORK)
#include "DEC21143.h"
#endif
#include "DPR.h"
#include "TraceEngine.h"
#include "lockstep.h"

CSystem * systm;
CAlphaCPU * cpu[4];
CSerial * srl[2];
CPort80 * port80;
CFloppyController * fc[2];
CFlash * srom;
CAliM1543C * ali = 0;
#if !defined(NO_NETWORK)
CDEC21143 * nic = 0;
#endif
CDPR * dpr = 0;

// "standard" locations for a configuration file.  This
// will be port specific.
char *path[]={
#if defined(_WIN32)
  ".\\es40.cfg",
  "c:\\es40.cfg",
  "c:\\windows\\es40.cfg",
#else
  "./es40.cfg",
  "/etc/es40.cfg",
  "/usr/etc/es40.cfg",
  "/usr/local/etc/es40.cfg",
#endif
  0
};

int main(int argc, char* argv[])
{
  printf("%%SYS-I-INITSTART: System initialization started.\n");

#if defined(IDB) && (defined(LS_MASTER) || defined(LS_SLAVE))
  lockstep_init();
#endif

#if defined(IDB)
  if ((argc == 2 || argc==3) && argv[1][0] != '@')
#else
  if (argc == 2)
#endif
  {
    systm = new CSystem(argv[1]);
  } else {
    char *filename = 0;
    FILE *f;
    for(int i = 0 ; path[i] ; i++) {
      filename=path[i];
      f=fopen(path[i],"r");
      if(f != NULL) {
	fclose(f);
	filename = path[i];
	break;
      }
    }
    if(filename==NULL)
      FAILURE("%%SYS-E-CONFIG:  Configuration file not found.\n");
    systm = new CSystem(filename);
  }

#if defined(IDB)
  trc = new CTraceEngine(systm);
#endif

  cpu[0] = new CAlphaCPU(systm);

  ali = new CAliM1543C(systm);

  srl[0] = new CSerial(systm, 0);
  srl[1] = new CSerial(systm, 1);
  port80 = new CPort80(systm);
  fc[0] = new CFloppyController(systm, 0);
  fc[1] = new CFloppyController(systm, 1);

  srom = new CFlash(systm);
  dpr = new CDPR(systm);

#if !defined(NO_NETWORK)
  nic = new CDEC21143(systm);
#endif

  systm->LoadROM();

  printf("%%SYS-I-INITEND: System initialization complete.\n");

  printf("\n\n");
  printf("   **======================================================================**\n");
  printf("   ||                                                                      ||\n");
  printf("   ||                             ES40  emulator                           ||\n");
  printf("   ||                              Version 0.16                            ||\n");
  printf("   ||                                                                      ||\n");
  printf("   ||  Copyright (C) 2007 by Camiel Vanderhoeven                           ||\n");
  printf("   ||  Website: www.camicom.com                                            ||\n");
  printf("   ||  E-mail : camiel@camicom.com                                         ||\n");
  printf("   ||                                                                      ||\n");
  printf("   ||  This program is free software; you can redistribute it and/or       ||\n");
  printf("   ||  modify it under the terms of the GNU General Public License         ||\n");
  printf("   ||  as published by the Free Software Foundation; either version 2      ||\n");
  printf("   ||  of the License, or (at your option) any later version.              ||\n");
  printf("   ||                                                                      ||\n");
  printf("   ||  This program is distributed in the hope that it will be useful,     ||\n");
  printf("   ||  but WITHOUT ANY WARRANTY; without even the implied warranty of      ||\n");
  printf("   ||  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       ||\n");
  printf("   ||  GNU General Public License for more details.                        ||\n");
  printf("   ||                                                                      ||\n");
  printf("   ||  You should have received a copy of the GNU General Public License   ||\n");
  printf("   ||  along with this program; if not, write to the Free Software         ||\n");
  printf("   ||  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,          ||\n");
  printf("   ||  MA  02110-1301, USA.                                                ||\n");
  printf("   ||                                                                      ||\n");
  printf("   **======================================================================**\n");
  printf("\n\n");

  srom->RestoreStateF(systm->GetConfig("rom.flash","flash.rom"));
  dpr->RestoreStateF(systm->GetConfig("rom.dpr","dpr.rom"));

#if defined(IDB)
  if (argc>1 && argc<4 && argv[argc-1][0]=='@')
    trc->run_script(argv[argc-1] + 1);
  else
    trc->run_script(NULL);
#else

  if (systm->Run()>0)
  {
    // save flash and dpr rom only if not terminated with a fatal error
    srom->SaveStateF(systm->GetConfig("rom.flash","flash.rom"));
    dpr->SaveStateF(systm->GetConfig("rom.dpr","dpr.rom"));
  }
#endif

  delete systm;

  return 0;
}

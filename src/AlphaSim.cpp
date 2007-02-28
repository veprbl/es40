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
 * \author Camiel Vanderhoeven (camiel@camicom.com / www.camicom.com)
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
  u64 loadat;

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
    if(filename==NULL) {
      printf("%%SYS-E-CONFIG:  Configuration file not found.\n");
      exit(1);
    }
    systm = new CSystem(filename);
  }

#if defined(IDB)
  trc = new CTraceEngine(systm);
#endif

  systm->load_ROM2(systm->GetConfig("rom.srm","cl67srmrom.exe"),0x240,X64(900000),2);

  cpu[0] = new CAlphaCPU(systm);

  ali = new CAliM1543C(systm);

  srl[0] = new CSerial(systm, 0);
  srl[1] = new CSerial(systm, 1);
  port80 = new CPort80(systm);
  fc[0] = new CFloppyController(systm, 0);
  fc[1] = new CFloppyController(systm, 1);

  srom = new CFlash(systm);
  dpr = new CDPR(systm);

  loadat = systm->Select_ROM();
  cpu[0]->set_pc(loadat+1);
  cpu[0]->set_PAL_BASE(loadat);

  printf("%%SYS-I-INITEND: System initialization complete.\n");

  printf("\n\n");
  printf("   **======================================================================**\n");
  printf("   ||                                                                      ||\n");
  printf("   ||                             ES40  emulator                           ||\n");
  printf("   ||                              Version 0.06                            ||\n");
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

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


#ifndef _WIN32
#include <sys/time.h>
#include <stdlib.h>
#endif

//#define DO_LISTING 1
//#define DO_TRACE 1
//#define DO_SAVESTATE "console.vms"
//#define DO_SAVESTATE "console2.vms"
//#define DO_LOADSTATE "console.vms"
//#define DO_LOADSTATE "console2.vms"
//#define RUN_CYCLES 8*1000*1000
//#define RUN_GT X64(400000)
//#define RUN_LT X64(400000)
//#define DO_SETPC X64(200000)
//#define DO_SETPC X64(0)
//#define DO_DISASM 1
//#define DISASM_START 90*1000*1000

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
#ifdef _WIN32
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
#ifdef _WIN32
  LARGE_INTEGER beginning;
  LARGE_INTEGER before;
  LARGE_INTEGER after;
  LARGE_INTEGER diff;
  LARGE_INTEGER freq;
#else
  struct timeval beginning, before, after;
  double t1, t2;
#endif
  double seconds;
  double ops_per_sec;
  FILE * ff;

  printf("%%SYS-I-INITSTART: System initialization started.\n");

#ifdef _WIN32	
  SetThreadAffinityMask(GetCurrentThread(), 1);

  QueryPerformanceFrequency(&freq);
#endif

  if(argc == 2) {
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

#ifdef IDB
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

#ifdef IDB
  trc->read_procfile("es40.csv");
  trc->read_procfile("vms83.csv");
#endif

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

  int i;

  char prf[300];
#ifdef _WIN32
  QueryPerformanceCounter(&before);
  beginning = before;
#else
  gettimeofday(&beginning,NULL);
  gettimeofday(&before,NULL);
#endif



#ifdef DO_LOADSTATE
  systm->RestoreState(DO_LOADSTATE);
#endif

#ifdef DO_DISASM
#ifndef DISASM_START
  bDisassemble = true;
#endif
#endif

#ifdef DO_TRACE
  bTrace = true;
#endif

#ifdef DO_LISTING
  bDisassemble = true;
  bListing = true;
#endif

#ifdef DO_SETPC
  cpu[0]->set_pc(DO_SETPC);
#endif

  ff = fopen(systm->GetConfig("rom.flash","flash.rom"),"rb");
  if (ff)
    {
      srom->RestoreState(ff);
      fclose(ff);
    }
  ff = fopen(systm->GetConfig("rom.dpr","dpr.rom"),"rb");
  if (ff)
    {
      dpr->RestoreState(ff);
      fclose(ff);
    }

  for(i=0;;i++)
    {
#ifdef RUN_CYCLES
      if (i >= RUN_CYCLES)
	break;
#endif

#ifdef RUN_GT
      if (cpu[0]->get_clean_pc() > RUN_GT)
	break;
#endif
#ifdef RUN_LT
	if (cpu[0]->get_clean_pc() < RUN_LT)
	  break;
#endif

      systm->DoClock();

#ifndef DO_LISTING
      // known speedups
      if  (     cpu[0]->get_clean_pc()==X64(14248) 
	     || cpu[0]->get_clean_pc()==X64(14288)
	     || cpu[0]->get_clean_pc()==X64(142c8)
	     || cpu[0]->get_clean_pc()==X64(68320)

	     || cpu[0]->get_clean_pc()==X64(8bb78)	// write in memory test (aa)
	     || cpu[0]->get_clean_pc()==X64(8bc0c)	// write in memory test (bb)
	     || cpu[0]->get_clean_pc()==X64(8bc94)	// write in memory test (00)
	     )
	cpu[0]->next_pc();

#endif

      if ((i&0x1ffff)==0 && i)
	{
#ifdef _WIN32
	  QueryPerformanceCounter(&after);
	  diff.QuadPart = after.QuadPart-before.QuadPart;
			
	  seconds = (after.QuadPart - before.QuadPart)/(double)freq.QuadPart;
#else
	  gettimeofday(&after,NULL);
	  t1 = ((double)(before.tv_sec*1000000)+(double)before.tv_usec)/1000000;
	  t2 = ((double)(after.tv_sec*1000000)+(double)after.tv_usec)/1000000;
	  seconds = t2 - t1;

#endif
	  before=after;

	  ops_per_sec = 0x20000 / seconds;
			
#ifdef _WIN32
	  sprintf(prf,"\r%dK | %8I64x | %e i/s",i/1000,cpu[0]->get_pc(),ops_per_sec);
#else
	  sprintf(prf,"\r%dK | %8llx | %e i/s",i/1000,cpu[0]->get_pc(),ops_per_sec);
#endif
	  srl[1]->write(prf);

#ifdef DO_DISASM
	  /* Start disassembling here! */
	  if(i > DISASM_START) 
	    bDisassemble = true;
#endif

	}


    }
#ifdef _WIN32
  QueryPerformanceCounter(&after);
  seconds = (after.QuadPart - beginning.QuadPart)/(double)freq.QuadPart;
#else
  t1 = ((double)(beginning.tv_sec*1000000)+(double)beginning.tv_usec)/1000000;
  t2 = ((double)(after.tv_sec*1000000)+(double)after.tv_usec)/1000000;
  seconds = t2 - t1;
#endif
  ops_per_sec = i / seconds;
  printf("%d instructions skipped. Time elapsed: %e sec. Avg. speed: %e ins/sec.              \n\n",i,seconds,ops_per_sec);

#ifdef DO_SAVESTATE
  systm->SaveState(DO_SAVESTATE);
#endif


  ff = fopen(systm->GetConfig("rom.flash","flash.rom"),"wb");
  if (ff)
    {
      srom->SaveState(ff);
      fclose(ff);
    }

  ff = fopen(systm->GetConfig("rom.dpr","dpr.rom"),"wb");
  if (ff)
    {
      dpr->SaveState(ff);
      fclose(ff);
    }

  delete systm;


  return 0;
}

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
 * Contains the definitions for the CPU tracing engine.
 *
 * X-1.15       Camiel Vanderhoeven                             30-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.14       Camiel Vanderhoeven                             14-MAR-2007
 *      bListing moved here from CAlphaCPU.
 *
 * X-1.13       Camiel Vanderhoeven                             14-MAR-2007
 *      Added list_all method.
 *
 * X-1.12       Camiel Vanderhoeven                             12-MAR-2007
 *      Added support for TranslationBuffer debugging.
 *
 * X-1.11       Camiel Vanderhoeven                             8-MAR-2007
 *      get_fnc_name now requires CCPU * as an argument.
 *
 * X-1.10       Camiel Vanderhoeven                             3-MAR-2007
 *      Added TRC_DEV6 macro.
 *
 * X-1.9        Camiel Vanderhoeven                             1-MAR-2007
 *      Made a couple of arguments const char *.
 *
 * X-1.8        Camiel Vanderhoeven                             18-FEB-2007
 *      Added bHashing.
 *
 * X-1.7        Camiel Vanderhoeven                             16-FEB-2007
 *      Added support for Interactive Debugger (IDB).
 *
 * X-1.6        Camiel Vanderhoeven                             12-FEB-2007
 *      Formatting.
 *
 * X-1.5        Camiel Vanderhoeven                             12-FEB-2007
 *      Added comments.
 *
 * X-1.4        Camiel Vanderhoeven                             9-FEB-2007
 *      Debugging flqags (booleans) moved here from CAlphaCPU.
 *
 * X-1.3        Camiel Vanderhoeven                             7-FEB-2007
 *      Debugging functions are enabled only when compiling with -DIDB 
 *      (Interactive Debugger, a future feature)
 *
 * X-1.2        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.1        Camiel Vanderhoeven                             19-JAN-2007
 *      Initial version in CVS.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#if !defined(INCLUDED_TRACEENGINE_H)
#define INCLUDED_TRACEENGINE_H

#if defined(IDB)

#include "datatypes.h"

/**
 * Structure used to define named functions within memory.
 **/

struct STraceFunction {
  u32 address;
  char * fn_name;
  char * fn_arglist;
  bool step_over;
};

/**
 * Structure used to keep track of PRBR values.
 **/

struct STracePRBR {
  u64 prbr;
  FILE * f;
  u64 trcadd[701];
  int trclvl;
  int trchide;
  u64 trc_waitfor;
  char procname[30];
  int generation;
};

/**
 * Structure used to keep track of CPU's
 **/

struct STraceCPU {
  int last_prbr;
};

/**
 * CPU tracing engine.
 **/

class CTraceEngine
{
 public:
  void read_procfile(char * filename);
  CTraceEngine(class CSystem * sys);
  ~CTraceEngine(void);
  void trace(class CAlphaCPU * cpu, u64 f, u64 t, bool down, bool up, const char * x, int y);
  void trace_br(class CAlphaCPU * cpu, u64 f, u64 t);
  void add_function(u64 address, char * fn_name, char * fn_arglist, bool step_over);
  bool get_fnc_name(class CAlphaCPU * cpu, u64 address, char ** p_fn_name);
  void set_waitfor(class CAlphaCPU * cpu, u64 address);
  FILE * trace_file();
  void trace_dev(const char * text);
  int parse(char command[100][100]);
  void run_script(char * filename);
  void list_all();

 protected:
  class CSystem * cSystem;
  int trcfncs;
  int iNumFunctions;
  int iNumPRBRs;
  struct STraceFunction asFunctions[25000];
  struct STraceCPU asCPUs[4];
  struct STracePRBR asPRBRs[1000];
  int get_prbr(u64 prbr);
  void write_arglist(CAlphaCPU * c, FILE * f, char * a);
  FILE * current_trace_file;
  u64 iBreakPoint;
  int iBreakPointMode;
  bool bBreakPoint;
};

extern bool bTrace;
extern bool bDisassemble;
extern bool bHashing;
extern bool bListing;

#if defined(DEBUG_TB)
extern bool bTB_Debug;
#endif

extern CTraceEngine * trc;

#define TRC_DEV(a) { if (bTrace) { char t [1000]; sprintf(t, a); trc->trace_dev(t); } }
#define TRC_DEV2(a,b) { if (bTrace) { char t [1000]; sprintf(t,a,b); trc->trace_dev(t); } }
#define TRC_DEV3(a,b,c) { if (bTrace) { char t [1000]; sprintf(t,a,b,c); trc->trace_dev(t); } }
#define TRC_DEV4(a,b,c,d) { if (bTrace) { char t [1000]; sprintf(t,a,b,c,d); trc->trace_dev(t); } }
#define TRC_DEV5(a,b,c,d,e) { if (bTrace) { char t [1000]; sprintf(t,a,b,c,d,e); trc->trace_dev(t); } }
#define TRC_DEV6(a,b,c,d,e,f) { if (bTrace) { char t [1000]; sprintf(t,a,b,c,d,e,f); trc->trace_dev(t); } }

#define DO_ACTION !bListing

#else //IDB

#define TRC_DEV(a) ;
#define TRC_DEV2(a,b) ;
#define TRC_DEV3(a,b,c) ;
#define TRC_DEV4(a,b,c,d) ;
#define TRC_DEV5(a,b,c,d,e) ;
#define TRC_DEV6(a,b,c,d,e,f) ;

#define DO_ACTION 1

#endif

#endif

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
 * Contains the code for the CPU tracing engine.
 * This will become the debugging engine (interactive debugger) soon.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / www.camicom.com)
 **/


#if defined(IDB)

#include "StdAfx.h"
#include "TraceEngine.h"
#include "AlphaCPU.h"
#include "System.h"
#include "DPR.h"
#include "Flash.h"
#include "lockstep.h"

extern CSystem * systm;
extern CAlphaCPU * cpu [4];
extern CDPR * dpr;
extern CFlash * srom;

CTraceEngine * trc;

inline void write_printable_s(char * dest, char * org)
{
  int cnt = 100;
  while (*org && cnt--)
    {
      *(dest++) = printable(*(org++));
    }
  *dest='\0';
}

CTraceEngine::CTraceEngine(CSystem * sys)
{
  int i;
  iNumFunctions = 0;
  iNumPRBRs = 0;
  cSystem = sys;
  for (i=0;i<4;i++)
    {
      asCPUs[0].last_prbr = -1;
    }
  current_trace_file = stdout;
  bBreakPoint = false;
}

CTraceEngine::~CTraceEngine(void)
{
  int i;
  for (i=0;i<iNumPRBRs;i++)
    fclose(asPRBRs[i].f);
}

void CTraceEngine::trace(CAlphaCPU * cpu, u64 f, u64 t, bool down, bool up, const char * x, int y)
{
  int p;
  int o;
  u64 f1;
  u64 t1;
  bool b;

  if (!cpu->get_tb(true)->convert_address(f,&f1,0,false,0,&b, false, false))
    f1 = ((f&X64(fffffffff0000000))==
  	    X64(0000000020000000))?
      f-X64(000000001fe00000):
      (((f&X64(fffffffff0000000))==
        X64(0000000010000000))?
       f-X64(000000000fffe000):f);
  if (!cpu->get_tb(true)->convert_address(t,&t1,0,false,0,&b, false, false))
    t1 = ((t&X64(fffffffff0000000))==
	    X64(0000000020000000))?
      t-X64(000000001fe00000):
      (((t&X64(fffffffff0000000))==
        X64(0000000010000000))?
       t-X64(000000000fffe000):t);


  p = get_prbr(cpu->get_prbr());
  o = asCPUs[cpu->get_cpuid()].last_prbr;
  if (p != o)
    {
      if (o != -1)
	{
	  fprintf(asPRBRs[o].f, "\n==> Switch to PRBR %" LL "x (%s)\n",   asPRBRs[p].prbr, asPRBRs[p].procname);
	  fprintf(asPRBRs[p].f, "    This is PRBR %" LL "x (%s)\n",       asPRBRs[p].prbr, asPRBRs[p].procname);
	  fprintf(asPRBRs[p].f, "<== Switch from PRBR %" LL "x (%s)\n\n", asPRBRs[o].prbr, asPRBRs[o].procname);
	}
      asCPUs[cpu->get_cpuid()].last_prbr = p;
    }

  if (asPRBRs[p].trc_waitfor)
    {
      if ((t&~X64(3))==asPRBRs[p].trc_waitfor)
	asPRBRs[p].trc_waitfor = 0;
      return;
    }
	
  int oldlvl = asPRBRs[p].trclvl;
  int i, j;

  u32 pc_f = (u32)(f1&X64(fffffffc));
  u32 pc_t = (u32)(t1&X64(fffffffc));
	
  if (      (pc_t <= pc_f+4)  
	 && (pc_t >= pc_f) )
    return;
  if (up)
    {
      for (i=oldlvl-1; i>=0;i--)
	{
	  if (   (asPRBRs[p].trcadd[i] == (pc_t-4))
		 || (asPRBRs[p].trcadd[i] == (pc_t))   )
	    {
	      asPRBRs[p].trclvl = i;
	      if (asPRBRs[p].trchide > i)
		asPRBRs[p].trchide = -1;
			
	      if(asPRBRs[p].trchide != -1)
		return;

	      for (j=0;j<oldlvl;j++)
		fprintf(asPRBRs[p].f," ");
	      fprintf(asPRBRs[p].f,"%08x ($r0 = %" LL "x)\n", pc_f, cpu->get_r(0,true));
			
	      for (j=0;j<asPRBRs[p].trclvl;j++)
		fprintf(asPRBRs[p].f," ");
				
	      fprintf(asPRBRs[p].f,"%08x <--\n",pc_t);
	      return;
	    }
	}
    }

  if(asPRBRs[p].trchide != -1)
    return;
 
  if (!down)
    {
      trace_br(cpu,f,t);
      return;
    }
	
  if (oldlvl<700)
    asPRBRs[p].trclvl = oldlvl + 1;
  asPRBRs[p].trcadd[oldlvl] = pc_f;

  if (x)
    {
      for (i=0;i<oldlvl;i++)
	fprintf(asPRBRs[p].f," ");
      fprintf(asPRBRs[p].f,x,y);
      fprintf(asPRBRs[p].f,"\n");
    }
	
  for (i=0;i<oldlvl;i++)
    fprintf(asPRBRs[p].f," ");
  fprintf(asPRBRs[p].f,"%08x -->\n", pc_f);

  for(i=0;i<asPRBRs[p].trclvl;i++)
    fprintf(asPRBRs[p].f," ");
	
  for (i=0;i<iNumFunctions;i++)
    {
      if (asFunctions[i].address == pc_t)
	{
	  fprintf(asPRBRs[p].f,asFunctions[i].fn_name);
	  write_arglist(cpu,asPRBRs[p].f,asFunctions[i].fn_arglist);
	  fprintf(asPRBRs[p].f,"\n");
	  if (asFunctions[i].step_over)
	    asPRBRs[p].trchide = asPRBRs[p].trclvl;
	  return;
	}
    }

  fprintf(asPRBRs[p].f,"%08x",pc_t);
  write_arglist(cpu,asPRBRs[p].f,"(%s|16%, %s|17%, %s|18%, %s|19%)");
  fprintf(asPRBRs[p].f,"\n");
}

void CTraceEngine::trace_br(CAlphaCPU * cpu, u64 f, u64 t)
{
  int p;
  u64 f1;
  u64 t1;
  bool b;

  if (!cpu->get_tb(true)->convert_address(f,&f1,0,false,0,&b, false, false))
    f1 = ((f&X64(fffffffff0000000))==
  	    X64(0000000020000000))?
      f-X64(000000001fe00000):
      (((f&X64(fffffffff0000000))==
        X64(0000000010000000))?
       f-X64(000000000fffe000):f);
  if (!cpu->get_tb(true)->convert_address(t,&t1,0,false,0,&b, false, false))
    t1 = ((t&X64(fffffffff0000000))==
	    X64(0000000020000000))?
      t-X64(000000001fe00000):
      (((t&X64(fffffffff0000000))==
        X64(0000000010000000))?
       t-X64(000000000fffe000):t);
  p = get_prbr(cpu->get_prbr());

  if (asPRBRs[p].trc_waitfor)
    {
      if ((t&~X64(3))==asPRBRs[p].trc_waitfor)
	asPRBRs[p].trc_waitfor = 0;
      return;
    }

  if (asPRBRs[p].trchide != -1)
    return;
    
  if (p!= asCPUs[cpu->get_cpuid()].last_prbr)
    {
      if (asCPUs[cpu->get_cpuid()].last_prbr != -1)
	{
	  fprintf(asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].f, "\n==> Switch to PRBR %" LL "x (%s)\n", cpu->get_prbr(), cSystem->PtrToMem(cpu->get_prbr()+0x154));
	  fprintf(asPRBRs[p].f, "    This is PRBR %" LL "x (%s)\n", cpu->get_prbr(), cSystem->PtrToMem(cpu->get_prbr()+0x154));
	  fprintf(asPRBRs[p].f,"<== Switch from PRBR %" LL "x (%s)\n\n", asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].prbr, cSystem->PtrToMem(asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].prbr+0x154));
	}
      asCPUs[cpu->get_cpuid()].last_prbr = p;
    }

  int i;

  u32 pc_f = (u32)(f1&X64(fffffffc));
  u32 pc_t = (u32)(t1&X64(fffffffc));

  if (   (pc_t > pc_f + 4) 
	 || (pc_t < pc_f))
    {
      for (i=0;i<asPRBRs[p].trclvl;i++)
	fprintf(asPRBRs[p].f," ");
      fprintf(asPRBRs[p].f,"%08x --+\n", pc_f);
      for(i=0;i<asPRBRs[p].trclvl;i++)
	fprintf(asPRBRs[p].f," ");

      for (i=0;i<iNumFunctions;i++)
	if (asFunctions[i].address == pc_t)
	  {
	    fprintf(asPRBRs[p].f,asFunctions[i].fn_name);
	    write_arglist(cpu,asPRBRs[p].f,asFunctions[i].fn_arglist);
	    fprintf(asPRBRs[p].f," <-+\n");
	    if (asFunctions[i].step_over)
	      asPRBRs[p].trchide = asPRBRs[p].trclvl;
	    return;
	  }

      fprintf(asPRBRs[p].f,"%08x <-+\n",pc_t);
    }
}

void CTraceEngine::add_function(u64 address, char * fn_name, char * fn_arglist, bool step_over)
{
  asFunctions[iNumFunctions].address = (u32)address&~3;
  asFunctions[iNumFunctions].fn_name = _strdup(fn_name);
  asFunctions[iNumFunctions].fn_arglist = _strdup(fn_arglist);
  asFunctions[iNumFunctions].step_over = step_over;
  iNumFunctions++;
}

void CTraceEngine::set_waitfor(CAlphaCPU * cpu, u64 address)
{
  int p;
  p = get_prbr(cpu->get_prbr());

  if(asPRBRs[p].trc_waitfor == 0)
    asPRBRs[p].trc_waitfor = address;
}
bool CTraceEngine::get_fnc_name(CAlphaCPU * c, u64 address, char ** p_fn_name)
{
  int i;

  u64 a;
  bool b;
  
  if (c->get_tb(true)->convert_address(address,&a,0,false,0,&b, false, false))
    a = ((address&X64(fffffffff0000000))
  	     ==X64(0000000020000000))?
     address-X64(000000001fe00000):
      (((address&X64(fffffffff0000000))==
        X64(0000000010000000))?
       address-X64(000000000fffe000):address);

  for (i=0;i<iNumFunctions;i++)
    {
      if (asFunctions[i].address == a)
	{
	  *p_fn_name = asFunctions[i].fn_name;
	  return true;
	}
    }
  *p_fn_name = (char *)0;
  return false;
}

int CTraceEngine::get_prbr(u64 prbr)
{
  int i;
  char filename[100];

  for (i=0;i<iNumPRBRs;i++)
    {
      if (asPRBRs[i].prbr == prbr)
	{
	  if (asPRBRs[i].f == current_trace_file)
	    return i;
	  if (!strncmp(asPRBRs[i].procname,cSystem->PtrToMem(prbr+0x154),20))
	    {
	      current_trace_file = asPRBRs[i].f;
	      return i;
	    }
	  fclose(asPRBRs[i].f);
	  break;
	}
    }

  if (i == iNumPRBRs)
    {
      asPRBRs[i].generation = 0;
      iNumPRBRs++;
    }
  asPRBRs[i].generation++;

  asPRBRs[i].prbr = prbr;
  if (prbr > 0 && prbr < (X64(1)<<cSystem->get_memory_bits()))
    strncpy(asPRBRs[i].procname, cSystem->PtrToMem(prbr+0x154), 20);
  else
    strcpy(asPRBRs[i].procname,"");
  sprintf(filename,"trace_%08" LL "x_%02d_%s.trc",prbr,asPRBRs[i].generation,asPRBRs[i].procname);
  asPRBRs[i].f = fopen(filename,"w");
  if (asPRBRs[i].f==0)
    printf("Failed to open file!!\n");
  asPRBRs[i].trclvl = 0;
  asPRBRs[i].trchide = -1;
  asPRBRs[i].trc_waitfor = 0;
  current_trace_file = asPRBRs[i].f;
  printf("Add PRBR: %" LL "x\n",prbr);
  return i;
}

void CTraceEngine::write_arglist(CAlphaCPU * c, FILE * fl, char * a)
{
  char o[500];
  char * op = o;
  char * ap = a;
  char f[20];
  char * fp;
  char * rp;
  int r;
  u64 value;
  u64 phys;
  bool b;

  while (*ap)
    {
      if (*ap=='%')
	{
	  ap++;
	  fp = f;
	  *(fp++) = '%';
	  while (*ap != '%')
	    *(fp++) = *(ap++);
	  ap++;
	  *fp = '\0';
	  // now we have a formatter in f.
	  rp = strchr(f,'|');
	  *(rp++) = '\0';
	  // and the register in rp;
	  r = atoi(rp);
	  value = c->get_r(r,true);
	  if (!strcmp(f,"%s"))
	    {
	      sprintf(op,"%" LL "x (",value);
	      while (*op)
		op++;
	      if (!c->get_tb(false)->convert_address(value,&phys,0,false,0,&b, false, false))
	      {
		      value = phys;
	      }
	      else
	      {
	        if ((value&X64(fffffffff0000000))
		    ==X64(0000000020000000))
		  value-=X64(000000001fe00000);
	        if ((value&X64(fffffffff0000000))
		    ==X64(0000000010000000))
		  value-=X64(000000000fffe000);
	      }
	      if ((value > 0) && (value < (X64(1)<<cSystem->get_memory_bits())))
		write_printable_s(op, cSystem->PtrToMem(value));
	      else
		sprintf(op,"INVPTR");
	      while (*op)
		op++;
	      *(op++) = ')';
	      *(op)='\0';
	    }
	  else if (!strcmp(f,"%c"))
	    sprintf(op,"%02" LL "x (%c)",value,printable((char)value));
	  else if (!strcmp(f,"%d"))
	    sprintf(op,"%" LL "d",value);
	  else if (!strcmp(f,"%x"))
	    sprintf(op,"%" LL "x",value);
	  else if (!strcmp(f,"%0x"))
	    sprintf(op,"%016" LL "x",value);
	  else if (!strcmp(f,"%016x"))
	    sprintf(op,"%016" LL "x",value);
	  else if (!strcmp(f,"%08x"))
	    sprintf(op,"%08" LL "x",value);
	  else if (!strcmp(f,"%04x"))
	    sprintf(op,"%04" LL "x",value);
	  else if (!strcmp(f,"%02x"))
	    sprintf(op,"%02" LL "x",value);
	  else
	    sprintf(op,f,value);
	  while(*op) 
	    op++;
	}
      else
	{
	  *(op++) = *(ap++);
	}
    }
  *op = '\0';

  fprintf(fl,"%s",o);
}

void CTraceEngine::read_procfile(char *filename)
{
  FILE * f;
  u64 address;
  char linebuffer[1000];
  char * fn_name;
  char * fn_args;
  char * sov;
  int step_over;
  int result;

  f = fopen(filename,"r");

  if (f)
    {
      while (fscanf(f,"%[^\n] ",linebuffer) != EOF)
	{
	  address = X64(0);
	  fn_name = strchr(linebuffer,';');
	  if (fn_name)
            {
	      *fn_name = '\0';
	      fn_name++;
	      result = sscanf(linebuffer,"%" LL "x",&address);
	      if ((result == 1) && address)
                {
		  fn_args = strchr(fn_name,';');
		  if (fn_args)
                    {
		      *fn_args = '\0';
		      fn_args++;
		      sov = strchr(fn_args,';');
		      if (sov)
                        {
			  *sov = '\0';
			  sov++;
			  result = sscanf(sov,"%d",&step_over);
			  if (result==1)
                            {
			      add_function(address,fn_name,fn_args,step_over?true:false);
                            }
                        }
                        
                    }
                }

            }
        }
      fclose(f);
    }

}

void CTraceEngine::trace_dev(const char * text)
{
  fprintf(current_trace_file, "%s", text);
}

FILE * CTraceEngine::trace_file()
{
  return current_trace_file;
}

void CTraceEngine::run_script(char * filename)
{
  char s[100][100];
  int i;
  bool u;

#if !defined(LS_SLAVE)

  char c = '\0';
  int j;
  FILE * f = NULL;
  
  if (filename)
  {
     f = fopen(filename,"r");
     if (!f)
     {
       printf("%%IDB-F-NOLOAD: File %s could not be opened.\n",filename);
       return;
     }
  }
  else
  {
    printf("This is the ES40 interactive debugger. To start non-interactively, run es40,\n");
    printf("Or run this executable (es40_idb) with a last argument of @<script-file>\n");
    f = stdin;
  }
#endif

  for (;;)
  {
#if !defined(LS_SLAVE)
    if (filename)
    {
      if (feof(f))
	break;
    }
    else
    {
      printf("IDB %016" LL "x %c>",cpu[0]->get_clean_pc(), (cpu[0]->get_pc()&X64(1))?'P':'-');
    }
#endif

    for (i=0; i<100; )
    {
#if defined(LS_SLAVE)
      lockstep_receive(s[i],100);
      if (!strcmp(s[i],"TERM"))
        break;
#else 
      u = false;
      for (j=0; j<100; )
      {
  	  fscanf(f,"%c",&c);
        if (c != '\n' && c!= '\r' && c!= ' ' && c!= '\t')
	{
	  s[i][j++] = c;
	  u = true;
	}
	if (c == ' ' || c == '\t' || c== '\n')
	  break;
      }
      s[i][j] = '\0';

      if (u) 
#endif
      {
#if defined(LS_MASTER)
        lockstep_send(s[i]);
#endif
        i++;
      }
#if !defined(LS_SLAVE)
      if (c == '\n')
      {
#if defined(LS_MASTER)
        lockstep_send("TERM");
#endif
        break;
      }
#endif
    }
    s[i][0] = '\0';
    if (parse(s))
      break;
  }
#if !defined(LS_SLAVE)
  if (filename)
    fclose(f);
#endif
}

int CTraceEngine::parse(char command[100][100])
{
  int i = 0;
  int numargs;
  int result;
  int RunCycles;
  u64 iFrom;
  u64 iTo;
  u64 iJump;

  for (numargs=0; command[numargs][0] != '\0'; numargs++);

  if ((numargs>0) && (command[0][0]=='#'||command[0][0] ==';' || command[0][0] =='!' 
      || (command[0][0]=='/' && command[0][1]=='/')))
    //comment
    return 0;
  switch (numargs)
  {
  case 0:
    // empty command
    return 0;
  case 1:
    if (!strncasecmp(command[0],"EXIT",strlen(command[0])) ||
	!strncasecmp(command[0],"QUIT",strlen(command[0])))
      return 1;
    if (!strncasecmp(command[0],"HELP",strlen(command[0])) ||
	!strncasecmp(command[0],"?",strlen(command[0])))
    {
      printf("                                                                     \n");
      printf("Available commands:                                                  \n");
      printf("  HELP | ?                                                           \n");
      printf("  EXIT | QUIT                                                        \n");
      printf("  STEP                                                               \n");
      printf("  TRACE [ ON | OFF ]                                                 \n");
      printf("  HASHING [ ON | OFF ]                                               \n");
      printf("  BREAKPOINT [ OFF | > | < | = ] <hex value>                         \n");
      printf("  DISASSEMBLE [ON | OFF ]	                                           \n");
#if defined(DEBUG_TB)
      printf("  TBDEBUG [ON | OFF ]	                                           \n");
#endif
      printf("  LIST [ ALL |<hex address> - <hex address> ]                        \n");
      printf("  RUN [ <max cycles> ]                                               \n");
      printf("  LOAD [ STATE | DPR | FLASH | CSV ] <file>                          \n");
      printf("  SAVE [ STATE | DPR | FLASH ] <file>                                \n");
      printf("  JUMP <hex address>                                                 \n");
      printf("  PAL [ ON | OFF ]                                                   \n"); 
      printf("  DUMPREGS                                                           \n");
      printf("  @<script-file>                                                     \n");
      printf("  # | // | ; | ! <comment>                                           \n");
      printf("                                                                     \n");
      printf("The words of each command can be abbreviated, e.g. B or BRE for      \n");
      printf("BREAKPOINT; S F for save flash.                                      \n");
      printf("                                                                     \n");
      return 0;
    }
    if (!strncasecmp(command[0],"STEP",strlen(command[0])))
    {
      printf("%%IDB-I-SSTEP : Single step.\n");
      systm->SingleStep();
      return 0;
    }
    if (!strncasecmp(command[0],"DUMPREGS",strlen(command[0])))
    {
      printf("\n==================== REGISTER VALUES ====================\n");
      for(i=0;i<32;i++)
      {
	if (i<10) printf("R");
	printf("%d:%016" LL "x", i, cpu[0]->get_r(i,false));
	if (i%4==3) printf("\n"); else printf(" ");
      }
      printf("\n");
      for(i=4;i<8;i++)
      {
	if (i<10) printf("S");
	printf("%d:%016" LL "x", i, cpu[0]->get_r(i+32,false));
	if (i%4==3) printf("\n"); else printf(" ");
      }
      for(i=20;i<24;i++)
      {
	if (i<10) printf("S");
	printf("%d:%016" LL "x", i, cpu[0]->get_r(i+32,false));
	if (i%4==3) printf("\n"); else printf(" ");
      }
      printf("\n");
      for(i=0;i<32;i++)
      {
	if (i<10) printf("F");
	printf("%d:%016" LL "x", i, cpu[0]->get_f(i));
	if (i%4==3) printf("\n"); else printf(" ");
      }
      printf("=========================================================\n");
      return 0;
    }
    if (!strncasecmp(command[0],"RUN",strlen(command[0])))
    {
      if (!bBreakPoint)
      {
	printf("%%IDB-F-NOBRKP: No breakpoint set, and RUN requested without number of cycles.\n");
	return 0;
      }
      printf("%%IDB-I-RUNBPT: Running until breakpoint found.\n");
      switch (iBreakPointMode)
      {
      case -1:
	for (i=0;;i++)
	{
	  if (systm->SingleStep())
	  {
	    printf("%%IDB-I-ABORT : Abort run requested (probably from serial port)\n");
	    break;
	  }
	  if (cpu[0]->get_clean_pc() < iBreakPoint)
	  {
            printf("%%IDB-I-BRKPT : Breakpoint encountered.\n");
	    break;
	  }
	}
	break;
      case 0:
	for(i=0;;i++)
	{
	  if (systm->SingleStep())
	  {
	    printf("%%IDB-I-ABORT : Abort run requested (probably from serial port)\n");
	    break;
	  }
	  if (cpu[0]->get_clean_pc() == iBreakPoint)
	  {
            printf("%%IDB-I-BRKPT : Breakpoint encountered.\n");
	    break;
	  }
	}
	break;
      case 1:
	for(i=0;;i++)
	{
	  if (systm->SingleStep())
	  {
	    printf("%%IDB-I-ABORT : Abort run requested (probably from serial port)\n");
	    break;
	  }
	  if (cpu[0]->get_clean_pc() > iBreakPoint)
	  {
            printf("%%IDB-I-BRKPT : Breakpoint encountered.\n");
	    break;
	  }
	}
	break;
      default:
	break;
      }
      printf("%%IDB-I-ENDRUN: End of run.\n");
      return 0;
    }
#if !defined(IDB) || !defined(LS_SLAVE)
    if (command[0][0]=='@')
    {
      run_script(command[0] + 1);
      return 0;
    }
#endif // !defined(IDB) || !defined(LS_SLAVE)
    break;
  case 2:
    if (!strncasecmp(command[0],"LIST",strlen(command[0])) && !strncasecmp(command[1],"ALL",strlen(command[1])))
      {
	list_all();
	return 0;
      }
    if (!strncasecmp(command[0],"TRACE",strlen(command[0])))
    {
      if (!strcasecmp(command[1],"ON"))
      {
	printf("%%IDB-I-TRCON : Tracing enabled.\n");
	bTrace = true;
	return 0;
      }
      if (!strcasecmp(command[1],"OFF"))
      {
	printf("%%IDB-I-TRCOFF: Tracing disabled.\n");
	bTrace =false;
	return 0;
      }
    }
#if defined(DEBUG_TB)
    if (!strncasecmp(command[0],"TBDEBUG",strlen(command[0])))
    {
      if (!strcasecmp(command[1],"ON"))
      {
	printf("%%IDB-I-TBDON : Translation Buffer Debugging enabled.\n");
	bTB_Debug = true;
	return 0;
      }
      if (!strcasecmp(command[1],"OFF"))
      {
	printf("%%IDB-I-TBDOFF: Translation Buffer Debugging disabled.\n");
	bTB_Debug =false;
	return 0;
      }
    }
#endif
    if (!strncasecmp(command[0],"HASHING",strlen(command[0])))
    {
      if (!strcasecmp(command[1],"ON"))
      {
	printf("%%IDB-I-HSHON : Hashing enabled.\n");
	bHashing = true;
	return 0;
      }
      if (!strcasecmp(command[1],"OFF"))
      {
	printf("%%IDB-I-HSHOFF: Hashing disabled.\n");
	bHashing = false;
	return 0;
      }
    }
    if (!strncasecmp(command[0],"PAL",strlen(command[0])))
    {
      if (!strcasecmp(command[1],"ON"))
      {
	printf("%%IDB-I-PALON : PALmode enabled.\n");
	cpu[0]->set_pc(cpu[0]->get_clean_pc() + 1);
	return 0;
      }
      if (!strcasecmp(command[1],"OFF"))
      {
	printf("%%IDB-I-PALOFF: PALmode disabled.\n");
	cpu[0]->set_pc(cpu[0]->get_clean_pc());
	return 0;
      }
    }
    if (!strncasecmp(command[0],"DISASSEMBLE",strlen(command[0])))
    {
      if (!strcasecmp(command[1],"ON"))
      {
	printf("%%IDB-I-DISON : Disassembling enabled.\n");
	bDisassemble = true;
	return 0;
      }
      if (!strcasecmp(command[1],"OFF"))
      {
	printf("%%IDB-I-DISOFF: Disassembling disabled.\n");
	bDisassemble = false;
	return 0;
      }
    }
    if (!strncasecmp(command[0],"BREAKPOINT",strlen(command[0])))
    {
      if (!strncasecmp(command[1],"OFF",strlen(command[1])))
      {
	printf("%%IDB-I-BRKOFF: Breakpoint disabled.\n");
	bBreakPoint = false;
	return 0;
      }
    }
    if (!strncasecmp(command[0],"RUN",strlen(command[0])))
    {
      result = sscanf(command[1],"%d",&RunCycles);
      if (result != 1)
      {
	printf("%%IDB-F-INVVAL: Invalid decimal value.\n");
	return 0;
      }
      if (bBreakPoint)
      {
        printf("%%IDB-I-RUNCBP: Running until breakpoint found or max cycles reached.\n");
        switch (iBreakPointMode)
	{
        case -1:
	  for (i=0;i<RunCycles;i++)
	  {
	    if (systm->SingleStep())
	    {
	      printf("%%IDB-I-ABORT : Abort run requested (probably from serial port)\n");
	      break;
	    }
 	    if (cpu[0]->get_clean_pc() < iBreakPoint)
	    {
              printf("%%IDB-I-BRKPT : Breakpoint encountered.\n");
	      break;
	    }
	  }
	  break;
        case 0:
	  for(i=0;i<RunCycles;i++)
	  {
	    if (systm->SingleStep())
	    {
	      printf("%%IDB-I-ABORT : Abort run requested (probably from serial port)\n");
	      break;
	    }
	    if (cpu[0]->get_clean_pc() == iBreakPoint)
	    {
              printf("%%IDB-I-BRKPT : Breakpoint encountered.\n");
	      break;
	    }
	  }
	  break;
        case 1:
	  for(i=0;i<RunCycles;i++)
	  {
	    if (systm->SingleStep())
	    {
	      printf("%%IDB-I-ABORT : Abort run requested (probably from serial port)\n");
	      break;
	    }
	    if (cpu[0]->get_clean_pc() > iBreakPoint)
	    {
              printf("%%IDB-I-BRKPT : Breakpoint encountered.\n");
	      break;
	    }
	  }
	  break;
        default:
	  break;
	}
      }
      else
      {
        printf("%%IDB-I-RUNCYC: Running until max cycles reached.\n");
	for(i=0;i<RunCycles;i++)
	  if (systm->SingleStep())
	  {
	    printf("%%IDB-I-ABORT : Abort run requested (probably from serial port)\n");
	    break;
	  }
      }
      printf("%%IDB-I-ENDRUN: End of run.\n");
      return 0;
    }
    if (!strncasecmp(command[0],"JUMP",strlen(command[0])))
    {
      result = sscanf(command[1],"%" LL "x",&iJump);
      if (result != 1)
      {
	printf("%%IDB-F-INVVAL: Invalid hexadecimal value.\n");
	return 0;
      }
      if (iJump&X64(3))
      {
	printf("%%IDB-F-ALGVAL: Value not aligned on a 4-byte bounday.\n");
	return 0;
      }
      printf("%%IDB-I-JUMPTO: Jumping.\n");
      cpu[0]->set_pc(iJump + (cpu[0]->get_pc() & X64(1)));
      return 0;
    }
    break;
  case 3:
    if (!strncasecmp(command[0],"BREAKPOINT",strlen(command[0])))
    {
      if (!strcmp(command[1],"=") ||
	  !strcmp(command[1],">") ||
	  !strcmp(command[1],"<"))
      {
	result = sscanf(command[2],"%" LL "x",&iBreakPoint);
	if (result != 1)
	{
	  printf("%%IDB-F-INVVAL: Invalid hexadecimal value.\n");
	  bBreakPoint = false;
	  return 0;
	}
	if (iBreakPoint&X64(3))
	{
	  printf("%%IDB-F-ALGVAL: Value not aligned on a 4-byte bounday.\n");
	  bBreakPoint = false;
	  return 0;
	}
	switch (command[1][0])
	{
	case '=':
	  iBreakPointMode = 0;
	  break;
	case '>':
	  iBreakPointMode = 1;
	  break;
	case '<':
	  iBreakPointMode = -1;
	  break;
	}
	printf("%%IDB-I-BRKSET: Breakpoint set when PC %c %016" LL "x.\n",command[1][0],iBreakPoint);
	bBreakPoint = true;
	return 0;
      }
    }
    if (!strncasecmp(command[0],"LOAD",strlen(command[0])))
    {
      if (!strncasecmp(command[1],"CSV",strlen(command[1])))
      {
        read_procfile(command[2]);
	return 0;
      }
      if (!strncasecmp(command[1],"STATE",strlen(command[1])))
      {
	systm->RestoreState(command[2]);
	return 0;
      }
      if (!strncasecmp(command[1],"DPR",strlen(command[1])))
      {
	dpr->RestoreStateF(command[2]);
	return 0;
      }
      if (!strncasecmp(command[1],"FLASH",strlen(command[1])))
      {
	srom->RestoreStateF(command[2]);
	return 0;
      }
    }
    if (!strncasecmp(command[0],"SAVE",strlen(command[0])))
    {
      if (!strncasecmp(command[1],"STATE",strlen(command[1])))
      {
	systm->SaveState(command[2]);
	return 0;
      }
      if (!strncasecmp(command[1],"DPR",strlen(command[1])))
      {
	dpr->SaveStateF(command[2]);
	return 0;
      }
      if (!strncasecmp(command[1],"FLASH",strlen(command[1])))
      {
	srom->SaveStateF(command[2]);
	return 0;
      }
    }
    break;
  case 4:
    if (!strncasecmp(command[0],"LIST",strlen(command[0])) && !strcmp(command[2],"-"))
    {
      result = sscanf(command[1],"%" LL "x",&iFrom);
      if (result==1)
	result =  sscanf(command[3],"%" LL "x",&iTo);
      if (result != 1)
      {
        printf("%%IDB-F-INVVAL: Invalid hexadecimal value.\n");
	return 0;
      }
      if (iFrom&X64(3) || iTo&X64(3))
      {
        printf("%%IDB-F-ALGVAL: Value not aligned on a 4-byte bounday.\n");
	return 0;
      }
      if (iFrom > iTo)
      {
	printf("%%IDB-F-FRLTTO: From value exceeds to value.\n");
	return 0;
      }
      cpu[0]->listing(iFrom,iTo);
      return 0;
    }
    break;
  default:
    break;
  }
  printf("%%IDB-F-SYNTAX: Syntax error. Type \"?\" or \"HELP\" for help.\n");
  return 0;
}

struct sRegion
{
  u64 from;
  u64 to;
  struct sRegion * pNext;
};

void CTraceEngine::list_all()
{
  struct sRegion * pR = NULL;
  struct sRegion ** ppN = &pR;
  struct sRegion *p = NULL;
  int f = 0;
  int t = 0;
  int ms   = 1<<(systm->get_memory_bits() - 3);
  u64 * pM = (u64*) systm->PtrToMem(0);
  

  for (;;)
  {
    while (!pM[f] && f<ms)
    {
      f++;
      if (!(f&0x1ffff)) printf(".");
    }
    if (f>=ms)
      break;
    t = f;
    for(;;)
    {
      while (pM[t] && t<ms)
      {
        t++;
        if (!(t&0x1ffff)) printf("x");
      }
      if (t+3<ms && !pM[t+1] && !pM[t+2] && !pM[t+3])
	break;
      t++;
      if (!(t&0x1ffff)) printf("x");
    }
    *ppN = new sRegion;
    (*ppN)->from = (u64)f*8;
    (*ppN)->to = (u64)((t-1)*8)+4;
    (*ppN)->pNext = NULL;
    ppN = &((*ppN)->pNext);
    f = t;
  }
  printf("\n");

  p = pR;

  while (p)
  {
    printf("\n======== DISASSEMBLING %08" LL "x TO %08" LL "x ========\n\n",p->from, p->to);
    cpu[0]->listing(p->from,p->to);
    p = p->pNext;
  }
}

bool bTrace = false;
bool bDisassemble = false;
bool bHashing = false;

#if defined(DEBUG_TB)
bool bTB_Debug = false;
#endif

#endif // IDB

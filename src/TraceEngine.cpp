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
 * 
 * TRACEENGINE.CPP contains the code for the cpu tracing engine.
 *
 **/

#include "StdAfx.h"
#include "TraceEngine.h"
#include "AlphaCPU.h"
#include "System.h"

#ifndef _WIN32
#define _strdup strdup
#include <stdlib.h>
#endif

inline void write_printable_s(char * dest, char * org)
{
  while (*org)
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
}

CTraceEngine::~CTraceEngine(void)
{
  int i;
  for (i=0;i<iNumPRBRs;i++)
    fclose(asPRBRs[i].f);
}

void CTraceEngine::trace(CAlphaCPU * cpu, u64 f, u64 t, bool down, bool up, char * x, int y)
{
  int p;
  u64 f1 = ((f&X64(fffffffff0000000))==
	    X64(0000000020000000))?
    f-X64(000000001fe00000):
    (((f&X64(fffffffff0000000))==
      X64(0000000010000000))?
     f-X64(000000000fffe000):f);
  u64 t1 = ((t&X64(fffffffff0000000))==
	    X64(0000000020000000))?
    t-X64(000000001fe00000):
    (((t&X64(fffffffff0000000))==
      X64(0000000010000000))?
     t-X64(000000000fffe000):t);


  p = get_prbr(cpu->get_prbr());
  if (p!= asCPUs[cpu->get_cpuid()].last_prbr)
    {
      if (asCPUs[cpu->get_cpuid()].last_prbr != -1)
	{
#ifdef _WIN32
	  fprintf(asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].f, "\n==> Switch to PRBR %I64x (%s)\n", cpu->get_prbr(), cSystem->PtrToMem(cpu->get_prbr()+0x154));
#else
	  fprintf(asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].f, "\n==> Switch to PRBR %llx (%s)\n", cpu->get_prbr(), cSystem->PtrToMem(cpu->get_prbr()+0x154));
#endif
#ifdef _WIN32
	  fprintf(asPRBRs[p].f, "    This is PRBR %I64x (%s)\n", cpu->get_prbr(), cSystem->PtrToMem(cpu->get_prbr()+0x154));
#else
	  fprintf(asPRBRs[p].f, "    This is PRBR %llx (%s)\n", cpu->get_prbr(), cSystem->PtrToMem(cpu->get_prbr()+0x154));
#endif
#ifdef _WIN32
	  fprintf(asPRBRs[p].f,"<== Switch from PRBR %I64x (%s)\n\n", asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].prbr, cSystem->PtrToMem(asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].prbr+0x154));
#else
	  fprintf(asPRBRs[p].f,"<== Switch from PRBR %llx (%s)\n\n", asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].prbr, cSystem->PtrToMem(asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].prbr+0x154));
#endif
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
	
  if (   ((pc_t - pc_f) <= 4) 
	 && ((pc_t - pc_f) >= 0))
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
#ifdef _WIN32
	      fprintf(asPRBRs[p].f,"%08x ($r0 = %I64x)\n", pc_f, cpu->get_r(0,true));
#else
	      fprintf(asPRBRs[p].f,"%08x ($r0 = %llx)\n", pc_f, cpu->get_r(0,true));
#endif
			
	      for (j=0;j<asPRBRs[p].trclvl;j++)
		fprintf(asPRBRs[p].f," ");
				
	      fprintf(asPRBRs[p].f,"%08x <--\n",pc_t);
	      return;
	    }
	}
    }
	
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

#ifdef _WIN32
  fprintf(asPRBRs[p].f,"%08x  (r27 = %I64x, r16 = %I64x, r17 = %I64x, r18 = %I64x, )\n",pc_t, cpu->get_r(27,true), cpu->get_r(16,true), cpu->get_r(17,true), cpu->get_r(18,true));
#else
  fprintf(asPRBRs[p].f,"%08x  (r27 = %llx, r16 = %llx, r17 = %llx, r18 = %llx, )\n",pc_t, cpu->get_r(27,true), cpu->get_r(16,true), cpu->get_r(17,true), cpu->get_r(18,true));
#endif
}

void CTraceEngine::trace_br(CAlphaCPU * cpu, u64 f, u64 t)
{
  int p;
  u64 f1 = ((f&X64(fffffffff0000000))==
	    X64(0000000020000000))?
    f-X64(000000001fe00000):
    (((f&X64(fffffffff0000000))==
      X64(0000000010000000))?
     f-X64(000000000fffe000):f);
  u64 t1 = ((t&X64(fffffffff0000000))==
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
#ifdef _WIN32
	  fprintf(asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].f, "\n==> Switch to PRBR %I64x (%s)\n", cpu->get_prbr(), cSystem->PtrToMem(cpu->get_prbr()+0x154));
#else
	  fprintf(asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].f, "\n==> Switch to PRBR %llx (%s)\n", cpu->get_prbr(), cSystem->PtrToMem(cpu->get_prbr()+0x154));
#endif
#ifdef _WIN32
	  fprintf(asPRBRs[p].f, "    This is PRBR %I64x (%s)\n", cpu->get_prbr(), cSystem->PtrToMem(cpu->get_prbr()+0x154));
#else
	  fprintf(asPRBRs[p].f, "    This is PRBR %llx (%s)\n", cpu->get_prbr(), cSystem->PtrToMem(cpu->get_prbr()+0x154));
#endif
#ifdef _WIN32
	  fprintf(asPRBRs[p].f,"<== Switch from PRBR %I64x (%s)\n\n", asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].prbr, cSystem->PtrToMem(asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].prbr+0x154));
#else
	  fprintf(asPRBRs[p].f,"<== Switch from PRBR %llx (%s)\n\n", asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].prbr, cSystem->PtrToMem(asPRBRs[asCPUs[cpu->get_cpuid()].last_prbr].prbr+0x154));
#endif
	}
      asCPUs[cpu->get_cpuid()].last_prbr = p;
    }

  int i;

  u32 pc_f = (u32)(f1&X64(fffffffc));
  u32 pc_t = (u32)(t1&X64(fffffffc));

  if (   ((pc_t - pc_f) > 4) 
	 || ((pc_t - pc_f) < 0))
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
bool CTraceEngine::get_fnc_name(u64 address, char ** p_fn_name)
{
  int i;
  u64 a = ((address&X64(fffffffff0000000))
	   ==X64(0000000020000000))?
    address-X64(000000001fe00000):
    (((address&X64(fffffffff0000000))==
      X64(0000000010000000))?
     address-X64(000000000fffe000):address);

    
  address;

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
	  if (!strcmp(asPRBRs[i].procname,cSystem->PtrToMem(prbr+0x154)))
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
  sprintf(asPRBRs[i].procname,"%s",cSystem->PtrToMem(prbr+0x154));
#ifdef _WIN32
  sprintf(filename,"trace_%08I64x_%02d_%s.trc",prbr,asPRBRs[i].generation,asPRBRs[i].procname);
#else
  sprintf(filename,"trace_%08llx_%02d_%s.trc",prbr,asPRBRs[i].generation,asPRBRs[i].procname);
#endif
  asPRBRs[i].f = fopen(filename,"w");
  if (asPRBRs[i].f==0)
    printf("Failed to open file!!\n");
  asPRBRs[i].trclvl = 0;
  asPRBRs[i].trchide = -1;
  asPRBRs[i].trc_waitfor = 0;
  current_trace_file = asPRBRs[i].f;
#ifdef _WIN32
  printf("Add PRBR: %I64x\n",prbr);
#else
  printf("Add PRBR: %llx\n",prbr);
#endif
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
#ifdef _WIN32
	      sprintf(op,"%I64x (",value);
#else
	      sprintf(op,"%llx (",value);
#endif
	      while (*op)
		op++;
	      if ((value&X64(fffffffff0000000))
		  ==X64(0000000020000000))
		value-=X64(000000001fe00000);
	      if ((value&X64(fffffffff0000000))
		  ==X64(0000000010000000))
		value-=X64(000000000fffe000);
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
#ifdef _WIN32
	    sprintf(op,"%02I64x (%c)",value,printable((char)value));
#else
	  sprintf(op,"%02llx (%c)",value,printable((char)value));
#endif
	  else if (!strcmp(f,"%d"))
#ifdef _WIN32
	    sprintf(op,"%I64d",value);
#else
	  sprintf(op,"%lld",value);
#endif
	  else if (!strcmp(f,"%x"))
#ifdef _WIN32
	    sprintf(op,"%I64x",value);
#else
	  sprintf(op,"%llx",value);
#endif
	  else if (!strcmp(f,"%0x"))
#ifdef _WIN32
	    sprintf(op,"%016I64x",value);
#else
	  sprintf(op,"%016llx",value);
#endif
	  else if (!strcmp(f,"%016x"))
#ifdef _WIN32
	    sprintf(op,"%016I64x",value);
#else
	  sprintf(op,"%016llx",value);
#endif
	  else if (!strcmp(f,"%08x"))
#ifdef _WIN32
	    sprintf(op,"%08I64x",value);
#else
	  sprintf(op,"%08llx",value);
#endif
	  else if (!strcmp(f,"%04x"))
#ifdef _WIN32
	    sprintf(op,"%04I64x",value);
#else
	  sprintf(op,"%04llx",value);
#endif
	  else if (!strcmp(f,"%02x"))
#ifdef _WIN32
	    sprintf(op,"%02I64x",value);
#else
	  sprintf(op,"%02llx",value);
#endif
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
#ifdef _WIN32
	      result = sscanf(linebuffer,"%I64x",&address);
#else
	      result = sscanf(linebuffer,"%llx",&address);
#endif
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

void CTraceEngine::trace_dev(char * text)
{
  fprintf(current_trace_file,text);
}

FILE * CTraceEngine::trace_file()
{
  return current_trace_file;
}

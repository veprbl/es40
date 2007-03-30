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
 * Contains the definitions for the emulated on-cpu translation buffers.
 *
 * X-1.12       Camiel Vanderhoeven                             30-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.11       Camiel Vanderhoeven                             12-MAR-2007
 *      Added support for "smart" Translation Buffers.
 *
 * X-1.10       Camiel Vanderhoeven                             8-MAR-2007
 *   a) Inline functions turned into normal functions, and moved to 
 *      TranslationBuffer.cpp
 *   b) TranslationBuffer functions get their asn information from 
 *      CAlphaCPU, rather than getting the ASN number as an argument.
 *
 * X-1.9        Camiel Vanderhoeven                             22-FEB-2007
 *   a) convert_address returns the ASM bit in a pointer-to-boolean argument.
 *   b) convert_address now also may return E_FAULT when FOR or FOW is set in
 *      the PTE.
 *
 * X-1.8        Camiel Vanderhoeven                             16-FEB-2007
 *      Changed header guards
 *
 * X-1.7        Camiel Vanderhoeven                             12-FEB-2007
 *      Members p_mask and v_mask are now private.
 *
 * X-1.6        Camiel Vanderhoeven                             12-FEB-2007
 *      Added comments.
 *
 * X-1.5        Camiel Vanderhoeven                             7-FEB-2007
 *      Added comments.
 *
 * X-1.4        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.3        Brian Wheeler                                   3-FEB-2007
 *      Constants made Linux-compatible
 *
 * X-1.2        Brian Wheeler                                   3-FEB-2007
 *      Includes are now case-correct (necessary on Linux)
 *
 * X-1.1        Camiel Vanderhoeven                             19-JAN-2007
 *      Initial version in CVS.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#if !defined(INCLUDED_TRANSLATIONBUFFER_H)
#define INCLUDED_TRANSLATIONBUFFER_H

#include "datatypes.h"
#include "System.h"

#define TB_ENTRIES	1024

#define CM_KERNEL	0
#define CM_EXECUTIVE	1
#define CM_SUPERVISOR	2
#define CM_USER		3

#define ACCESS_READ	0
#define ACCESS_WRITE	1

#define E_ACCESS	1
#define E_NOT_FOUND	2
#define E_FAULT		3

#define MBOX_PHYS_S	19
#define ASM_BIT		4
#define GH_BIT		5
#define ACCESS_BIT	8
#define FAULT_BIT	1

/**
 * Translation Buffer Entry.
 * A translation buffer entry provides the mapping from a page of virtual memory to a page of physical memory.
 **/

struct STBEntry {
  u64 virt;		/**< Virtual address of page*/
  u64 phys;		/**< Physical address of page*/
  int asn;		/**< Address Space Number*/
  u8 gh;		/**< Granularity Hint*/
  bool asm_bit;		/**< Address Space Match bit*/
  bool access[2][4];	/**< Access permitted [read/write][current mode]*/
  bool fault[2];        /**< Fault on access [read/write]*/
  bool valid;		/**< Valid entry*/
};

/**
 * Translation Buffer.
 * The translation buffers are two on-chip translation units for translating virtual addresses to physical
 * memory. Each Alpha CPU has one translation buffers for instructions, and one for data.
 * For speed, most methods are implemented as inline functions. Documentation for the ITB can be found in
 * [HRM 5-6..7].
 **/

class CTranslationBuffer  
{
 public:
  virtual void RestoreState(FILE * f);
  void SaveState(FILE * f);
  int FindEntry(u64 virt);
  void InvalidateAll();
  void InvalidateAllProcess();
  void InvalidateSingle(u64 address);
  int convert_address(u64 virt, u64 * phys, u8 access, bool check, int cm, bool * asm_bit, bool cascade, bool forreal);
  void write_pte(int number, u64 value);
  void write_tag(int number, u64 value);
  CTranslationBuffer(class CAlphaCPU * c, bool ibox);
  virtual ~CTranslationBuffer();

 private:
  u64 v_mask;				/**< bitmask for the virtual part of the address. */
  u64 p_mask;				/**< bitmask for the physical part of the address. */
  bool bIBOX;
  class CAlphaCPU * cCPU;
  struct STBEntry entry[TB_ENTRIES];
  int    next_entry;
  u64	   temp_tag[2];
};

#endif // !defined(INCLUDED_TRANSLATIONBUFFER_H)

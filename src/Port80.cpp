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
 * 
 * PORT80.CPP contains the code for the emulated Port 80 devices.
 */

#include "StdAfx.h"
#include "Port80.h"
#include "System.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPort80::CPort80(CSystem * c) : CSystemComponent(c)
{
  c->RegisterMemory(this, 0, X64(00000801fc000080),1);
  p80 = 0;
}

CPort80::~CPort80()
{

}

u64 CPort80::ReadMem(int index, u64 address, int dsize)
{
  dsize;
  index;
  address;

  return p80;
}

void CPort80::WriteMem(int index, u64 address, int dsize, u64 data)
{
  dsize;
  index;
  address;

  p80 = (u8)data;
}

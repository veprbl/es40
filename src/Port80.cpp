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
 * Contains the code for the emulated Port 80 device.
 *
 * X-1.9        Camiel Vanderhoeven                             10-DEC-2007
 *      Use configurator.
 *
 * X-1.8        Camiel Vanderhoeven                             31-MAR-2007
 *      Added old changelog comments.
 *
 * X-1.7	Brian Wheeler					13-FEB-2007
 *	Formatting.
 *
 * X-1.6 	Camiel Vanderhoeven				12-FEB-2007
 *	Added comments.
 *
 * X-1.5        Camiel Vanderhoeven                             9-FEB-2007
 *      Added comments.
 *
 * X-1.4        Brian Wheeler                                   3-FEB-2007
 *      Formatting.
 *
 * X-1.3        Brian Wheeler                                   3-FEB-2007
 *      64-bit literals made compatible with Linux/GCC/glibc.
 *      
 * X-1.2        Brian Wheeler                                   3-FEB-2007
 *      Includes are now case-correct (necessary on Linux)
 *
 * X-1.1        Camiel Vanderhoeven                             19-JAN-2007
 *      Initial version in CVS.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / www.camicom.com)
 **/

#include "StdAfx.h"
#include "Port80.h"
#include "System.h"

/**
 * Constructor.
 **/

CPort80::CPort80(CConfigurator * cfg, CSystem * c) : CSystemComponent(cfg,c)
{
  c->RegisterMemory(this, 0, X64(00000801fc000080),1);
  p80 = 0;
}

/**
 * Destructor.
 **/

CPort80::~CPort80()
{

}

/**
 * Read from port 80.
 * Returns the value last written to port 80.
 **/

u64 CPort80::ReadMem(int index, u64 address, int dsize)
{
  return p80;
}

/**
 * Write to port 80.
 **/

void CPort80::WriteMem(int index, u64 address, int dsize, u64 data)
{
  p80 = (u8)data;
}

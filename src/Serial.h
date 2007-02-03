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
  * SERIAL.H contains the definitions for the emulated Serial Port devices.
  *
  **/

#if !defined(AFX_SERIAL_H__7FD919DE_84AC_4364_A682_0BE164142E9B__INCLUDED_)
#define AFX_SERIAL_H__7FD919DE_84AC_4364_A682_0BE164142E9B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SystemComponent.h"

class CSerial : public CSystemComponent  
{
public:
	void write(char * s);
	void WriteMem(int index, u64 address, int dsize, u64 data);
	u64 ReadMem(int index, u64 address, int dsize);
	CSerial(CSystem * c, int number);
	virtual ~CSerial();
	void receive(const char* data);

private:
	u8 bTHR;
	u8 bRDR;
    u8 bBRB_LSB;
    u8 bBRB_MSB;
    u8 bIER;
    u8 bIIR;
    u8 bFCR;
    u8 bLCR;
    u8 bMCR;
    u8 bLSR;
    u8 bMSR;
    u8 bSPR;
	class CTelnet * cTelnet;
	char rcvBuffer[1024];
	int rcvW;
	int rcvR;
	int iNumber;
};

#endif // !defined(AFX_SERIAL_H__7FD919DE_84AC_4364_A682_0BE164142E9B__INCLUDED_)

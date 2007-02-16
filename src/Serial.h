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
 * Contains the definitions for the emulated Serial Port devices.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / www.camicom.com)
 **/

#if !defined(INCLUDED_SERIAL_H)
#define INCLUDED_SERIAL_H

#include "SystemComponent.h"

/**
 * Emulated serial port.
 * The serial port is translated to a telnet port.
 **/

class CSerial : public CSystemComponent  
{
 public:
  void write(char * s);
  void WriteMem(int index, u64 address, int dsize, u64 data);
  u64 ReadMem(int index, u64 address, int dsize);
  CSerial(CSystem * c, u16 number);
  virtual ~CSerial();
  void receive(const char* data);
  int DoClock();

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
  int listenSocket;
  int connectSocket;
  int serial_cycles;
  char rcvBuffer[1024];
  int rcvW;
  int rcvR;
  int iNumber;
};

#endif // !defined(INCLUDED_SERIAL_H)

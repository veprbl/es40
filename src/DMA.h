/* ES40 emulator.
 * Copyright (C) 2007-2008 by the ES40 Emulator Project
 *
 * WWW    : http://sourceforge.net/projects/es40
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
 * Contains the definitions for the emulated DMA controller.
 *
 * $Id: DMA.h,v 1.1 2008/02/26 11:22:15 iamcamiel Exp $
 *
 * X-1.1        Camiel Vanderhoeven                             26-FEB-2008
 *      Created. Contains code previously found in AliM1543C.h
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#if !defined(INCLUDED_DMA_H)
#define INCLUDED_DMA_H

#include "SystemComponent.h"


/**
 * \brief Emulated DMA controller.
 **/

class CDMA : public CSystemComponent  
{
 public:
  CDMA(CConfigurator * cfg, CSystem * c);
  virtual ~CDMA();

  virtual int DoClock();
  virtual void WriteMem(int index, u64 address, int dsize, u64 data);
  virtual u64 ReadMem(int index, u64 address, int dsize);
  virtual int SaveState(FILE * f);
  virtual int RestoreState(FILE * f);

 private:

   
  /// The state structure contains all elements that need to be saved to the statefile.
  struct SDMA_state {

    /// DMA channel state
    struct SDMA_chan {
      bool a_lobyte; // address lobyte expected
      bool c_lobyte; // count lobyte expected
      u16 current;
      u16 base;
      u16 pagebase;
      u16 count;
    } channel[8];

    /// DMA controller state
    struct SDMA_ctrl {
      u8 status;
      u8 command;
      u8 writereq;
      u8 mask;
      u8 mode;
    } controller[2];
  } state;
};

#endif // !defined(INCLUDED_DMA_H)

/////////////////////////////////////////////////////////////////////////
// $Id: scancodes.h,v 1.2 2007/12/06 15:31:14 iamcamiel Exp $
/////////////////////////////////////////////////////////////////////////
//
/*  ES40 emulator.
 *
 *  This file is based upon Bochs.
 *
 *  Copyright (C) 2002  MandrakeSoft S.A.
 *
 *    MandrakeSoft S.A.
 *    43, rue d'Aboukir
 *    75002 Paris - France
 *    http://www.linux-mandrake.com/
 *    http://www.mandrakesoft.com/
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

/**
 *
 * X-1.1        Camiel Vanderhoeven                             6-DEC-2007
 *      Initial version for ES40 emulator.
 *
 **/

#include "gui.h"
#ifndef BX_SCANCODES_H
#define BX_SCANCODES_H

// Translation table of the 8042
extern unsigned char translation8042[256];

typedef struct { 
  const char *make;
  const char *brek;
} scancode;

// Scancodes table
extern scancode scancodes[BX_KEY_NBKEYS][3];

#endif
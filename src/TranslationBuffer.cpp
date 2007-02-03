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
  * TRANSLATIONBUFFER.CPP contains the code for the emulated on-cpu instruction and
  * data translation buffers.
  *
  **/

#include "StdAfx.h"
#include "TranslationBuffer.h"
#include "AlphaCPU.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTranslationBuffer::CTranslationBuffer(class CAlphaCPU * c, bool ibox)
{
	cCpu = c;
	bIBOX = ibox;
	p_mask = 0;
	v_mask = 0;
	temp_tag[0] = 0;
	temp_tag[1] = 0;

	InvalidateAll();

    if (ibox)
        printf("%%ITB-I-INIT: IBOX Page Translation Buffer for CPU %d initialized.\n",c->get_cpuid());
    else
        printf("%%DTB-I-INIT: MBOX Page Translation Buffer for CPU %d initialized.\n",c->get_cpuid());
}

CTranslationBuffer::~CTranslationBuffer()
{

}


void CTranslationBuffer::SaveState(FILE *f)
{
    fwrite(&bIBOX,1,sizeof(bool),f);
    fwrite(entry,1,TB_ENTRIES * sizeof(struct STBEntry),f);
    fwrite(&next_entry,1,sizeof(int),f);
    fwrite(temp_tag,1,2*8,f);
}

void CTranslationBuffer::RestoreState(FILE *f)
{
    fread(&bIBOX,1,sizeof(bool),f);
    fread(entry,1,TB_ENTRIES * sizeof(struct STBEntry),f);
    fread(&next_entry,1,sizeof(int),f);
    fread(temp_tag,1,2*8,f);
}

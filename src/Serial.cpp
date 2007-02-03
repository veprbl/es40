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
  * SERIAL.CPP contains the code for the emulated Serial Port devices.
  *
  **/

#include "StdAfx.h"
#include "Serial.h"
#include "windows/telnet.h"
#include "System.h"
#include <process.h>
#include "AliM1543C.h"

extern CAliM1543C * ali;

CRITICAL_SECTION critSection;

bool bStopping = false;
int  iCounter  = 0;
bool bStop     = false;

#define FIFO_SIZE 1024

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerial::CSerial(CSystem * c, int number) : CSystemComponent(c)
{
	c->RegisterMemory (this, 0, X64(00000801fc0003f8) - (0x100*number), 8);
	InitializeCriticalSection( &critSection );
	cTelnet = new CTelnet(8000+number, this);
	iNumber = number;

	rcvW = 0;
	rcvR = 0;

		// start the server
	if ( !cTelnet->start() )
		return;

	// start the client
	char s[100];
	sprintf(s,"telnet://localhost:%d/",8000+number);
	_spawnl(P_NOWAIT,"putty.exe","putty.exe",s,NULL);

	// wait until connection
	for (;!cTelnet->getLoggedOn();) ;

    printf("%%SRL-I-INIT: Serial Interface %d emulator initialized.\n",number);
    printf("%%SRL-I-ADDRESS: Serial Interface %d on telnet port %d.\n",number,number+8000);

	sprintf(s,"This is serial port #%d on AlphaSim\r\n",number);
	cTelnet->write(s);

    bLCR = 0x00;
	bLSR = 0x60; // THRE, TSRE
	bMSR = 0x30; // CTS, DSR
    bIIR = 0x01; // no interrupt
}

CSerial::~CSerial()
{
	cTelnet->stop();
	free(cTelnet);

}

u64 CSerial::ReadMem(int index, u64 address, int dsize)
{
    dsize;
    index;
    char trcbuffer[1000];

	u8 d;

	switch (address)
	{
	case 0:						// data buffer
        if (bLCR & 0x80)
        {
            return bBRB_LSB;
        }
        else
        {
		    if (rcvR != rcvW)
		    {   
			    bRDR = rcvBuffer[rcvR];
			    rcvR++;
			    if (rcvR == FIFO_SIZE)
				    rcvR = 0;
	    	    sprintf(trcbuffer,"Read character %02x (%c) on serial port %d\n",bRDR,bRDR,iNumber);
		        cSystem->trace->trace_dev(trcbuffer);
	    	}
            else
            {
	    	    sprintf(trcbuffer,"Read past FIFO on serial port %d\n",iNumber);
		        cSystem->trace->trace_dev(trcbuffer);
            }
            return bRDR;
        }
    case 1:
        if (bLCR & 0x80)
        {
            return bBRB_MSB;
        }
        else
        {
            return bIER;
        }
	case 2:						//interrupt cause
        d = bIIR;
        bIIR = 0x01;
        return d;
    case 3:
        return bLCR;
    case 4:
        return bMCR;
	case 5:						//serialization state
		if (rcvR != rcvW)
			bLSR = 0x61; // THRE, TSRE, RxRD
		else
			bLSR = 0x60; // THRE, TSRE
        return bLSR;
    case 6:
        return bMSR;
    default:
        return bSPR;
	}
}

void CSerial::WriteMem(int index, u64 address, int dsize, u64 data)
{
    dsize;
    index;

	u8 d;
	char s[5];
	d = (u8)data;
	char trcbuffer[500];

	switch (address)
	{
	case 0:						// data buffer
        if (bLCR & 0x80)
        {
            bBRB_LSB = d;
        }
        else
        {
		    sprintf(s,"%c",d);
		    cTelnet->write(s);
		    sprintf(trcbuffer,"Write character %02x (%c) on serial port %d\n",d,d,iNumber);
		    cSystem->trace->trace_dev(trcbuffer);
            if (bIER & 0x2)
            {
                bIIR = (bIIR>0x02)?bIIR:0x02;
        			ali->pic_interrupt(0, 4 - iNumber);
            }
        }
        break;
    case 1:
        if (bLCR & 0x80)
        {
            bBRB_MSB = d;
        }
        else
        {
            bIER = d;
            bIIR = 0x01;
            if (bIER & 0x2)
            {
                bIIR = (bIIR>0x02)?bIIR:0x02;
        			ali->pic_interrupt(0, 4 - iNumber);
            }
        }
        break;
	case 2:			
        bFCR = d;
        break;
    case 3:
        bLCR = d;
        break;
    case 4:
        bMCR = d;
        break;
    default:
        bSPR = d;
	}
}

void CSerial::write(char *s)
{
	cTelnet->write(s);
}

void CSerial::receive(const char* data)
{
	char * x;

	x = (char *) data;

	while (*x)
	{
//		if (	(rcvW==rcvR-1)					// overflow...
//			||  ((rcvR==0) && (rcvW==3)))
//			break;
		rcvBuffer[rcvW++] = *x;
		if (rcvW == FIFO_SIZE)
			rcvW = 0;
		x++;
        if (bIER & 0x1)
        {
            bIIR = (bIIR>0x04)?bIIR:0x04;
			    ali->pic_interrupt(0, 4 - iNumber);
        }
	}
}

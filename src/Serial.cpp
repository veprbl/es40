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
 * Contains the code for the emulated Serial Port devices.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / www.camicom.com)
 **/

#include "StdAfx.h"
#include "Serial.h"
#include "System.h"
#include "AliM1543C.h"

#include <stdlib.h>
#include <ctype.h>

#if defined(_WIN32)
#include <winsock.h>
#define ssize_t size_t
#endif

#if defined(__VMS)
#include <socket.h>
#include <in.h>
#include <inet.h>
#define INVALID_SOCKET -1
#endif

#if defined(_WIN32) || defined(__VMS)
#define socklen_t int


#define	IAC	255		/* interpret as command: */
#define	DONT	254		/* you are not to use option */
#define	DO	253		/* please, you use option */
#define	WONT	252		/* I won't use option */
#define	WILL	251		/* I will use option */
#define	SB	250		/* interpret as subnegotiation */
#define	GA	249		/* you may reverse the line */
#define	EL	248		/* erase the current line */
#define	EC	247		/* erase the current character */
#define	AYT	246		/* are you there */
#define	AO	245		/* abort output--but let prog finish */
#define	IP	244		/* interrupt process--permanently */
#define	BREAK	243		/* break */
#define	DM	242		/* data mark--for connect. cleaning */
#define	NOP	241		/* nop */
#define	SE	240		/* end sub negotiation */
#define EOR     239             /* end of record (transparent mode) */
#define	ABORT	238		/* Abort process */
#define	SUSP	237		/* Suspend process */
#define	xEOF	236		/* End of file: EOF is already used... */

#define SYNCH	242		/* for telfunc calls */
#define TELOPT_ECHO	1	/* echo */
#define	TELOPT_SGA	3	/* suppress go ahead */
#define	TELOPT_NAWS	31	/* window size */
#define	TELOPT_LFLOW	33	/* remote flow control */

#else // defined(_WIN32) || defined(__VMS)

#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#define INVALID_SOCKET 1

#endif // defined (_WIN32) || defined(__VMS)

#define RECV_TICKS 10

extern CAliM1543C * ali;


bool bStopping = false;
int  iCounter  = 0;
bool bStop     = false;

#define FIFO_SIZE 1024

/**
 * Constructor.
 **/

CSerial::CSerial(CSystem * c, u16 number) : CSystemComponent(c)
{
  u16 base = (u16)atoi(c->GetConfig("serial.base","8000"));
  char s[100];
  
  c->RegisterMemory (this, 0, X64(00000801fc0003f8) - (0x100*number), 8);

// Start Telnet server

  c->RegisterClock(this, true);

#if defined(_WIN32)
  // Windows Sockets only work after calling WSAStartup.
  WSADATA wsa;
  WSAStartup(0x0101, &wsa);
#endif // defined (_WIN32)

  struct sockaddr_in Address;
  socklen_t nAddressSize=sizeof(struct sockaddr_in);

  listenSocket = socket(AF_INET,SOCK_STREAM,0);
  if (listenSocket == INVALID_SOCKET)
  {
    printf("Could not open socket to listen on!\n");
  }
  
  Address.sin_addr.s_addr=INADDR_ANY;
  Address.sin_port=htons((u16)(base+number));
  Address.sin_family=AF_INET;

  int optval = 1;
  setsockopt(listenSocket,SOL_SOCKET,SO_REUSEADDR, (char*)&optval,sizeof(optval));
  bind(listenSocket,(struct sockaddr *)&Address,sizeof(Address));
  listen(listenSocket,5);

  printf("%%SRL-I-WAIT: Waiting for connection on port %d.\n",number+base);


//  Wait until we have a connection

  connectSocket = INVALID_SOCKET;
  while (connectSocket == INVALID_SOCKET)
  {
    connectSocket = accept(listenSocket,(struct sockaddr*)&Address,&nAddressSize);
  }

  serial_cycles = 0;

  // Send some control characters to the telnet client to handle 
  // character-at-a-time mode.  
  char *telnet_options="%c%c%c";
  char buffer[8];

  sprintf(buffer,telnet_options,IAC,DO,TELOPT_ECHO);
  this->write(buffer);

  sprintf(buffer,telnet_options,IAC,DO,TELOPT_NAWS);
  write(buffer);

  sprintf(buffer,telnet_options,IAC,DO,TELOPT_LFLOW);
  this->write(buffer);

  sprintf(buffer,telnet_options,IAC,WILL,TELOPT_ECHO);
  this->write(buffer);

  sprintf(buffer,telnet_options,IAC,WILL,TELOPT_SGA);
  this->write(buffer);

  sprintf(s,"This is serial port #%d on AlphaSim\r\n",number);
  this->write(s);

  printf("%%SRL-I-INIT: Serial Interface %d emulator initialized.\n",number);
  printf("%%SRL-I-ADDRESS: Serial Interface %d on telnet port %d.\n",number,number+base);

  iNumber = number;

  rcvW = 0;
  rcvR = 0;

  bLCR = 0x00;
  bLSR = 0x60; // THRE, TSRE
  bMSR = 0x30; // CTS, DSR
  bIIR = 0x01; // no interrupt
}

/**
 * Destructor.
 **/

CSerial::~CSerial()
{
}

u64 CSerial::ReadMem(int index, u64 address, int dsize)
{
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
		  TRC_DEV4("Read character %02x (%c) on serial port %d\n",bRDR,printable(bRDR),iNumber);
	    }
	  else
            {
	      TRC_DEV2("Read past FIFO on serial port %d\n",iNumber);
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
  u8 d;
  char s[5];
  d = (u8)data;

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
	  write(s);
	  TRC_DEV4("Write character %02x (%c) on serial port %d\n",d,printable(d),iNumber);
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
  send(connectSocket,s,strlen(s)+1,0);
}

void CSerial::receive(const char* data)
{
  char * x;

  x = (char *) data;

	while (*x)
	{
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

int CSerial::DoClock() {
  fd_set readset;
  unsigned char buffer[FIFO_SIZE+1];
  unsigned char cbuffer[FIFO_SIZE+1];  // cooked buffer
  unsigned char *b, *c;
  ssize_t size;
  struct timeval tv;

  serial_cycles++;
  if(serial_cycles >= RECV_TICKS) {
    FD_ZERO(&readset);
    FD_SET(connectSocket,&readset);
    tv.tv_sec=0;
    tv.tv_usec=0;
    if(select(connectSocket+1,&readset,NULL,NULL,&tv) > 0) {
#if defined(_WIN32)
      // Windows Sockets has no direct equivalent of BSD's read
      size = recv(connectSocket,(char*)buffer,FIFO_SIZE, 0);
#else
      size = read(connectSocket,&buffer,FIFO_SIZE);
#endif
      buffer[size+1]=0; // force null termination.
      b=buffer;
      c=cbuffer;
      while(b - buffer < size) {
	if(*b == IAC) {
	  if(*(b+1) == IAC) { // escaped IAC.
	    b++;
	  } else if(*(b+1) >= WILL) { // will/won't/do/don't
	    b+=3;  // skip this byte, and following two. (telnet escape)
	    continue;
	  } else if(*(b+1) == SB) { // skip until IAC SE
	    b+=2; // now we're at start of subnegotiation.
	    while(*b!=IAC && *(b+1)!=SE) b++;
	    b+=2;
	    continue;
	  } else if(*(b+1) == BREAK) { // break (== halt button?)
	    b+=2;
	    write("\r\n<BREAK> received. What do you want to do?\r\n");
	    write("     0. Continue\r\n");
#if defined(IDB)
	    write("     1. End run\r\n");
#else
	    write("     1. Exit emulator gracefully\r\n");
	    write("     2. Abort emulator (no changes saved)\r\n");
#endif
	    for (;;)
	    {
	      FD_ZERO(&readset);
	      FD_SET(connectSocket,&readset);
	      tv.tv_sec = 60;
	      tv.tv_usec = 0;
	      if (select(connectSocket+1,&readset,NULL,NULL,&tv) <= 0) 
	      {
		write("%SRL-I-TIMEOUT: no timely answer received. Continuing emulation.\r\n");
		return 0;
	      }
#if defined(_WIN32)
	      size = recv(connectSocket,(char*)buffer,FIFO_SIZE, 0);
#else
              size = read(connectSocket,&buffer,FIFO_SIZE);
#endif
 	      switch (buffer[0])
	      {
	      case '0':
	        write("%SRL-I-CONTINUE: continuing emulation.\r\n");
	        return 0;
	      case '1':
	        write("%SRL-I-EXIT: exiting emulation gracefully.\r\n");
	        return 1;
	      case '2':
	        write("%SRL-I-ABORT: aborting emulation.\r\n");
	        return -1;
	      }
	      write("%SRL-W-INVALID: Not a valid answer.\r\n");
	    }
	  } else if(*(b+1) == AYT) { // are you there?
	    
	  } else { // misc single byte command.
	    b+=2;
	    continue;
	  }
	}
	*c=*b;
	c++; b++;
      }
      *c=0; // null terminate it.
      this->receive((const char*)&cbuffer);
    }
    serial_cycles=0;
  }
  return 0;
}

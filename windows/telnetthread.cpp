///////////////////////////////////////////////////////////////////////
// Filename: telnetthread.cpp
//
// Date    : 27-May-2001
// Author  : J.Hogendoorn ( jeroenhog@gmx.net )
//
// Note    : this code may be used anywhere as long
//         : as this comment remains. 
//
// Modified: november 2006, by Camiel Vanderhoeven (camiel@camicom.com)
//           Simplified this file to suit the needs of the ES40 emulator.
//
///////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "telnet.h"

//////////////////////////////////////////////////////////////////////
// Function: TelnetThread
// In      : pointer to CTelnet instance
// Out     : none
// Purpose : uses CTelnet and CTelnetSocket to implement a telnet service
// Note    : runs as a seperate thread
void TelnetThread(void* telnet)
{
	// get pointer to telnet class
	CTelnet* tel = (CTelnet*)telnet;

	// if initialize fails, we'd better stop
	if ( !tel->getTelnetSocket()->initialize( tel->getPortNumber() ) )
	{
		// signal event ( makes main thread run )
		SetEvent( tel->getThreadStartupFailedEvent() );
		return;
	}

	// signal event ( makes main thread run )
	SetEvent( tel->getThreadStartupSucceededEvent() );

	// loop forever
	while(1)
	{
		// we are not logged on yet
		tel->setNotLoggedOn();

		if ( !tel->getTelnetSocket()->waitForConnection() )
		{
			continue;
		}

		////////////////////////////////////////////////////////////////////////////////
		// login sequence 
		////////////////////////////////////////////////////////////////////////////////

		// we got a connection now, send the intro screen
		//if ( !tel->getTelnetSocket()->sendData( "" ) )
		//{
//			tel->getTelnetSocket()->closeClientSocket();
//			continue;
//		}

		// logged on now
		tel->setLoggedOn();
		char sBuffer[4];
		
		///////////////////////////////////////////////////////////////////////////
		// handle commands
		///////////////////////////////////////////////////////////////////////////
		while(1)
		{
			memset(sBuffer,0,4);
			if ( !tel->getTelnetSocket()->waitForData (sBuffer))
			{
				tel->getTelnetSocket()->closeClientSocket();
				continue;
			}
			else if (sBuffer[0])
				tel->processData(sBuffer);
		}
	}
};


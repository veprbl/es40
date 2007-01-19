///////////////////////////////////////////////////////////////////////
// Filename: telnet.cpp
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
#include "../serial.h"

//////////////////////////////////////////////////////////////////////
// Method  : constructor
// In      : basic values needed for telnet service
// Out     : none
// Purpose : 
// Note    : when userid or password are not filled in, no check will
//           be performed for them
CTelnet::CTelnet( DWORD          dwPortNumber, CSerial * serial )
{
	m_dwPortNumber            = dwPortNumber;
	m_hThread                 = NULL;
	m_bIsRunning              = false;
	m_telnetSock              = new CTelnetSocket();

	cSerial = serial;

	// events needed for checking if the start of the telnetThread succeeds
	m_threadStartupSuccceeded = CreateEvent( NULL , TRUE , FALSE , "threadStartUpSuccceeded" );
	m_threadStartupFailed     = CreateEvent( NULL , TRUE , FALSE , "threadStartUpFailed" );

	// critical section is needed, because some attributes of this class are
	// shared between telnetThread and the main thread
	InitializeCriticalSection( &m_critSection );

	// not logged on yet
	setNotLoggedOn();

}

//////////////////////////////////////////////////////////////////////
// Method  : destructor
// In      : none
// Out     : none
// Purpose : stops the telnet service
// Note    : 
CTelnet::~CTelnet()
{
	cleanUp();

	delete m_telnetSock;
}

//////////////////////////////////////////////////////////////////////
// Method  : setLoggedOn
// In      : none
// Out     : none
// Purpose : sets the indication that a cliennt is logged on
// Note    : 
void CTelnet::setLoggedOn()
{
	EnterCriticalSection( &m_critSection );
	m_bLoggedOn = true;
	LeaveCriticalSection( &m_critSection );
}

//////////////////////////////////////////////////////////////////////
// Method  : setNotLoggedOn
// In      : none
// Out     : none
// Purpose : sets the indication that a cliennt is Not logged on
// Note    : 
void CTelnet::setNotLoggedOn()
{
	EnterCriticalSection( &m_critSection );
	m_bLoggedOn = false;
	LeaveCriticalSection( &m_critSection );
}

//////////////////////////////////////////////////////////////////////
// Method  : getLoggedOn
// In      : none
// Out     : true if logged on, false if not
// Purpose : 
// Note    : 
bool CTelnet::getLoggedOn()
{
	bool bRetval;

	EnterCriticalSection( &m_critSection );
	bRetval = m_bLoggedOn;
	LeaveCriticalSection( &m_critSection );

	return bRetval;
}

//////////////////////////////////////////////////////////////////////
// Method  : write
// In      : data to write
// Out     : true if succeeded, else if not
// Purpose : 
// Note    : client must be logged on before data can be send to the client
//           call getLastError to find out what went wrong
bool CTelnet::write( const CString& sData )
{
	// no critical sections are needed here Winsock is threadSafe
	if ( getLoggedOn() )
	{
		if ( m_bIsRunning )
		{
			if ( !m_telnetSock->sendData( sData ) )
				return false;
		}
		else
			return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Method  : start
// In      : none
// Out     : true if succeeded, else if not
// Purpose : basically starts up the telnetThread
// Note    : 
bool CTelnet::start()
{
	ResetEvent( m_threadStartupSuccceeded );
	ResetEvent( m_threadStartupFailed );

	// start up the telnetThread
	m_hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)TelnetThread, this , NULL, NULL);
	if ( m_hThread == NULL )
		return false;
	else
	{
		// wait for signal from the telnetWorkerThread
		HANDLE hArray[2] = {	m_threadStartupSuccceeded , m_threadStartupFailed };
		DWORD dwResult = WaitForMultipleObjects( 2 , hArray , FALSE , INFINITE );

		if ( dwResult == WAIT_OBJECT_0 )
			m_bIsRunning = true;
		else
			return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Method  : stop
// In      : none
// Out     : none
// Purpose : basically stops the telnetThread
// Note    :
void CTelnet::stop()
{
	cleanUp();
}

//////////////////////////////////////////////////////////////////////
// Method  : cleanUp
// In      : none
// Out     : none
// Purpose : basically stops the telnetThread, and closes the open sockets
// Note    :
void CTelnet::cleanUp()
{
	if ( m_bIsRunning )
	{

		// close client and server socket
		m_telnetSock->closeClientSocket();
		m_telnetSock->closeServerSocket();

		// brutely kill the thread
		if ( m_hThread )
		{
			DWORD dummy = 0;
			TerminateThread( m_hThread , dummy );
		}
	}
}


void CTelnet::processData(const char * sData)
{
	cSerial->receive(sData);
}
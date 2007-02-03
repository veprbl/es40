///////////////////////////////////////////////////////////////////////
// Filename: telnetsocket.cpp
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
#include "telnetsocket.h"

//////////////////////////////////////////////////////////////////////
// Method  : constructor
// In      : none
// Out     : none
// Purpose : initializes some member vars
// Note    : 
CTelnetSocket::CTelnetSocket()
{
	m_iSock       = 0;
	m_iClientSock = 0;
	m_sClientIp   = "";
}

//////////////////////////////////////////////////////////////////////
// Method  : destructor
// In      : none
// Out     : none
// Purpose : cleans up windows socket stuff
// Note    : 
CTelnetSocket::~CTelnetSocket()
{
	// cleanup windows sockets
	WSACleanup();  
}

//////////////////////////////////////////////////////////////////////
// Method  : closeClientSocket
// In      : none
// Out     : none
// Purpose : closes the client socket
// Note    : 
void CTelnetSocket::closeClientSocket()
{
	closesocket(m_iClientSock);
}

//////////////////////////////////////////////////////////////////////
// Method  : closeServerSocket
// In      : none
// Out     : none
// Purpose : closes the server socket
// Note    : 
void CTelnetSocket::closeServerSocket()
{
	closesocket(m_iSock);
}

//////////////////////////////////////////////////////////////////////
// Method  : initialize
// In      : port number to run on
// Out     : true if succeeded, false if not
// Purpose : initializes the connection
// Note    : 
bool CTelnetSocket::initialize( DWORD dwPortNr )
{
	// Windows socket initialization
	WORD    wVersionRequested; 
	WSADATA wsaData; 
	wVersionRequested = MAKEWORD(1, 1); 

	// Find winsock version
	if ( WSAStartup(wVersionRequested, &wsaData) )
		return false;

	// create socket
	m_iSock = (int)socket(AF_INET, SOCK_STREAM, 0);
	if ( m_iSock == INVALID_SOCKET  )
		return false;

	int iOptval = 1;
	if ( setsockopt( m_iSock, SOL_SOCKET, SO_REUSEADDR, (char *) &iOptval, sizeof(int)) )
		return false;

	m_siUs.sin_family      = AF_INET;
	m_siUs.sin_port        = htons( USHORT(dwPortNr) );
	m_siUs.sin_addr.s_addr = INADDR_ANY;

	// Bind to the given port
	if ( bind( m_iSock, (struct sockaddr *) &m_siUs, sizeof(m_siUs) ) )
		return false;

	// change to passive socket
	if ( listen( m_iSock , SOMAXCONN ) )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
// Method  : waitForConnection
// In      : none
// Out     : true if succeeded, false if not
// Purpose : waits for a client to connect
// Note    : 
bool CTelnetSocket::waitForConnection()
{
	// wait for port
	int         iLen        = sizeof( sockaddr_in );
	int         iClientSock = 0;

	// fill with zero terms
	memset( &m_siThem , 0 , sizeof( m_siThem) );

	// wait for connetion
	m_iClientSock = (int)accept( m_iSock, (struct sockaddr *)&(m_siThem) , &iLen );
	if ( m_iClientSock == INVALID_SOCKET )
		return false;

	// get client ip address
	char szClientIp[255];
	strncpy( szClientIp, inet_ntoa(m_siThem.sin_addr), 128);
	m_sClientIp = szClientIp;

	return true;
}

bool CTelnetSocket::sendData( const CString& sData )
{
	if ( send( m_iClientSock , sData , sData.GetLength() , 0 ) != sData.GetLength() )
		return false;

	return true;
}

bool CTelnetSocket::waitForData( char * sData )
{
	// receive max 3 bytes ( e.g. arrow up will be chr(27) + [ + A )
	int ierr = recv( m_iClientSock, sData , 3 , 0);
	if (ierr < 1 ) 
		return false;
	return true;
}

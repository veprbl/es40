///////////////////////////////////////////////////////////////////////
// Filename: telnetsocket.h
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

#ifndef _TELNETSOCKET_H_
#define _TELNETSOCKET_H_

class CTelnetSocket  
{
public:
	CTelnetSocket();
	virtual ~CTelnetSocket();

	bool    initialize( DWORD dwPortNr );
	bool    waitForConnection();
	CString getClientIp() const;
	bool    sendData( const CString& sData );
	bool    waitForData( char * sData );
	void    closeClientSocket();
	void    closeServerSocket();
private:
	int         m_iSock;
	int 	    m_iClientSock;
	CString     m_sClientIp;
	sockaddr_in m_siUs;
	sockaddr_in m_siThem;
};
//-------------------------------------------------------------------------
inline CString CTelnetSocket::getClientIp() const
{
	return m_sClientIp;
};
//-------------------------------------------------------------------------
#endif 

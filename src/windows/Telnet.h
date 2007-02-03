///////////////////////////////////////////////////////////////////////
// Filename: telnet.h
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

#ifndef _TELNET_H_
#define _TELNET_H_

#include "telnetsocket.h"

void TelnetThread(void* telnet);

class CTelnet  
{

public:
	CTelnet(  DWORD          dwPortNumber,
			  class CSerial * serial
		   );

	virtual ~CTelnet();

	bool           start();
	void           stop();
	bool           write( const CString& sData );
	HANDLE		   getThreadStartupSucceededEvent() const;
	HANDLE         getThreadStartupFailedEvent() const;
	DWORD          getPortNumber() const;
	CTelnetSocket* getTelnetSocket() const;
	void           setLoggedOn();
	void           setNotLoggedOn();
	bool           getLoggedOn() ;
	void		   processData(const char * sData);

private:
	class CSerial * cSerial;
	void             cleanUp();
	bool             m_bIsRunning;
	bool             m_bLoggedOn;
	DWORD            m_dwPortNumber;
//	CString          m_sCommand;
	HANDLE           m_hThread;
	CTelnetSocket*   m_telnetSock;
	HANDLE           m_threadStartupSuccceeded;
	HANDLE           m_threadStartupFailed;
	CRITICAL_SECTION m_critSection;
};
//-------------------------------------------------------------------------
inline HANDLE CTelnet::getThreadStartupSucceededEvent() const
{
	return m_threadStartupSuccceeded;
}
//-------------------------------------------------------------------------
inline HANDLE CTelnet::getThreadStartupFailedEvent() const
{
	return m_threadStartupFailed;
}
//-------------------------------------------------------------------------
inline CTelnetSocket* CTelnet::getTelnetSocket() const
{
	return m_telnetSock;
}
//-------------------------------------------------------------------------
inline DWORD CTelnet::getPortNumber() const
{
	return m_dwPortNumber;
}
//-------------------------------------------------------------------------

#endif 

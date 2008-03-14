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
 * Contains the definitions for the different exceptions used.
 *
 * $Id: Exception.h,v 1.1 2008/03/14 14:50:24 iamcamiel Exp $
 *
 * X-1.1        Camiel Vanderhoeven                             14-MAR-2007
 *      File created.
 **/

POCO_DECLARE_EXCEPTION(,CConfigurationException,Poco::Exception);
POCO_DECLARE_EXCEPTION(,CThreadException,Poco::Exception);
POCO_DECLARE_EXCEPTION(,CWin32Exception,Poco::Exception);
POCO_DECLARE_EXCEPTION(,CSDLException,Poco::Exception);
POCO_DECLARE_EXCEPTION(,CGracefulException,Poco::Exception);  /* User request to exit */
POCO_DECLARE_EXCEPTION(,CAbortException,Poco::Exception);     /* User request to abort */

#define CInvalidArgumentException Poco::InvalidArgumentException
#define CFileNotFoundException Poco::FileNotFoundException
#define CTimeoutException Poco::TimeoutException
#define CNotImplementedException Poco::NotImplementedException
#define CIllegalStateException Poco::IllegalStateException
#define CLogicException Poco::LogicException
#define CRuntimeException Poco::RuntimeException
#define COutOfMemoryException Poco::OutOfMemoryException
# Microsoft Developer Studio Project File - Name="es40" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=es40 - Win32 Debug

!MESSAGE ES40 emulator.
!MESSAGE Copyright (C) 2007 by Camiel Vanderhoeven
!MESSAGE
!MESSAGE Website: www.camicom.com
!MESSAGE E-mail : camiel@camicom.com
!MESSAGE 
!MESSAGE This program is free software; you can redistribute it and/or
!MESSAGE modify it under the terms of the GNU General Public License
!MESSAGE as published by the Free Software Foundation; either version 2
!MESSAGE of the License, or (at your option) any later version.
!MESSAGE 
!MESSAGE This program is distributed in the hope that it will be useful,
!MESSAGE but WITHOUT ANY WARRANTY; without even the implied warranty of
!MESSAGE MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!MESSAGE GNU General Public License for more details.
!MESSAGE 
!MESSAGE You should have received a copy of the GNU General Public License
!MESSAGE along with this program; if not, write to the Free Software
!MESSAGE Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
!MESSAGE 
!MESSAGE Although this is not required, the author would appreciate being notified of, 
!MESSAGE and receiving any modifications you may make to the source code that might serve
!MESSAGE the general public.

!MESSAGE DSP file to build ES40 with Visual Studio 6.0.
!MESSAGE
!MESSAGE X-1.1		Camiel Vanderhoeven				11-APR-2007
!MESSAGE   	File created.
!MESSAGE
!MESSAGE \author Camiel Vanderhoeven

!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "es40.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "es40.mak" CFG="es40 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "es40 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release IDB" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release LSS" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release LSM" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "es40 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40.exe"

!ELSEIF  "$(CFG)" == "es40 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x413 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "es40 - Win32 Release IDB"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release IDB"
# PROP BASE Intermediate_Dir "Release IDB"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release IDB"
# PROP Intermediate_Dir "Release IDB"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ot /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_idb.exe"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release LSS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release LSS"
# PROP BASE Intermediate_Dir "Release LSS"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release LSS"
# PROP Intermediate_Dir "Release LSS"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Ot /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ot /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /D "LS_SLAVE" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_idb.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_lss.exe"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release LSM"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release LSM"
# PROP BASE Intermediate_Dir "Release LSM"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release LSM"
# PROP Intermediate_Dir "Release LSM"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Ot /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ot /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /D "LS_MASTER" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_idb.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_lsm.exe"

!ENDIF 

# Begin Target

# Name "es40 - Win32 Release"
# Name "es40 - Win32 Debug"
# Name "es40 - Win32 Release IDB"
# Name "es40 - Win32 Release LSS"
# Name "es40 - Win32 Release LSM"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\AliM1543C.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AlphaCPU.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AlphaSim.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DPR.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Flash.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FloppyController.cpp
# End Source File
# Begin Source File

SOURCE=.\src\lockstep.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Port80.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Serial.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\src\System.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SystemComponent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TraceEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TranslationBuffer.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\AliM1543C.h
# End Source File
# Begin Source File

SOURCE=.\src\AlphaCPU.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_arith.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_bwx.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_control.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_debug.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_fp_branch.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_fp_memory.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_fp_operate.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_logical.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_memory.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_misc.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_mvi.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_pal.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_srm.h
# End Source File
# Begin Source File

SOURCE=.\src\cpu_vax.h
# End Source File
# Begin Source File

SOURCE=.\src\datatypes.h
# End Source File
# Begin Source File

SOURCE=.\src\DPR.h
# End Source File
# Begin Source File

SOURCE=.\src\es40_endian.h
# End Source File
# Begin Source File

SOURCE=.\src\es40_float.h
# End Source File
# Begin Source File

SOURCE=.\src\Flash.h
# End Source File
# Begin Source File

SOURCE=.\src\FloppyController.h
# End Source File
# Begin Source File

SOURCE=.\src\lockstep.h
# End Source File
# Begin Source File

SOURCE=.\src\Port80.h
# End Source File
# Begin Source File

SOURCE=.\src\Serial.h
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\src\System.h
# End Source File
# Begin Source File

SOURCE=.\src\SystemComponent.h
# End Source File
# Begin Source File

SOURCE=.\src\telnet.h
# End Source File
# Begin Source File

SOURCE=.\src\TraceEngine.h
# End Source File
# Begin Source File

SOURCE=.\src\TranslationBuffer.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project

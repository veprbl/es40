# Microsoft Developer Studio Project File - Name="es40" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=es40 - Win32 Debug
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
!MESSAGE "es40 - Win32 Release NN" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release NN IDB" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release NN LSS" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release NN LSM" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release NN NS" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release NN NS IDB" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release NN NS LSS" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release NN NS LSM" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release NS" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release NS IDB" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release NS LSS" (based on "Win32 (x86) Console Application")
!MESSAGE "es40 - Win32 Release NS LSM" (based on "Win32 (x86) Console Application")
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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "c:\program files\winpcap\include" /I "c:\program files\sdl\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "HAVE_PCAP" /D "HAVE_SDL" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib wpcap.lib sdl.lib sdlmain.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40.exe" /libpath:"c:\program files\sdl\lib" /libpath:"c:\program files\winpcap\lib"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "c:\program files\winpcap\include" /I "c:\program files\sdl\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "HAVE_PCAP" /D "HAVE_SDL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x413 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib wpcap.lib sdl.lib sdlmain.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"c:\program files\sdl\lib" /libpath:"c:\program files\winpcap\lib"

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
# ADD CPP /nologo /MT /W3 /GX /Ot /I "c:\program files\winpcap\include" /I "c:\program files\sdl\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /D "HAVE_PCAP" /D "HAVE_SDL" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib wpcap.lib sdl.lib sdlmain.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_idb.exe" /libpath:"c:\program files\sdl\lib" /libpath:"c:\program files\winpcap\lib"

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
# ADD CPP /nologo /MT /W3 /GX /Ot /I "c:\program files\winpcap\include" /I "c:\program files\sdl\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /D "LS_SLAVE" /D "HAVE_PCAP" /D "HAVE_SDL" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_idb.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib wpcap.lib sdl.lib sdlmain.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_lss.exe" /libpath:"c:\program files\sdl\lib" /libpath:"c:\program files\winpcap\lib"

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
# ADD CPP /nologo /MT /W3 /GX /Ot /I "c:\program files\winpcap\include" /I "c:\program files\sdl\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /D "LS_MASTER" /D "HAVE_PCAP" /D "HAVE_SDL" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_idb.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib wpcap.lib sdl.lib sdlmain.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_lsm.exe" /libpath:"c:\program files\sdl\lib" /libpath:"c:\program files\winpcap\lib"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release NN"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release NN"
# PROP BASE Intermediate_Dir "Release NN"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release NN"
# PROP Intermediate_Dir "Release NN"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "c:\program files\sdl\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "HAVE_SDL" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib sdl.lib sdlmain.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_nn.exe" /libpath:"c:\program files\sdl\lib"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release NN IDB"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release NN IDB"
# PROP BASE Intermediate_Dir "Release NN IDB"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release NN IDB"
# PROP Intermediate_Dir "Release NN IDB"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ot /I "c:\program files\sdl\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /D "HAVE_SDL" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib sdl.lib sdlmain.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_nn_idb.exe" /libpath:"c:\program files\sdl\lib"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release NN LSS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release NN LSS"
# PROP BASE Intermediate_Dir "Release NN LSS"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release NN LSS"
# PROP Intermediate_Dir "Release NN LSS"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Ot /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ot /I "c:\program files\sdl\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /D "LS_SLAVE" /D "HAVE_SDL" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_idb.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib sdl.lib sdlmain.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_nn_lss.exe" /libpath:"c:\program files\sdl\lib"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release NN LSM"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release NN LSM"
# PROP BASE Intermediate_Dir "Release NN LSM"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release NN LSM"
# PROP Intermediate_Dir "Release NN LSM"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Ot /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ot /I "c:\program files\sdl\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /D "LS_MASTER" /D "HAVE_SDL" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_idb.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib sdl.lib sdlmain.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_nn_lsm.exe" /libpath:"c:\program files\sdl\lib"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release NN NS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release NN NS"
# PROP BASE Intermediate_Dir "Release NN NS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release NN NS"
# PROP Intermediate_Dir "Release NN NS"
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_nn_ns.exe"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release NN NS IDB"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release NN NS IDB"
# PROP BASE Intermediate_Dir "Release NN NS IDB"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release NN NS IDB"
# PROP Intermediate_Dir "Release NN NS IDB"
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_nn_ns_idb.exe"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release NN NS LSS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release NN NS LSS"
# PROP BASE Intermediate_Dir "Release NN NS LSS"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release NN NS LSS"
# PROP Intermediate_Dir "Release NN NS LSS"
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_nn_ns_lss.exe"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release NN NS LSM"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release NN NS LSM"
# PROP BASE Intermediate_Dir "Release NN NS LSM"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release NN NS LSM"
# PROP Intermediate_Dir "Release NN NS LSM"
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
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_nn_ns_lsm.exe"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release NS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release NS"
# PROP BASE Intermediate_Dir "Release NS"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release NS"
# PROP Intermediate_Dir "Release NS"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "c:\program files\winpcap\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "HAVE_PCAP" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib wpcap.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_ns.exe" /libpath:"c:\program files\winpcap\lib"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release NS IDB"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release NS IDB"
# PROP BASE Intermediate_Dir "Release NS IDB"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release NS IDB"
# PROP Intermediate_Dir "Release NS IDB"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ot /I "c:\program files\winpcap\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /D "HAVE_PCAP" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib wpcap.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_ns_idb.exe" /libpath:"c:\program files\winpcap\lib"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release NS LSS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release NS LSS"
# PROP BASE Intermediate_Dir "Release NS LSS"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release NS LSS"
# PROP Intermediate_Dir "Release NS LSS"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Ot /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ot /I "c:\program files\winpcap\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /D "LS_SLAVE" /D "HAVE_PCAP" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_idb.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib wpcap.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_ns_lss.exe" /libpath:"c:\program files\winpcap\lib"

!ELSEIF  "$(CFG)" == "es40 - Win32 Release NS LSM"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release NS LSM"
# PROP BASE Intermediate_Dir "Release NS LSM"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release NS LSM"
# PROP Intermediate_Dir "Release NS LSM"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Ot /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Ot /I "c:\program files\winpcap\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "IDB" /D "LS_MASTER" /D "HAVE_PCAP" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_idb.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib wpcap.lib /nologo /subsystem:console /machine:I386 /out:"bin/es40_ns_lsm.exe" /libpath:"c:\program files\winpcap\lib"

!ENDIF 

# Begin Target

# Name "es40 - Win32 Release"
# Name "es40 - Win32 Debug"
# Name "es40 - Win32 Release IDB"
# Name "es40 - Win32 Release LSS"
# Name "es40 - Win32 Release LSM"
# Name "es40 - Win32 Release NN"
# Name "es40 - Win32 Release NN IDB"
# Name "es40 - Win32 Release NN LSS"
# Name "es40 - Win32 Release NN LSM"
# Name "es40 - Win32 Release NN NS"
# Name "es40 - Win32 Release NN NS IDB"
# Name "es40 - Win32 Release NN NS LSS"
# Name "es40 - Win32 Release NN NS LSM"
# Name "es40 - Win32 Release NS"
# Name "es40 - Win32 Release NS IDB"
# Name "es40 - Win32 Release NS LSS"
# Name "es40 - Win32 Release NS LSM"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\AliM1543C.cpp
# End Source File
# Begin Source File

SOURCE=..\AliM1543C_ide.cpp
# End Source File
# Begin Source File

SOURCE=..\AliM1543C_usb.cpp
# End Source File
# Begin Source File

SOURCE=..\AlphaCPU.cpp
# End Source File
# Begin Source File

SOURCE=..\AlphaCPU_vmspal.cpp
# End Source File
# Begin Source File

SOURCE=..\AlphaSim.cpp
# End Source File
# Begin Source File

SOURCE=..\Cirrus.cpp
# End Source File
# Begin Source File

SOURCE=..\Configurator.cpp
# End Source File
# Begin Source File

SOURCE=..\DEC21143.cpp
# End Source File
# Begin Source File

SOURCE=..\Disk.cpp
# End Source File
# Begin Source File

SOURCE=..\DiskController.cpp
# End Source File
# Begin Source File

SOURCE=..\DiskFile.cpp
# End Source File
# Begin Source File

SOURCE=..\DiskRam.cpp
# End Source File
# Begin Source File

SOURCE=..\DPR.cpp
# End Source File
# Begin Source File

SOURCE=..\es40_debug.cpp
# End Source File
# Begin Source File

SOURCE=..\Flash.cpp
# End Source File
# Begin Source File

SOURCE=..\FloppyController.cpp
# End Source File
# Begin Source File

SOURCE=..\gui\gui.cpp
# End Source File
# Begin Source File

SOURCE=..\gui\keymap.cpp
# End Source File
# Begin Source File

SOURCE=..\lockstep.cpp
# End Source File
# Begin Source File

SOURCE=..\PCIDevice.cpp
# End Source File
# Begin Source File

SOURCE=..\Port80.cpp
# End Source File
# Begin Source File

SOURCE=..\S3Trio64.cpp
# End Source File
# Begin Source File

SOURCE=..\gui\scancodes.cpp
# End Source File
# Begin Source File

SOURCE=..\gui\sdl.cpp
# End Source File
# Begin Source File

SOURCE=..\Serial.cpp
# End Source File
# Begin Source File

SOURCE=..\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=..\Sym53C895.cpp
# End Source File
# Begin Source File

SOURCE=..\System.cpp
# End Source File
# Begin Source File

SOURCE=..\SystemComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\TraceEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\VGA.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\AliM1543C.h
# End Source File
# Begin Source File

SOURCE=..\AliM1543C_ide.h
# End Source File
# Begin Source File

SOURCE=..\AliM1543C_usb.h
# End Source File
# Begin Source File

SOURCE=..\AlphaCPU.h
# End Source File
# Begin Source File

SOURCE=..\Cirrus.h
# End Source File
# Begin Source File

SOURCE=..\Configurator.h
# End Source File
# Begin Source File

SOURCE=..\cpu_arith.h
# End Source File
# Begin Source File

SOURCE=..\cpu_bwx.h
# End Source File
# Begin Source File

SOURCE=..\cpu_control.h
# End Source File
# Begin Source File

SOURCE=..\cpu_debug.h
# End Source File
# Begin Source File

SOURCE=..\cpu_fp_branch.h
# End Source File
# Begin Source File

SOURCE=..\cpu_fp_memory.h
# End Source File
# Begin Source File

SOURCE=..\cpu_fp_operate.h
# End Source File
# Begin Source File

SOURCE=..\cpu_logical.h
# End Source File
# Begin Source File

SOURCE=..\cpu_memory.h
# End Source File
# Begin Source File

SOURCE=..\cpu_misc.h
# End Source File
# Begin Source File

SOURCE=..\cpu_mvi.h
# End Source File
# Begin Source File

SOURCE=..\cpu_pal.h
# End Source File
# Begin Source File

SOURCE=..\cpu_vax.h
# End Source File
# Begin Source File

SOURCE=..\datatypes.h
# End Source File
# Begin Source File

SOURCE=..\DEC21143.h
# End Source File
# Begin Source File

SOURCE=..\DEC21143_mii.h
# End Source File
# Begin Source File

SOURCE=..\DEC21143_tulipreg.h
# End Source File
# Begin Source File

SOURCE=..\Disk.h
# End Source File
# Begin Source File

SOURCE=..\DiskController.h
# End Source File
# Begin Source File

SOURCE=..\DiskFile.h
# End Source File
# Begin Source File

SOURCE=..\DiskRam.h
# End Source File
# Begin Source File

SOURCE=..\DPR.h
# End Source File
# Begin Source File

SOURCE=..\es40_debug.h
# End Source File
# Begin Source File

SOURCE=..\es40_endian.h
# End Source File
# Begin Source File

SOURCE=..\es40_float.h
# End Source File
# Begin Source File

SOURCE=..\Flash.h
# End Source File
# Begin Source File

SOURCE=..\FloppyController.h
# End Source File
# Begin Source File

SOURCE=..\gui\gui.h
# End Source File
# Begin Source File

SOURCE=..\gui\keymap.h
# End Source File
# Begin Source File

SOURCE=..\lockstep.h
# End Source File
# Begin Source File

SOURCE=..\PCIDevice.h
# End Source File
# Begin Source File

SOURCE=..\gui\plugin.h
# End Source File
# Begin Source File

SOURCE=..\Port80.h
# End Source File
# Begin Source File

SOURCE=..\S3Trio64.h
# End Source File
# Begin Source File

SOURCE=..\gui\scancodes.h
# End Source File
# Begin Source File

SOURCE=..\gui\sdl_fonts.h
# End Source File
# Begin Source File

SOURCE=..\gui\sdlkeys.h
# End Source File
# Begin Source File

SOURCE=..\Serial.h
# End Source File
# Begin Source File

SOURCE=..\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\Sym53C895.h
# End Source File
# Begin Source File

SOURCE=..\System.h
# End Source File
# Begin Source File

SOURCE=..\SystemComponent.h
# End Source File
# Begin Source File

SOURCE=..\telnet.h
# End Source File
# Begin Source File

SOURCE=..\TraceEngine.h
# End Source File
# Begin Source File

SOURCE=..\gui\vga.h
# End Source File
# Begin Source File

SOURCE=..\VGA.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project

$ SET NOVERIFY
$!
$! ES40 Emulator
$! Copyright (C) 2007-2008 by the ES40 Emulator Project
$!
$! This file was created by make_vms.sh. Please refer to that file
$! for more information.
$!
$ SAY = "WRITE SYS$OUTPUT"
$!
$! DETERMINE ES40 SRC ROOT PATH IN UNIX-STYLE SYNTAX
$!
$ DFLT = F$STRING("/" + F$ENVIRONMENT("DEFAULT"))
$ DLEN = F$LENGTH("''DFLT'")
$!
$ loop_dot:
$   DD = F$LOCATE(".",DFLT)
$   IF DD .EQ. DLEN
$   THEN
$     GOTO loop_dot_end
$   ENDIF
$   DFLT[DD,1]:="/"
$ GOTO loop_dot
$ loop_dot_end:
$!
$ DD = F$LOCATE(":[",DFLT)
$ IF DD .NE. DLEN
$ THEN
$   DFLT[DD,2]:="/"
$ ENDIF
$!
$ DD = F$LOCATE("]",DFLT)
$ IF DD .NE. DLEN
$ THEN
$   DFLT[DD,1]:=""
$ ENDIF
$!
$ DD = F$LOCATE("$",DFLT)
$ IF DD .NE. DLEN
$ THEN
$   DFLT=F$STRING(F$EXTRACT(0,DD,DFLT) + "\$" + F$EXTRACT(DD+1,DLEN-DD,DFLT))
$ ENDIF
$!
$ ES40_ROOT = F$EDIT(DFLT,"COLLAPSE")
$!
$! Determine if X11 support is available...
$!
$ CREATE X11TEST.CPP
$ DECK
#include <X11/Xlib.h>

void x() { XOpenDisplay(NULL); }
$ EOD
$ SET NOON
$ CXX X11TEST.CPP /OBJECT=X11TEST.OBJ
$ IF $STATUS
$ THEN
$   SAY "Have found X11 support"
$   X11_DEF=",HAVE_X11"
$   X11_LIB=",SYS$LIBRARY:DECWINDOWS/LIB"
$ ELSE
$   SAY "Have not found X11 support"
$   X11_DEF=""
$   X11_LIB=""
$ ENDIF
$ DELETE X11TEST.CPP;
$ DELETE X11TEST.OBJ;
$ SET ON
$!
$!
$! Compile sources for es40
$!
$! Compile with the following defines: ES40,__USE_STD_IOSTREAM
$!
$ SAY "Compiling es40..."
$!
$! Check if es40_AliM1543C.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AliM1543C.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_AliM1543C.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_AliM1543C.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AliM1543C.cpp to es40_AliM1543C.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AliM1543C.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_AliM1543C.obj
$ ENDIF
$!
$! Check if es40_AliM1543C_ide.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AliM1543C_ide.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_AliM1543C_ide.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_AliM1543C_ide.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AliM1543C_ide.cpp to es40_AliM1543C_ide.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AliM1543C_ide.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_AliM1543C_ide.obj
$ ENDIF
$!
$! Check if es40_AliM1543C_usb.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AliM1543C_usb.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_AliM1543C_usb.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_AliM1543C_usb.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AliM1543C_usb.cpp to es40_AliM1543C_usb.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AliM1543C_usb.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_AliM1543C_usb.obj
$ ENDIF
$!
$! Check if es40_AlphaCPU.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_AlphaCPU.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_AlphaCPU.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU.cpp to es40_AlphaCPU.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_AlphaCPU.obj
$ ENDIF
$!
$! Check if es40_AlphaCPU_ieeefloat.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU_ieeefloat.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_AlphaCPU_ieeefloat.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_AlphaCPU_ieeefloat.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU_ieeefloat.cpp to es40_AlphaCPU_ieeefloat.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU_ieeefloat.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_AlphaCPU_ieeefloat.obj
$ ENDIF
$!
$! Check if es40_AlphaCPU_vaxfloat.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU_vaxfloat.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_AlphaCPU_vaxfloat.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_AlphaCPU_vaxfloat.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU_vaxfloat.cpp to es40_AlphaCPU_vaxfloat.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU_vaxfloat.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_AlphaCPU_vaxfloat.obj
$ ENDIF
$!
$! Check if es40_AlphaCPU_vmspal.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU_vmspal.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_AlphaCPU_vmspal.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_AlphaCPU_vmspal.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU_vmspal.cpp to es40_AlphaCPU_vmspal.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU_vmspal.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_AlphaCPU_vmspal.obj
$ ENDIF
$!
$! Check if es40_AlphaSim.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaSim.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_AlphaSim.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_AlphaSim.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaSim.cpp to es40_AlphaSim.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaSim.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_AlphaSim.obj
$ ENDIF
$!
$! Check if es40_Cirrus.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Cirrus.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_Cirrus.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_Cirrus.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Cirrus.cpp to es40_Cirrus.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Cirrus.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_Cirrus.obj
$ ENDIF
$!
$! Check if es40_Configurator.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Configurator.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_Configurator.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_Configurator.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Configurator.cpp to es40_Configurator.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Configurator.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_Configurator.obj
$ ENDIF
$!
$! Check if es40_DEC21143.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DEC21143.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_DEC21143.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_DEC21143.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DEC21143.cpp to es40_DEC21143.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DEC21143.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_DEC21143.obj
$ ENDIF
$!
$! Check if es40_Disk.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Disk.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_Disk.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_Disk.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Disk.cpp to es40_Disk.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Disk.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_Disk.obj
$ ENDIF
$!
$! Check if es40_DiskController.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskController.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_DiskController.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_DiskController.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskController.cpp to es40_DiskController.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskController.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_DiskController.obj
$ ENDIF
$!
$! Check if es40_DiskDevice.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskDevice.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_DiskDevice.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_DiskDevice.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskDevice.cpp to es40_DiskDevice.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskDevice.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_DiskDevice.obj
$ ENDIF
$!
$! Check if es40_DiskFile.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskFile.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_DiskFile.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_DiskFile.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskFile.cpp to es40_DiskFile.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskFile.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_DiskFile.obj
$ ENDIF
$!
$! Check if es40_DiskRam.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskRam.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_DiskRam.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_DiskRam.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskRam.cpp to es40_DiskRam.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskRam.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_DiskRam.obj
$ ENDIF
$!
$! Check if es40_DMA.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DMA.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_DMA.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_DMA.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DMA.cpp to es40_DMA.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DMA.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_DMA.obj
$ ENDIF
$!
$! Check if es40_DPR.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DPR.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_DPR.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_DPR.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DPR.cpp to es40_DPR.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DPR.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_DPR.obj
$ ENDIF
$!
$! Check if es40_es40_debug.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_debug.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_es40_debug.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_es40_debug.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile es40_debug.cpp to es40_es40_debug.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX es40_debug.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_es40_debug.obj
$ ENDIF
$!
$! Check if es40_Ethernet.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Ethernet.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_Ethernet.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_Ethernet.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Ethernet.cpp to es40_Ethernet.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Ethernet.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_Ethernet.obj
$ ENDIF
$!
$! Check if es40_Flash.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Flash.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_Flash.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_Flash.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Flash.cpp to es40_Flash.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Flash.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_Flash.obj
$ ENDIF
$!
$! Check if es40_FloppyController.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("FloppyController.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_FloppyController.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_FloppyController.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile FloppyController.cpp to es40_FloppyController.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX FloppyController.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_FloppyController.obj
$ ENDIF
$!
$! Check if es40_Keyboard.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Keyboard.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_Keyboard.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_Keyboard.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Keyboard.cpp to es40_Keyboard.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Keyboard.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_Keyboard.obj
$ ENDIF
$!
$! Check if es40_lockstep.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("lockstep.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lockstep.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lockstep.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile lockstep.cpp to es40_lockstep.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX lockstep.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lockstep.obj
$ ENDIF
$!
$! Check if es40_PCIDevice.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("PCIDevice.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_PCIDevice.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_PCIDevice.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile PCIDevice.cpp to es40_PCIDevice.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX PCIDevice.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_PCIDevice.obj
$ ENDIF
$!
$! Check if es40_Port80.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Port80.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_Port80.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_Port80.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Port80.cpp to es40_Port80.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Port80.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_Port80.obj
$ ENDIF
$!
$! Check if es40_S3Trio64.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("S3Trio64.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_S3Trio64.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_S3Trio64.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile S3Trio64.cpp to es40_S3Trio64.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX S3Trio64.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_S3Trio64.obj
$ ENDIF
$!
$! Check if es40_SCSIBus.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("SCSIBus.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_SCSIBus.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_SCSIBus.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile SCSIBus.cpp to es40_SCSIBus.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX SCSIBus.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_SCSIBus.obj
$ ENDIF
$!
$! Check if es40_SCSIDevice.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("SCSIDevice.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_SCSIDevice.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_SCSIDevice.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile SCSIDevice.cpp to es40_SCSIDevice.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX SCSIDevice.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_SCSIDevice.obj
$ ENDIF
$!
$! Check if es40_Serial.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Serial.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_Serial.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_Serial.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Serial.cpp to es40_Serial.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Serial.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_Serial.obj
$ ENDIF
$!
$! Check if es40_StdAfx.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("StdAfx.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_StdAfx.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_StdAfx.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile StdAfx.cpp to es40_StdAfx.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX StdAfx.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_StdAfx.obj
$ ENDIF
$!
$! Check if es40_Sym53C810.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Sym53C810.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_Sym53C810.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_Sym53C810.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Sym53C810.cpp to es40_Sym53C810.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Sym53C810.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_Sym53C810.obj
$ ENDIF
$!
$! Check if es40_Sym53C895.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Sym53C895.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_Sym53C895.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_Sym53C895.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Sym53C895.cpp to es40_Sym53C895.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Sym53C895.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_Sym53C895.obj
$ ENDIF
$!
$! Check if es40_SystemComponent.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("SystemComponent.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_SystemComponent.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_SystemComponent.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile SystemComponent.cpp to es40_SystemComponent.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX SystemComponent.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_SystemComponent.obj
$ ENDIF
$!
$! Check if es40_System.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("System.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_System.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_System.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile System.cpp to es40_System.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX System.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_System.obj
$ ENDIF
$!
$! Check if es40_TraceEngine.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("TraceEngine.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_TraceEngine.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_TraceEngine.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile TraceEngine.cpp to es40_TraceEngine.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX TraceEngine.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_TraceEngine.obj
$ ENDIF
$!
$! Check if es40_VGA.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("VGA.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_VGA.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_VGA.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile VGA.cpp to es40_VGA.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX VGA.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_VGA.obj
$ ENDIF
$!
$! Check if es40_gui_gui.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]gui.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_gui_gui.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_gui_gui.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]gui.cpp to es40_gui_gui.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]gui.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_gui_gui.obj
$ ENDIF
$!
$! Check if es40_gui_gui_x11.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]gui_x11.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_gui_gui_x11.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_gui_gui_x11.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]gui_x11.cpp to es40_gui_gui_x11.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]gui_x11.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_gui_gui_x11.obj
$ ENDIF
$!
$! Check if es40_gui_keymap.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]keymap.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_gui_keymap.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_gui_keymap.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]keymap.cpp to es40_gui_keymap.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]keymap.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_gui_keymap.obj
$ ENDIF
$!
$! Check if es40_gui_scancodes.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]scancodes.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_gui_scancodes.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_gui_scancodes.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]scancodes.cpp to es40_gui_scancodes.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]scancodes.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_gui_scancodes.obj
$ ENDIF
$!
$! Check if es40_gui_sdl.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]sdl.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_gui_sdl.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_gui_sdl.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]sdl.cpp to es40_gui_sdl.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]sdl.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_gui_sdl.obj
$ ENDIF
$!
$! Check if es40_base_Bugcheck.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Bugcheck.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_base_Bugcheck.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_base_Bugcheck.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Bugcheck.cpp to es40_base_Bugcheck.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Bugcheck.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_base_Bugcheck.obj
$ ENDIF
$!
$! Check if es40_base_ErrorHandler.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]ErrorHandler.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_base_ErrorHandler.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_base_ErrorHandler.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]ErrorHandler.cpp to es40_base_ErrorHandler.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]ErrorHandler.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_base_ErrorHandler.obj
$ ENDIF
$!
$! Check if es40_base_Event.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Event.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_base_Event.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_base_Event.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Event.cpp to es40_base_Event.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Event.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_base_Event.obj
$ ENDIF
$!
$! Check if es40_base_Exception.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Exception.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_base_Exception.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_base_Exception.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Exception.cpp to es40_base_Exception.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Exception.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_base_Exception.obj
$ ENDIF
$!
$! Check if es40_base_Mutex.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Mutex.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_base_Mutex.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_base_Mutex.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Mutex.cpp to es40_base_Mutex.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Mutex.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_base_Mutex.obj
$ ENDIF
$!
$! Check if es40_base_NumberFormatter.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]NumberFormatter.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_base_NumberFormatter.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_base_NumberFormatter.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]NumberFormatter.cpp to es40_base_NumberFormatter.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]NumberFormatter.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_base_NumberFormatter.obj
$ ENDIF
$!
$! Check if es40_base_RefCountedObject.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]RefCountedObject.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_base_RefCountedObject.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_base_RefCountedObject.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]RefCountedObject.cpp to es40_base_RefCountedObject.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]RefCountedObject.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_base_RefCountedObject.obj
$ ENDIF
$!
$! Check if es40_base_Runnable.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Runnable.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_base_Runnable.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_base_Runnable.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Runnable.cpp to es40_base_Runnable.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Runnable.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_base_Runnable.obj
$ ENDIF
$!
$! Check if es40_base_RWLock.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]RWLock.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_base_RWLock.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_base_RWLock.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]RWLock.cpp to es40_base_RWLock.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]RWLock.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_base_RWLock.obj
$ ENDIF
$!
$! Check if es40_base_Semaphore.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Semaphore.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_base_Semaphore.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_base_Semaphore.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Semaphore.cpp to es40_base_Semaphore.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Semaphore.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_base_Semaphore.obj
$ ENDIF
$!
$! Check if es40_base_SignalHandler.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]SignalHandler.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_base_SignalHandler.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_base_SignalHandler.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]SignalHandler.cpp to es40_base_SignalHandler.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]SignalHandler.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_base_SignalHandler.obj
$ ENDIF
$!
$! Check if es40_base_ThreadLocal.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]ThreadLocal.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_base_ThreadLocal.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_base_ThreadLocal.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]ThreadLocal.cpp to es40_base_ThreadLocal.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]ThreadLocal.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_base_ThreadLocal.obj
$ ENDIF
$!
$! Check if es40_base_Thread.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Thread.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_base_Thread.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_base_Thread.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Thread.cpp to es40_base_Thread.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Thread.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_base_Thread.obj
$ ENDIF
$!
$! Link es40
$!
$ SAY "Linking es40..."
$!
$ CXXLINK es40_AliM1543C.obj,es40_AliM1543C_ide.obj,es40_AliM1543C_usb.obj,es40_AlphaCPU.obj,es40_AlphaCPU_ieeefloat.obj,es40_AlphaCPU_vaxfloat.obj,es40_AlphaCPU_vmspal.obj,es40_AlphaSim.obj,es40_Cirrus.obj,es40_Configurator.obj,es40_DEC21143.obj,es40_Disk.obj,es40_DiskController.obj,es40_DiskDevice.obj,es40_DiskFile.obj,es40_DiskRam.obj,es40_DMA.obj,es40_DPR.obj,es40_es40_debug.obj,es40_Ethernet.obj,es40_Flash.obj,es40_FloppyController.obj,es40_Keyboard.obj,es40_lockstep.obj,es40_PCIDevice.obj,es40_Port80.obj,es40_S3Trio64.obj,es40_SCSIBus.obj,es40_SCSIDevice.obj,es40_Serial.obj,es40_StdAfx.obj,es40_Sym53C810.obj,es40_Sym53C895.obj,es40_SystemComponent.obj,es40_System.obj,es40_TraceEngine.obj,es40_VGA.obj,es40_gui_gui.obj,es40_gui_gui_x11.obj,es40_gui_keymap.obj,es40_gui_scancodes.obj,es40_gui_sdl.obj,es40_base_Bugcheck.obj,es40_base_ErrorHandler.obj,es40_base_Event.obj,es40_base_Exception.obj,es40_base_Mutex.obj,es40_base_NumberFormatter.obj,es40_base_RefCountedObject.obj,es40_base_Runnable.obj,es40_base_RWLock.obj,es40_base_Semaphore.obj,es40_base_SignalHandler.obj,es40_base_ThreadLocal.obj,es40_base_Thread.obj'X11_LIB' -
           /EXECUTABLE=es40.exe
$!
$! Compile sources for es40_idb
$!
$! Compile with the following defines: ES40,__USE_STD_IOSTREAM,IDB
$!
$ SAY "Compiling es40_idb..."
$!
$! Check if es40_idb_AliM1543C.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AliM1543C.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_AliM1543C.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_AliM1543C.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AliM1543C.cpp to es40_idb_AliM1543C.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AliM1543C.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_AliM1543C.obj
$ ENDIF
$!
$! Check if es40_idb_AliM1543C_ide.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AliM1543C_ide.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_AliM1543C_ide.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_AliM1543C_ide.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AliM1543C_ide.cpp to es40_idb_AliM1543C_ide.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AliM1543C_ide.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_AliM1543C_ide.obj
$ ENDIF
$!
$! Check if es40_idb_AliM1543C_usb.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AliM1543C_usb.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_AliM1543C_usb.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_AliM1543C_usb.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AliM1543C_usb.cpp to es40_idb_AliM1543C_usb.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AliM1543C_usb.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_AliM1543C_usb.obj
$ ENDIF
$!
$! Check if es40_idb_AlphaCPU.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_AlphaCPU.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_AlphaCPU.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU.cpp to es40_idb_AlphaCPU.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_AlphaCPU.obj
$ ENDIF
$!
$! Check if es40_idb_AlphaCPU_ieeefloat.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU_ieeefloat.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_AlphaCPU_ieeefloat.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_AlphaCPU_ieeefloat.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU_ieeefloat.cpp to es40_idb_AlphaCPU_ieeefloat.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU_ieeefloat.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_AlphaCPU_ieeefloat.obj
$ ENDIF
$!
$! Check if es40_idb_AlphaCPU_vaxfloat.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU_vaxfloat.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_AlphaCPU_vaxfloat.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_AlphaCPU_vaxfloat.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU_vaxfloat.cpp to es40_idb_AlphaCPU_vaxfloat.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU_vaxfloat.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_AlphaCPU_vaxfloat.obj
$ ENDIF
$!
$! Check if es40_idb_AlphaCPU_vmspal.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU_vmspal.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_AlphaCPU_vmspal.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_AlphaCPU_vmspal.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU_vmspal.cpp to es40_idb_AlphaCPU_vmspal.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU_vmspal.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_AlphaCPU_vmspal.obj
$ ENDIF
$!
$! Check if es40_idb_AlphaSim.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaSim.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_AlphaSim.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_AlphaSim.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaSim.cpp to es40_idb_AlphaSim.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaSim.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_AlphaSim.obj
$ ENDIF
$!
$! Check if es40_idb_Cirrus.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Cirrus.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_Cirrus.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_Cirrus.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Cirrus.cpp to es40_idb_Cirrus.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Cirrus.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_Cirrus.obj
$ ENDIF
$!
$! Check if es40_idb_Configurator.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Configurator.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_Configurator.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_Configurator.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Configurator.cpp to es40_idb_Configurator.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Configurator.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_Configurator.obj
$ ENDIF
$!
$! Check if es40_idb_DEC21143.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DEC21143.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_DEC21143.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_DEC21143.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DEC21143.cpp to es40_idb_DEC21143.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DEC21143.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_DEC21143.obj
$ ENDIF
$!
$! Check if es40_idb_Disk.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Disk.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_Disk.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_Disk.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Disk.cpp to es40_idb_Disk.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Disk.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_Disk.obj
$ ENDIF
$!
$! Check if es40_idb_DiskController.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskController.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_DiskController.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_DiskController.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskController.cpp to es40_idb_DiskController.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskController.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_DiskController.obj
$ ENDIF
$!
$! Check if es40_idb_DiskDevice.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskDevice.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_DiskDevice.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_DiskDevice.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskDevice.cpp to es40_idb_DiskDevice.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskDevice.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_DiskDevice.obj
$ ENDIF
$!
$! Check if es40_idb_DiskFile.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskFile.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_DiskFile.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_DiskFile.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskFile.cpp to es40_idb_DiskFile.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskFile.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_DiskFile.obj
$ ENDIF
$!
$! Check if es40_idb_DiskRam.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskRam.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_DiskRam.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_DiskRam.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskRam.cpp to es40_idb_DiskRam.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskRam.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_DiskRam.obj
$ ENDIF
$!
$! Check if es40_idb_DMA.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DMA.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_DMA.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_DMA.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DMA.cpp to es40_idb_DMA.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DMA.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_DMA.obj
$ ENDIF
$!
$! Check if es40_idb_DPR.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DPR.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_DPR.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_DPR.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DPR.cpp to es40_idb_DPR.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DPR.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_DPR.obj
$ ENDIF
$!
$! Check if es40_idb_es40_debug.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_debug.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_es40_debug.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_es40_debug.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile es40_debug.cpp to es40_idb_es40_debug.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX es40_debug.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_es40_debug.obj
$ ENDIF
$!
$! Check if es40_idb_Ethernet.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Ethernet.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_Ethernet.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_Ethernet.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Ethernet.cpp to es40_idb_Ethernet.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Ethernet.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_Ethernet.obj
$ ENDIF
$!
$! Check if es40_idb_Flash.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Flash.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_Flash.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_Flash.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Flash.cpp to es40_idb_Flash.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Flash.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_Flash.obj
$ ENDIF
$!
$! Check if es40_idb_FloppyController.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("FloppyController.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_FloppyController.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_FloppyController.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile FloppyController.cpp to es40_idb_FloppyController.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX FloppyController.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_FloppyController.obj
$ ENDIF
$!
$! Check if es40_idb_Keyboard.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Keyboard.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_Keyboard.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_Keyboard.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Keyboard.cpp to es40_idb_Keyboard.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Keyboard.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_Keyboard.obj
$ ENDIF
$!
$! Check if es40_idb_lockstep.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("lockstep.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_lockstep.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_lockstep.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile lockstep.cpp to es40_idb_lockstep.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX lockstep.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_lockstep.obj
$ ENDIF
$!
$! Check if es40_idb_PCIDevice.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("PCIDevice.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_PCIDevice.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_PCIDevice.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile PCIDevice.cpp to es40_idb_PCIDevice.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX PCIDevice.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_PCIDevice.obj
$ ENDIF
$!
$! Check if es40_idb_Port80.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Port80.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_Port80.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_Port80.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Port80.cpp to es40_idb_Port80.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Port80.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_Port80.obj
$ ENDIF
$!
$! Check if es40_idb_S3Trio64.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("S3Trio64.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_S3Trio64.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_S3Trio64.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile S3Trio64.cpp to es40_idb_S3Trio64.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX S3Trio64.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_S3Trio64.obj
$ ENDIF
$!
$! Check if es40_idb_SCSIBus.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("SCSIBus.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_SCSIBus.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_SCSIBus.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile SCSIBus.cpp to es40_idb_SCSIBus.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX SCSIBus.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_SCSIBus.obj
$ ENDIF
$!
$! Check if es40_idb_SCSIDevice.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("SCSIDevice.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_SCSIDevice.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_SCSIDevice.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile SCSIDevice.cpp to es40_idb_SCSIDevice.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX SCSIDevice.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_SCSIDevice.obj
$ ENDIF
$!
$! Check if es40_idb_Serial.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Serial.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_Serial.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_Serial.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Serial.cpp to es40_idb_Serial.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Serial.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_Serial.obj
$ ENDIF
$!
$! Check if es40_idb_StdAfx.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("StdAfx.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_StdAfx.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_StdAfx.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile StdAfx.cpp to es40_idb_StdAfx.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX StdAfx.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_StdAfx.obj
$ ENDIF
$!
$! Check if es40_idb_Sym53C810.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Sym53C810.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_Sym53C810.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_Sym53C810.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Sym53C810.cpp to es40_idb_Sym53C810.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Sym53C810.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_Sym53C810.obj
$ ENDIF
$!
$! Check if es40_idb_Sym53C895.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Sym53C895.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_Sym53C895.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_Sym53C895.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Sym53C895.cpp to es40_idb_Sym53C895.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Sym53C895.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_Sym53C895.obj
$ ENDIF
$!
$! Check if es40_idb_SystemComponent.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("SystemComponent.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_SystemComponent.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_SystemComponent.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile SystemComponent.cpp to es40_idb_SystemComponent.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX SystemComponent.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_SystemComponent.obj
$ ENDIF
$!
$! Check if es40_idb_System.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("System.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_System.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_System.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile System.cpp to es40_idb_System.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX System.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_System.obj
$ ENDIF
$!
$! Check if es40_idb_TraceEngine.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("TraceEngine.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_TraceEngine.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_TraceEngine.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile TraceEngine.cpp to es40_idb_TraceEngine.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX TraceEngine.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_TraceEngine.obj
$ ENDIF
$!
$! Check if es40_idb_VGA.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("VGA.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_VGA.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_VGA.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile VGA.cpp to es40_idb_VGA.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX VGA.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_VGA.obj
$ ENDIF
$!
$! Check if es40_idb_gui_gui.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]gui.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_gui_gui.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_gui_gui.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]gui.cpp to es40_idb_gui_gui.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]gui.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_gui_gui.obj
$ ENDIF
$!
$! Check if es40_idb_gui_gui_x11.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]gui_x11.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_gui_gui_x11.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_gui_gui_x11.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]gui_x11.cpp to es40_idb_gui_gui_x11.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]gui_x11.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_gui_gui_x11.obj
$ ENDIF
$!
$! Check if es40_idb_gui_keymap.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]keymap.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_gui_keymap.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_gui_keymap.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]keymap.cpp to es40_idb_gui_keymap.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]keymap.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_gui_keymap.obj
$ ENDIF
$!
$! Check if es40_idb_gui_scancodes.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]scancodes.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_gui_scancodes.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_gui_scancodes.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]scancodes.cpp to es40_idb_gui_scancodes.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]scancodes.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_gui_scancodes.obj
$ ENDIF
$!
$! Check if es40_idb_gui_sdl.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]sdl.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_gui_sdl.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_gui_sdl.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]sdl.cpp to es40_idb_gui_sdl.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]sdl.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_gui_sdl.obj
$ ENDIF
$!
$! Check if es40_idb_base_Bugcheck.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Bugcheck.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_base_Bugcheck.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_base_Bugcheck.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Bugcheck.cpp to es40_idb_base_Bugcheck.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Bugcheck.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_base_Bugcheck.obj
$ ENDIF
$!
$! Check if es40_idb_base_ErrorHandler.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]ErrorHandler.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_base_ErrorHandler.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_base_ErrorHandler.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]ErrorHandler.cpp to es40_idb_base_ErrorHandler.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]ErrorHandler.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_base_ErrorHandler.obj
$ ENDIF
$!
$! Check if es40_idb_base_Event.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Event.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_base_Event.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_base_Event.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Event.cpp to es40_idb_base_Event.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Event.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_base_Event.obj
$ ENDIF
$!
$! Check if es40_idb_base_Exception.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Exception.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_base_Exception.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_base_Exception.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Exception.cpp to es40_idb_base_Exception.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Exception.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_base_Exception.obj
$ ENDIF
$!
$! Check if es40_idb_base_Mutex.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Mutex.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_base_Mutex.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_base_Mutex.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Mutex.cpp to es40_idb_base_Mutex.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Mutex.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_base_Mutex.obj
$ ENDIF
$!
$! Check if es40_idb_base_NumberFormatter.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]NumberFormatter.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_base_NumberFormatter.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_base_NumberFormatter.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]NumberFormatter.cpp to es40_idb_base_NumberFormatter.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]NumberFormatter.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_base_NumberFormatter.obj
$ ENDIF
$!
$! Check if es40_idb_base_RefCountedObject.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]RefCountedObject.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_base_RefCountedObject.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_base_RefCountedObject.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]RefCountedObject.cpp to es40_idb_base_RefCountedObject.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]RefCountedObject.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_base_RefCountedObject.obj
$ ENDIF
$!
$! Check if es40_idb_base_Runnable.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Runnable.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_base_Runnable.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_base_Runnable.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Runnable.cpp to es40_idb_base_Runnable.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Runnable.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_base_Runnable.obj
$ ENDIF
$!
$! Check if es40_idb_base_RWLock.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]RWLock.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_base_RWLock.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_base_RWLock.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]RWLock.cpp to es40_idb_base_RWLock.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]RWLock.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_base_RWLock.obj
$ ENDIF
$!
$! Check if es40_idb_base_Semaphore.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Semaphore.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_base_Semaphore.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_base_Semaphore.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Semaphore.cpp to es40_idb_base_Semaphore.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Semaphore.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_base_Semaphore.obj
$ ENDIF
$!
$! Check if es40_idb_base_SignalHandler.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]SignalHandler.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_base_SignalHandler.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_base_SignalHandler.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]SignalHandler.cpp to es40_idb_base_SignalHandler.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]SignalHandler.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_base_SignalHandler.obj
$ ENDIF
$!
$! Check if es40_idb_base_ThreadLocal.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]ThreadLocal.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_base_ThreadLocal.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_base_ThreadLocal.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]ThreadLocal.cpp to es40_idb_base_ThreadLocal.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]ThreadLocal.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_base_ThreadLocal.obj
$ ENDIF
$!
$! Check if es40_idb_base_Thread.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Thread.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_idb_base_Thread.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_idb_base_Thread.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Thread.cpp to es40_idb_base_Thread.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Thread.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_idb_base_Thread.obj
$ ENDIF
$!
$! Link es40_idb
$!
$ SAY "Linking es40_idb..."
$!
$ CXXLINK es40_idb_AliM1543C.obj,es40_idb_AliM1543C_ide.obj,es40_idb_AliM1543C_usb.obj,es40_idb_AlphaCPU.obj,es40_idb_AlphaCPU_ieeefloat.obj,es40_idb_AlphaCPU_vaxfloat.obj,es40_idb_AlphaCPU_vmspal.obj,es40_idb_AlphaSim.obj,es40_idb_Cirrus.obj,es40_idb_Configurator.obj,es40_idb_DEC21143.obj,es40_idb_Disk.obj,es40_idb_DiskController.obj,es40_idb_DiskDevice.obj,es40_idb_DiskFile.obj,es40_idb_DiskRam.obj,es40_idb_DMA.obj,es40_idb_DPR.obj,es40_idb_es40_debug.obj,es40_idb_Ethernet.obj,es40_idb_Flash.obj,es40_idb_FloppyController.obj,es40_idb_Keyboard.obj,es40_idb_lockstep.obj,es40_idb_PCIDevice.obj,es40_idb_Port80.obj,es40_idb_S3Trio64.obj,es40_idb_SCSIBus.obj,es40_idb_SCSIDevice.obj,es40_idb_Serial.obj,es40_idb_StdAfx.obj,es40_idb_Sym53C810.obj,es40_idb_Sym53C895.obj,es40_idb_SystemComponent.obj,es40_idb_System.obj,es40_idb_TraceEngine.obj,es40_idb_VGA.obj,es40_idb_gui_gui.obj,es40_idb_gui_gui_x11.obj,es40_idb_gui_keymap.obj,es40_idb_gui_scancodes.obj,es40_idb_gui_sdl.obj,es40_idb_base_Bugcheck.obj,es40_idb_base_ErrorHandler.obj,es40_idb_base_Event.obj,es40_idb_base_Exception.obj,es40_idb_base_Mutex.obj,es40_idb_base_NumberFormatter.obj,es40_idb_base_RefCountedObject.obj,es40_idb_base_Runnable.obj,es40_idb_base_RWLock.obj,es40_idb_base_Semaphore.obj,es40_idb_base_SignalHandler.obj,es40_idb_base_ThreadLocal.obj,es40_idb_base_Thread.obj'X11_LIB' -
           /EXECUTABLE=es40_idb.exe
$!
$! Compile sources for es40_lss
$!
$! Compile with the following defines: ES40,__USE_STD_IOSTREAM,IDB,LSS
$!
$ SAY "Compiling es40_lss..."
$!
$! Check if es40_lss_AliM1543C.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AliM1543C.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_AliM1543C.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_AliM1543C.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AliM1543C.cpp to es40_lss_AliM1543C.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AliM1543C.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_AliM1543C.obj
$ ENDIF
$!
$! Check if es40_lss_AliM1543C_ide.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AliM1543C_ide.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_AliM1543C_ide.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_AliM1543C_ide.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AliM1543C_ide.cpp to es40_lss_AliM1543C_ide.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AliM1543C_ide.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_AliM1543C_ide.obj
$ ENDIF
$!
$! Check if es40_lss_AliM1543C_usb.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AliM1543C_usb.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_AliM1543C_usb.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_AliM1543C_usb.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AliM1543C_usb.cpp to es40_lss_AliM1543C_usb.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AliM1543C_usb.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_AliM1543C_usb.obj
$ ENDIF
$!
$! Check if es40_lss_AlphaCPU.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_AlphaCPU.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_AlphaCPU.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU.cpp to es40_lss_AlphaCPU.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_AlphaCPU.obj
$ ENDIF
$!
$! Check if es40_lss_AlphaCPU_ieeefloat.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU_ieeefloat.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_AlphaCPU_ieeefloat.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_AlphaCPU_ieeefloat.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU_ieeefloat.cpp to es40_lss_AlphaCPU_ieeefloat.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU_ieeefloat.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_AlphaCPU_ieeefloat.obj
$ ENDIF
$!
$! Check if es40_lss_AlphaCPU_vaxfloat.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU_vaxfloat.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_AlphaCPU_vaxfloat.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_AlphaCPU_vaxfloat.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU_vaxfloat.cpp to es40_lss_AlphaCPU_vaxfloat.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU_vaxfloat.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_AlphaCPU_vaxfloat.obj
$ ENDIF
$!
$! Check if es40_lss_AlphaCPU_vmspal.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU_vmspal.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_AlphaCPU_vmspal.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_AlphaCPU_vmspal.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU_vmspal.cpp to es40_lss_AlphaCPU_vmspal.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU_vmspal.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_AlphaCPU_vmspal.obj
$ ENDIF
$!
$! Check if es40_lss_AlphaSim.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaSim.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_AlphaSim.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_AlphaSim.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaSim.cpp to es40_lss_AlphaSim.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaSim.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_AlphaSim.obj
$ ENDIF
$!
$! Check if es40_lss_Cirrus.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Cirrus.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_Cirrus.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_Cirrus.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Cirrus.cpp to es40_lss_Cirrus.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Cirrus.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_Cirrus.obj
$ ENDIF
$!
$! Check if es40_lss_Configurator.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Configurator.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_Configurator.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_Configurator.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Configurator.cpp to es40_lss_Configurator.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Configurator.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_Configurator.obj
$ ENDIF
$!
$! Check if es40_lss_DEC21143.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DEC21143.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_DEC21143.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_DEC21143.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DEC21143.cpp to es40_lss_DEC21143.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DEC21143.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_DEC21143.obj
$ ENDIF
$!
$! Check if es40_lss_Disk.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Disk.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_Disk.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_Disk.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Disk.cpp to es40_lss_Disk.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Disk.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_Disk.obj
$ ENDIF
$!
$! Check if es40_lss_DiskController.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskController.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_DiskController.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_DiskController.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskController.cpp to es40_lss_DiskController.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskController.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_DiskController.obj
$ ENDIF
$!
$! Check if es40_lss_DiskDevice.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskDevice.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_DiskDevice.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_DiskDevice.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskDevice.cpp to es40_lss_DiskDevice.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskDevice.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_DiskDevice.obj
$ ENDIF
$!
$! Check if es40_lss_DiskFile.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskFile.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_DiskFile.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_DiskFile.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskFile.cpp to es40_lss_DiskFile.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskFile.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_DiskFile.obj
$ ENDIF
$!
$! Check if es40_lss_DiskRam.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskRam.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_DiskRam.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_DiskRam.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskRam.cpp to es40_lss_DiskRam.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskRam.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_DiskRam.obj
$ ENDIF
$!
$! Check if es40_lss_DMA.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DMA.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_DMA.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_DMA.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DMA.cpp to es40_lss_DMA.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DMA.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_DMA.obj
$ ENDIF
$!
$! Check if es40_lss_DPR.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DPR.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_DPR.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_DPR.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DPR.cpp to es40_lss_DPR.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DPR.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_DPR.obj
$ ENDIF
$!
$! Check if es40_lss_es40_debug.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_debug.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_es40_debug.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_es40_debug.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile es40_debug.cpp to es40_lss_es40_debug.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX es40_debug.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_es40_debug.obj
$ ENDIF
$!
$! Check if es40_lss_Ethernet.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Ethernet.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_Ethernet.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_Ethernet.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Ethernet.cpp to es40_lss_Ethernet.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Ethernet.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_Ethernet.obj
$ ENDIF
$!
$! Check if es40_lss_Flash.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Flash.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_Flash.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_Flash.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Flash.cpp to es40_lss_Flash.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Flash.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_Flash.obj
$ ENDIF
$!
$! Check if es40_lss_FloppyController.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("FloppyController.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_FloppyController.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_FloppyController.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile FloppyController.cpp to es40_lss_FloppyController.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX FloppyController.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_FloppyController.obj
$ ENDIF
$!
$! Check if es40_lss_Keyboard.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Keyboard.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_Keyboard.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_Keyboard.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Keyboard.cpp to es40_lss_Keyboard.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Keyboard.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_Keyboard.obj
$ ENDIF
$!
$! Check if es40_lss_lockstep.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("lockstep.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_lockstep.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_lockstep.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile lockstep.cpp to es40_lss_lockstep.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX lockstep.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_lockstep.obj
$ ENDIF
$!
$! Check if es40_lss_PCIDevice.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("PCIDevice.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_PCIDevice.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_PCIDevice.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile PCIDevice.cpp to es40_lss_PCIDevice.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX PCIDevice.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_PCIDevice.obj
$ ENDIF
$!
$! Check if es40_lss_Port80.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Port80.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_Port80.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_Port80.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Port80.cpp to es40_lss_Port80.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Port80.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_Port80.obj
$ ENDIF
$!
$! Check if es40_lss_S3Trio64.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("S3Trio64.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_S3Trio64.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_S3Trio64.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile S3Trio64.cpp to es40_lss_S3Trio64.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX S3Trio64.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_S3Trio64.obj
$ ENDIF
$!
$! Check if es40_lss_SCSIBus.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("SCSIBus.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_SCSIBus.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_SCSIBus.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile SCSIBus.cpp to es40_lss_SCSIBus.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX SCSIBus.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_SCSIBus.obj
$ ENDIF
$!
$! Check if es40_lss_SCSIDevice.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("SCSIDevice.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_SCSIDevice.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_SCSIDevice.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile SCSIDevice.cpp to es40_lss_SCSIDevice.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX SCSIDevice.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_SCSIDevice.obj
$ ENDIF
$!
$! Check if es40_lss_Serial.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Serial.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_Serial.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_Serial.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Serial.cpp to es40_lss_Serial.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Serial.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_Serial.obj
$ ENDIF
$!
$! Check if es40_lss_StdAfx.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("StdAfx.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_StdAfx.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_StdAfx.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile StdAfx.cpp to es40_lss_StdAfx.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX StdAfx.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_StdAfx.obj
$ ENDIF
$!
$! Check if es40_lss_Sym53C810.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Sym53C810.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_Sym53C810.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_Sym53C810.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Sym53C810.cpp to es40_lss_Sym53C810.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Sym53C810.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_Sym53C810.obj
$ ENDIF
$!
$! Check if es40_lss_Sym53C895.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Sym53C895.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_Sym53C895.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_Sym53C895.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Sym53C895.cpp to es40_lss_Sym53C895.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Sym53C895.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_Sym53C895.obj
$ ENDIF
$!
$! Check if es40_lss_SystemComponent.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("SystemComponent.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_SystemComponent.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_SystemComponent.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile SystemComponent.cpp to es40_lss_SystemComponent.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX SystemComponent.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_SystemComponent.obj
$ ENDIF
$!
$! Check if es40_lss_System.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("System.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_System.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_System.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile System.cpp to es40_lss_System.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX System.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_System.obj
$ ENDIF
$!
$! Check if es40_lss_TraceEngine.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("TraceEngine.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_TraceEngine.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_TraceEngine.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile TraceEngine.cpp to es40_lss_TraceEngine.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX TraceEngine.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_TraceEngine.obj
$ ENDIF
$!
$! Check if es40_lss_VGA.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("VGA.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_VGA.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_VGA.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile VGA.cpp to es40_lss_VGA.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX VGA.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_VGA.obj
$ ENDIF
$!
$! Check if es40_lss_gui_gui.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]gui.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_gui_gui.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_gui_gui.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]gui.cpp to es40_lss_gui_gui.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]gui.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_gui_gui.obj
$ ENDIF
$!
$! Check if es40_lss_gui_gui_x11.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]gui_x11.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_gui_gui_x11.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_gui_gui_x11.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]gui_x11.cpp to es40_lss_gui_gui_x11.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]gui_x11.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_gui_gui_x11.obj
$ ENDIF
$!
$! Check if es40_lss_gui_keymap.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]keymap.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_gui_keymap.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_gui_keymap.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]keymap.cpp to es40_lss_gui_keymap.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]keymap.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_gui_keymap.obj
$ ENDIF
$!
$! Check if es40_lss_gui_scancodes.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]scancodes.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_gui_scancodes.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_gui_scancodes.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]scancodes.cpp to es40_lss_gui_scancodes.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]scancodes.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_gui_scancodes.obj
$ ENDIF
$!
$! Check if es40_lss_gui_sdl.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]sdl.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_gui_sdl.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_gui_sdl.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]sdl.cpp to es40_lss_gui_sdl.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]sdl.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_gui_sdl.obj
$ ENDIF
$!
$! Check if es40_lss_base_Bugcheck.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Bugcheck.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_base_Bugcheck.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_base_Bugcheck.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Bugcheck.cpp to es40_lss_base_Bugcheck.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Bugcheck.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_base_Bugcheck.obj
$ ENDIF
$!
$! Check if es40_lss_base_ErrorHandler.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]ErrorHandler.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_base_ErrorHandler.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_base_ErrorHandler.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]ErrorHandler.cpp to es40_lss_base_ErrorHandler.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]ErrorHandler.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_base_ErrorHandler.obj
$ ENDIF
$!
$! Check if es40_lss_base_Event.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Event.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_base_Event.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_base_Event.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Event.cpp to es40_lss_base_Event.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Event.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_base_Event.obj
$ ENDIF
$!
$! Check if es40_lss_base_Exception.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Exception.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_base_Exception.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_base_Exception.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Exception.cpp to es40_lss_base_Exception.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Exception.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_base_Exception.obj
$ ENDIF
$!
$! Check if es40_lss_base_Mutex.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Mutex.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_base_Mutex.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_base_Mutex.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Mutex.cpp to es40_lss_base_Mutex.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Mutex.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_base_Mutex.obj
$ ENDIF
$!
$! Check if es40_lss_base_NumberFormatter.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]NumberFormatter.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_base_NumberFormatter.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_base_NumberFormatter.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]NumberFormatter.cpp to es40_lss_base_NumberFormatter.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]NumberFormatter.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_base_NumberFormatter.obj
$ ENDIF
$!
$! Check if es40_lss_base_RefCountedObject.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]RefCountedObject.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_base_RefCountedObject.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_base_RefCountedObject.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]RefCountedObject.cpp to es40_lss_base_RefCountedObject.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]RefCountedObject.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_base_RefCountedObject.obj
$ ENDIF
$!
$! Check if es40_lss_base_Runnable.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Runnable.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_base_Runnable.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_base_Runnable.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Runnable.cpp to es40_lss_base_Runnable.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Runnable.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_base_Runnable.obj
$ ENDIF
$!
$! Check if es40_lss_base_RWLock.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]RWLock.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_base_RWLock.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_base_RWLock.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]RWLock.cpp to es40_lss_base_RWLock.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]RWLock.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_base_RWLock.obj
$ ENDIF
$!
$! Check if es40_lss_base_Semaphore.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Semaphore.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_base_Semaphore.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_base_Semaphore.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Semaphore.cpp to es40_lss_base_Semaphore.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Semaphore.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_base_Semaphore.obj
$ ENDIF
$!
$! Check if es40_lss_base_SignalHandler.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]SignalHandler.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_base_SignalHandler.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_base_SignalHandler.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]SignalHandler.cpp to es40_lss_base_SignalHandler.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]SignalHandler.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_base_SignalHandler.obj
$ ENDIF
$!
$! Check if es40_lss_base_ThreadLocal.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]ThreadLocal.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_base_ThreadLocal.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_base_ThreadLocal.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]ThreadLocal.cpp to es40_lss_base_ThreadLocal.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]ThreadLocal.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_base_ThreadLocal.obj
$ ENDIF
$!
$! Check if es40_lss_base_Thread.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Thread.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lss_base_Thread.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lss_base_Thread.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Thread.cpp to es40_lss_base_Thread.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Thread.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSS'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lss_base_Thread.obj
$ ENDIF
$!
$! Link es40_lss
$!
$ SAY "Linking es40_lss..."
$!
$ CXXLINK es40_lss_AliM1543C.obj,es40_lss_AliM1543C_ide.obj,es40_lss_AliM1543C_usb.obj,es40_lss_AlphaCPU.obj,es40_lss_AlphaCPU_ieeefloat.obj,es40_lss_AlphaCPU_vaxfloat.obj,es40_lss_AlphaCPU_vmspal.obj,es40_lss_AlphaSim.obj,es40_lss_Cirrus.obj,es40_lss_Configurator.obj,es40_lss_DEC21143.obj,es40_lss_Disk.obj,es40_lss_DiskController.obj,es40_lss_DiskDevice.obj,es40_lss_DiskFile.obj,es40_lss_DiskRam.obj,es40_lss_DMA.obj,es40_lss_DPR.obj,es40_lss_es40_debug.obj,es40_lss_Ethernet.obj,es40_lss_Flash.obj,es40_lss_FloppyController.obj,es40_lss_Keyboard.obj,es40_lss_lockstep.obj,es40_lss_PCIDevice.obj,es40_lss_Port80.obj,es40_lss_S3Trio64.obj,es40_lss_SCSIBus.obj,es40_lss_SCSIDevice.obj,es40_lss_Serial.obj,es40_lss_StdAfx.obj,es40_lss_Sym53C810.obj,es40_lss_Sym53C895.obj,es40_lss_SystemComponent.obj,es40_lss_System.obj,es40_lss_TraceEngine.obj,es40_lss_VGA.obj,es40_lss_gui_gui.obj,es40_lss_gui_gui_x11.obj,es40_lss_gui_keymap.obj,es40_lss_gui_scancodes.obj,es40_lss_gui_sdl.obj,es40_lss_base_Bugcheck.obj,es40_lss_base_ErrorHandler.obj,es40_lss_base_Event.obj,es40_lss_base_Exception.obj,es40_lss_base_Mutex.obj,es40_lss_base_NumberFormatter.obj,es40_lss_base_RefCountedObject.obj,es40_lss_base_Runnable.obj,es40_lss_base_RWLock.obj,es40_lss_base_Semaphore.obj,es40_lss_base_SignalHandler.obj,es40_lss_base_ThreadLocal.obj,es40_lss_base_Thread.obj'X11_LIB' -
           /EXECUTABLE=es40_lss.exe
$!
$! Compile sources for es40_lsm
$!
$! Compile with the following defines: ES40,__USE_STD_IOSTREAM,IDB,LSM
$!
$ SAY "Compiling es40_lsm..."
$!
$! Check if es40_lsm_AliM1543C.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AliM1543C.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_AliM1543C.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_AliM1543C.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AliM1543C.cpp to es40_lsm_AliM1543C.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AliM1543C.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_AliM1543C.obj
$ ENDIF
$!
$! Check if es40_lsm_AliM1543C_ide.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AliM1543C_ide.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_AliM1543C_ide.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_AliM1543C_ide.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AliM1543C_ide.cpp to es40_lsm_AliM1543C_ide.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AliM1543C_ide.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_AliM1543C_ide.obj
$ ENDIF
$!
$! Check if es40_lsm_AliM1543C_usb.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AliM1543C_usb.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_AliM1543C_usb.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_AliM1543C_usb.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AliM1543C_usb.cpp to es40_lsm_AliM1543C_usb.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AliM1543C_usb.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_AliM1543C_usb.obj
$ ENDIF
$!
$! Check if es40_lsm_AlphaCPU.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_AlphaCPU.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_AlphaCPU.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU.cpp to es40_lsm_AlphaCPU.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_AlphaCPU.obj
$ ENDIF
$!
$! Check if es40_lsm_AlphaCPU_ieeefloat.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU_ieeefloat.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_AlphaCPU_ieeefloat.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_AlphaCPU_ieeefloat.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU_ieeefloat.cpp to es40_lsm_AlphaCPU_ieeefloat.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU_ieeefloat.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_AlphaCPU_ieeefloat.obj
$ ENDIF
$!
$! Check if es40_lsm_AlphaCPU_vaxfloat.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU_vaxfloat.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_AlphaCPU_vaxfloat.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_AlphaCPU_vaxfloat.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU_vaxfloat.cpp to es40_lsm_AlphaCPU_vaxfloat.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU_vaxfloat.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_AlphaCPU_vaxfloat.obj
$ ENDIF
$!
$! Check if es40_lsm_AlphaCPU_vmspal.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaCPU_vmspal.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_AlphaCPU_vmspal.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_AlphaCPU_vmspal.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaCPU_vmspal.cpp to es40_lsm_AlphaCPU_vmspal.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaCPU_vmspal.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_AlphaCPU_vmspal.obj
$ ENDIF
$!
$! Check if es40_lsm_AlphaSim.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("AlphaSim.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_AlphaSim.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_AlphaSim.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile AlphaSim.cpp to es40_lsm_AlphaSim.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX AlphaSim.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_AlphaSim.obj
$ ENDIF
$!
$! Check if es40_lsm_Cirrus.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Cirrus.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_Cirrus.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_Cirrus.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Cirrus.cpp to es40_lsm_Cirrus.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Cirrus.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_Cirrus.obj
$ ENDIF
$!
$! Check if es40_lsm_Configurator.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Configurator.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_Configurator.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_Configurator.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Configurator.cpp to es40_lsm_Configurator.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Configurator.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_Configurator.obj
$ ENDIF
$!
$! Check if es40_lsm_DEC21143.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DEC21143.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_DEC21143.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_DEC21143.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DEC21143.cpp to es40_lsm_DEC21143.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DEC21143.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_DEC21143.obj
$ ENDIF
$!
$! Check if es40_lsm_Disk.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Disk.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_Disk.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_Disk.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Disk.cpp to es40_lsm_Disk.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Disk.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_Disk.obj
$ ENDIF
$!
$! Check if es40_lsm_DiskController.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskController.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_DiskController.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_DiskController.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskController.cpp to es40_lsm_DiskController.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskController.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_DiskController.obj
$ ENDIF
$!
$! Check if es40_lsm_DiskDevice.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskDevice.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_DiskDevice.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_DiskDevice.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskDevice.cpp to es40_lsm_DiskDevice.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskDevice.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_DiskDevice.obj
$ ENDIF
$!
$! Check if es40_lsm_DiskFile.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskFile.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_DiskFile.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_DiskFile.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskFile.cpp to es40_lsm_DiskFile.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskFile.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_DiskFile.obj
$ ENDIF
$!
$! Check if es40_lsm_DiskRam.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DiskRam.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_DiskRam.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_DiskRam.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DiskRam.cpp to es40_lsm_DiskRam.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DiskRam.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_DiskRam.obj
$ ENDIF
$!
$! Check if es40_lsm_DMA.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DMA.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_DMA.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_DMA.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DMA.cpp to es40_lsm_DMA.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DMA.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_DMA.obj
$ ENDIF
$!
$! Check if es40_lsm_DPR.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("DPR.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_DPR.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_DPR.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile DPR.cpp to es40_lsm_DPR.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX DPR.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_DPR.obj
$ ENDIF
$!
$! Check if es40_lsm_es40_debug.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_debug.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_es40_debug.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_es40_debug.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile es40_debug.cpp to es40_lsm_es40_debug.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX es40_debug.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_es40_debug.obj
$ ENDIF
$!
$! Check if es40_lsm_Ethernet.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Ethernet.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_Ethernet.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_Ethernet.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Ethernet.cpp to es40_lsm_Ethernet.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Ethernet.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_Ethernet.obj
$ ENDIF
$!
$! Check if es40_lsm_Flash.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Flash.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_Flash.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_Flash.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Flash.cpp to es40_lsm_Flash.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Flash.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_Flash.obj
$ ENDIF
$!
$! Check if es40_lsm_FloppyController.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("FloppyController.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_FloppyController.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_FloppyController.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile FloppyController.cpp to es40_lsm_FloppyController.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX FloppyController.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_FloppyController.obj
$ ENDIF
$!
$! Check if es40_lsm_Keyboard.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Keyboard.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_Keyboard.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_Keyboard.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Keyboard.cpp to es40_lsm_Keyboard.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Keyboard.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_Keyboard.obj
$ ENDIF
$!
$! Check if es40_lsm_lockstep.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("lockstep.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_lockstep.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_lockstep.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile lockstep.cpp to es40_lsm_lockstep.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX lockstep.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_lockstep.obj
$ ENDIF
$!
$! Check if es40_lsm_PCIDevice.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("PCIDevice.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_PCIDevice.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_PCIDevice.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile PCIDevice.cpp to es40_lsm_PCIDevice.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX PCIDevice.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_PCIDevice.obj
$ ENDIF
$!
$! Check if es40_lsm_Port80.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Port80.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_Port80.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_Port80.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Port80.cpp to es40_lsm_Port80.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Port80.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_Port80.obj
$ ENDIF
$!
$! Check if es40_lsm_S3Trio64.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("S3Trio64.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_S3Trio64.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_S3Trio64.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile S3Trio64.cpp to es40_lsm_S3Trio64.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX S3Trio64.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_S3Trio64.obj
$ ENDIF
$!
$! Check if es40_lsm_SCSIBus.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("SCSIBus.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_SCSIBus.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_SCSIBus.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile SCSIBus.cpp to es40_lsm_SCSIBus.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX SCSIBus.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_SCSIBus.obj
$ ENDIF
$!
$! Check if es40_lsm_SCSIDevice.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("SCSIDevice.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_SCSIDevice.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_SCSIDevice.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile SCSIDevice.cpp to es40_lsm_SCSIDevice.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX SCSIDevice.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_SCSIDevice.obj
$ ENDIF
$!
$! Check if es40_lsm_Serial.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Serial.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_Serial.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_Serial.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Serial.cpp to es40_lsm_Serial.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Serial.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_Serial.obj
$ ENDIF
$!
$! Check if es40_lsm_StdAfx.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("StdAfx.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_StdAfx.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_StdAfx.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile StdAfx.cpp to es40_lsm_StdAfx.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX StdAfx.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_StdAfx.obj
$ ENDIF
$!
$! Check if es40_lsm_Sym53C810.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Sym53C810.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_Sym53C810.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_Sym53C810.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Sym53C810.cpp to es40_lsm_Sym53C810.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Sym53C810.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_Sym53C810.obj
$ ENDIF
$!
$! Check if es40_lsm_Sym53C895.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("Sym53C895.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_Sym53C895.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_Sym53C895.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile Sym53C895.cpp to es40_lsm_Sym53C895.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX Sym53C895.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_Sym53C895.obj
$ ENDIF
$!
$! Check if es40_lsm_SystemComponent.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("SystemComponent.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_SystemComponent.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_SystemComponent.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile SystemComponent.cpp to es40_lsm_SystemComponent.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX SystemComponent.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_SystemComponent.obj
$ ENDIF
$!
$! Check if es40_lsm_System.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("System.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_System.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_System.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile System.cpp to es40_lsm_System.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX System.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_System.obj
$ ENDIF
$!
$! Check if es40_lsm_TraceEngine.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("TraceEngine.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_TraceEngine.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_TraceEngine.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile TraceEngine.cpp to es40_lsm_TraceEngine.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX TraceEngine.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_TraceEngine.obj
$ ENDIF
$!
$! Check if es40_lsm_VGA.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("VGA.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_VGA.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_VGA.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile VGA.cpp to es40_lsm_VGA.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX VGA.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_VGA.obj
$ ENDIF
$!
$! Check if es40_lsm_gui_gui.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]gui.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_gui_gui.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_gui_gui.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]gui.cpp to es40_lsm_gui_gui.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]gui.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_gui_gui.obj
$ ENDIF
$!
$! Check if es40_lsm_gui_gui_x11.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]gui_x11.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_gui_gui_x11.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_gui_gui_x11.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]gui_x11.cpp to es40_lsm_gui_gui_x11.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]gui_x11.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_gui_gui_x11.obj
$ ENDIF
$!
$! Check if es40_lsm_gui_keymap.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]keymap.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_gui_keymap.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_gui_keymap.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]keymap.cpp to es40_lsm_gui_keymap.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]keymap.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_gui_keymap.obj
$ ENDIF
$!
$! Check if es40_lsm_gui_scancodes.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]scancodes.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_gui_scancodes.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_gui_scancodes.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]scancodes.cpp to es40_lsm_gui_scancodes.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]scancodes.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_gui_scancodes.obj
$ ENDIF
$!
$! Check if es40_lsm_gui_sdl.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.gui]sdl.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_gui_sdl.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_gui_sdl.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.gui]sdl.cpp to es40_lsm_gui_sdl.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.gui]sdl.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_gui_sdl.obj
$ ENDIF
$!
$! Check if es40_lsm_base_Bugcheck.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Bugcheck.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_base_Bugcheck.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_base_Bugcheck.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Bugcheck.cpp to es40_lsm_base_Bugcheck.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Bugcheck.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_base_Bugcheck.obj
$ ENDIF
$!
$! Check if es40_lsm_base_ErrorHandler.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]ErrorHandler.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_base_ErrorHandler.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_base_ErrorHandler.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]ErrorHandler.cpp to es40_lsm_base_ErrorHandler.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]ErrorHandler.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_base_ErrorHandler.obj
$ ENDIF
$!
$! Check if es40_lsm_base_Event.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Event.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_base_Event.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_base_Event.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Event.cpp to es40_lsm_base_Event.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Event.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_base_Event.obj
$ ENDIF
$!
$! Check if es40_lsm_base_Exception.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Exception.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_base_Exception.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_base_Exception.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Exception.cpp to es40_lsm_base_Exception.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Exception.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_base_Exception.obj
$ ENDIF
$!
$! Check if es40_lsm_base_Mutex.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Mutex.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_base_Mutex.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_base_Mutex.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Mutex.cpp to es40_lsm_base_Mutex.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Mutex.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_base_Mutex.obj
$ ENDIF
$!
$! Check if es40_lsm_base_NumberFormatter.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]NumberFormatter.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_base_NumberFormatter.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_base_NumberFormatter.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]NumberFormatter.cpp to es40_lsm_base_NumberFormatter.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]NumberFormatter.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_base_NumberFormatter.obj
$ ENDIF
$!
$! Check if es40_lsm_base_RefCountedObject.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]RefCountedObject.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_base_RefCountedObject.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_base_RefCountedObject.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]RefCountedObject.cpp to es40_lsm_base_RefCountedObject.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]RefCountedObject.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_base_RefCountedObject.obj
$ ENDIF
$!
$! Check if es40_lsm_base_Runnable.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Runnable.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_base_Runnable.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_base_Runnable.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Runnable.cpp to es40_lsm_base_Runnable.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Runnable.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_base_Runnable.obj
$ ENDIF
$!
$! Check if es40_lsm_base_RWLock.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]RWLock.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_base_RWLock.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_base_RWLock.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]RWLock.cpp to es40_lsm_base_RWLock.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]RWLock.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_base_RWLock.obj
$ ENDIF
$!
$! Check if es40_lsm_base_Semaphore.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Semaphore.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_base_Semaphore.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_base_Semaphore.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Semaphore.cpp to es40_lsm_base_Semaphore.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Semaphore.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_base_Semaphore.obj
$ ENDIF
$!
$! Check if es40_lsm_base_SignalHandler.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]SignalHandler.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_base_SignalHandler.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_base_SignalHandler.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]SignalHandler.cpp to es40_lsm_base_SignalHandler.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]SignalHandler.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_base_SignalHandler.obj
$ ENDIF
$!
$! Check if es40_lsm_base_ThreadLocal.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]ThreadLocal.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_base_ThreadLocal.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_base_ThreadLocal.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]ThreadLocal.cpp to es40_lsm_base_ThreadLocal.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]ThreadLocal.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_base_ThreadLocal.obj
$ ENDIF
$!
$! Check if es40_lsm_base_Thread.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("[.base]Thread.cpp","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_lsm_base_Thread.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_lsm_base_Thread.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile [.base]Thread.cpp to es40_lsm_base_Thread.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX [.base]Thread.cpp -
         /DEFINE=(ES40,__USE_STD_IOSTREAM,IDB,LSM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_lsm_base_Thread.obj
$ ENDIF
$!
$! Link es40_lsm
$!
$ SAY "Linking es40_lsm..."
$!
$ CXXLINK es40_lsm_AliM1543C.obj,es40_lsm_AliM1543C_ide.obj,es40_lsm_AliM1543C_usb.obj,es40_lsm_AlphaCPU.obj,es40_lsm_AlphaCPU_ieeefloat.obj,es40_lsm_AlphaCPU_vaxfloat.obj,es40_lsm_AlphaCPU_vmspal.obj,es40_lsm_AlphaSim.obj,es40_lsm_Cirrus.obj,es40_lsm_Configurator.obj,es40_lsm_DEC21143.obj,es40_lsm_Disk.obj,es40_lsm_DiskController.obj,es40_lsm_DiskDevice.obj,es40_lsm_DiskFile.obj,es40_lsm_DiskRam.obj,es40_lsm_DMA.obj,es40_lsm_DPR.obj,es40_lsm_es40_debug.obj,es40_lsm_Ethernet.obj,es40_lsm_Flash.obj,es40_lsm_FloppyController.obj,es40_lsm_Keyboard.obj,es40_lsm_lockstep.obj,es40_lsm_PCIDevice.obj,es40_lsm_Port80.obj,es40_lsm_S3Trio64.obj,es40_lsm_SCSIBus.obj,es40_lsm_SCSIDevice.obj,es40_lsm_Serial.obj,es40_lsm_StdAfx.obj,es40_lsm_Sym53C810.obj,es40_lsm_Sym53C895.obj,es40_lsm_SystemComponent.obj,es40_lsm_System.obj,es40_lsm_TraceEngine.obj,es40_lsm_VGA.obj,es40_lsm_gui_gui.obj,es40_lsm_gui_gui_x11.obj,es40_lsm_gui_keymap.obj,es40_lsm_gui_scancodes.obj,es40_lsm_gui_sdl.obj,es40_lsm_base_Bugcheck.obj,es40_lsm_base_ErrorHandler.obj,es40_lsm_base_Event.obj,es40_lsm_base_Exception.obj,es40_lsm_base_Mutex.obj,es40_lsm_base_NumberFormatter.obj,es40_lsm_base_RefCountedObject.obj,es40_lsm_base_Runnable.obj,es40_lsm_base_RWLock.obj,es40_lsm_base_Semaphore.obj,es40_lsm_base_SignalHandler.obj,es40_lsm_base_ThreadLocal.obj,es40_lsm_base_Thread.obj'X11_LIB' -
           /EXECUTABLE=es40_lsm.exe
$!
$! Compile sources for es40_cfg
$!
$! Compile with the following defines: ES40,__USE_STD_IOSTREAM
$!
$ SAY "Compiling es40_cfg..."
$!
$! Check if es40_cfg_es40-cfg.cpp
.obj is up-to-date...
$!
$ SRCTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40-cfg.cpp
","RDT"),"COMPARISON")
$ OBJFILE = F$SEARCH("es40_cfg_es40-cfg.cpp
.obj")
$ IF OBJFILE .NES. ""
$ THEN
$   OBJTIME = F$CVTIME(F$FILE_ATTRIBUTES("es40_cfg_es40-cfg.cpp
.obj","RDT"),"COMPARISON")
$ ELSE
$   OBJTIME = F$CVTIME("01-JAN-1970 00:00:00.00","COMPARISON")
$ ENDIF
$!
$! Compile es40-cfg.cpp
 to es40_cfg_es40-cfg.cpp
.obj
$ IF SRCTIME .GTS. OBJTIME
$ THEN
$   CXX es40-cfg.cpp
 -
         /DEFINE=(ES40,__USE_STD_IOSTREAM'X11_DEF') -
         /INCLUDE=("''ES40_ROOT'/GUI","''ES40_ROOT'/BASE'") -
         /STANDARD=GNU -
         /ARCHITECTURE=HOST -
         /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
         /OBJECT=es40_cfg_es40-cfg.cpp
.obj
$ ENDIF
$!
$! Link es40_cfg
$!
$ SAY "Linking es40_cfg..."
$!
$ CXXLINK es40_cfg_es40-cfg.cpp
.obj'X11_LIB' -
           /EXECUTABLE=es40_cfg.exe
$!
$ SAY "That's all, folks!"
$!
$ EXIT

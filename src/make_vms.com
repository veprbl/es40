$ SET VERIFY
$!
$! ES40 Emulator
$! Copyright (C) 2007-2008 by the ES40 Emulator Project
$!
$! This file was created by make_vms.sh. Please refer to that file
$! for more information.
$!
$ SAY = "WRITE SYS$OUTPUT"
$!
$! Compile sources for es40
$!
$! Compile with the following defines: ES40
$!
$ SAY "Compiling es40..."
$!
$! Compile AliM1543C.cpp to es40_AliM1543C.obj
$ CXX AliM1543C.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_AliM1543C.obj
$!
$! Compile AliM1543C_ide.cpp to es40_AliM1543C_ide.obj
$ CXX AliM1543C_ide.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_AliM1543C_ide.obj
$!
$! Compile AliM1543C_usb.cpp to es40_AliM1543C_usb.obj
$ CXX AliM1543C_usb.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_AliM1543C_usb.obj
$!
$! Compile AlphaCPU.cpp to es40_AlphaCPU.obj
$ CXX AlphaCPU.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_AlphaCPU.obj
$!
$! Compile AlphaCPU_ieeefloat.cpp to es40_AlphaCPU_ieeefloat.obj
$ CXX AlphaCPU_ieeefloat.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_AlphaCPU_ieeefloat.obj
$!
$! Compile AlphaCPU_vaxfloat.cpp to es40_AlphaCPU_vaxfloat.obj
$ CXX AlphaCPU_vaxfloat.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_AlphaCPU_vaxfloat.obj
$!
$! Compile AlphaCPU_vmspal.cpp to es40_AlphaCPU_vmspal.obj
$ CXX AlphaCPU_vmspal.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_AlphaCPU_vmspal.obj
$!
$! Compile AlphaSim.cpp to es40_AlphaSim.obj
$ CXX AlphaSim.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_AlphaSim.obj
$!
$! Compile Cirrus.cpp to es40_Cirrus.obj
$ CXX Cirrus.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_Cirrus.obj
$!
$! Compile Configurator.cpp to es40_Configurator.obj
$ CXX Configurator.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_Configurator.obj
$!
$! Compile DEC21143.cpp to es40_DEC21143.obj
$ CXX DEC21143.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_DEC21143.obj
$!
$! Compile Disk.cpp to es40_Disk.obj
$ CXX Disk.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_Disk.obj
$!
$! Compile DiskController.cpp to es40_DiskController.obj
$ CXX DiskController.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_DiskController.obj
$!
$! Compile DiskDevice.cpp to es40_DiskDevice.obj
$ CXX DiskDevice.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_DiskDevice.obj
$!
$! Compile DiskFile.cpp to es40_DiskFile.obj
$ CXX DiskFile.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_DiskFile.obj
$!
$! Compile DiskRam.cpp to es40_DiskRam.obj
$ CXX DiskRam.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_DiskRam.obj
$!
$! Compile DMA.cpp to es40_DMA.obj
$ CXX DMA.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_DMA.obj
$!
$! Compile DPR.cpp to es40_DPR.obj
$ CXX DPR.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_DPR.obj
$!
$! Compile es40_debug.cpp to es40_es40_debug.obj
$ CXX es40_debug.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_es40_debug.obj
$!
$! Compile Ethernet.cpp to es40_Ethernet.obj
$ CXX Ethernet.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_Ethernet.obj
$!
$! Compile Exception.cpp to es40_Exception.obj
$ CXX Exception.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_Exception.obj
$!
$! Compile Flash.cpp to es40_Flash.obj
$ CXX Flash.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_Flash.obj
$!
$! Compile FloppyController.cpp to es40_FloppyController.obj
$ CXX FloppyController.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_FloppyController.obj
$!
$! Compile Keyboard.cpp to es40_Keyboard.obj
$ CXX Keyboard.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_Keyboard.obj
$!
$! Compile lockstep.cpp to es40_lockstep.obj
$ CXX lockstep.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lockstep.obj
$!
$! Compile PCIDevice.cpp to es40_PCIDevice.obj
$ CXX PCIDevice.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_PCIDevice.obj
$!
$! Compile Port80.cpp to es40_Port80.obj
$ CXX Port80.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_Port80.obj
$!
$! Compile S3Trio64.cpp to es40_S3Trio64.obj
$ CXX S3Trio64.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_S3Trio64.obj
$!
$! Compile SCSIBus.cpp to es40_SCSIBus.obj
$ CXX SCSIBus.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_SCSIBus.obj
$!
$! Compile SCSIDevice.cpp to es40_SCSIDevice.obj
$ CXX SCSIDevice.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_SCSIDevice.obj
$!
$! Compile Serial.cpp to es40_Serial.obj
$ CXX Serial.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_Serial.obj
$!
$! Compile StdAfx.cpp to es40_StdAfx.obj
$ CXX StdAfx.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_StdAfx.obj
$!
$! Compile Sym53C810.cpp to es40_Sym53C810.obj
$ CXX Sym53C810.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_Sym53C810.obj
$!
$! Compile Sym53C895.cpp to es40_Sym53C895.obj
$ CXX Sym53C895.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_Sym53C895.obj
$!
$! Compile SystemComponent.cpp to es40_SystemComponent.obj
$ CXX SystemComponent.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_SystemComponent.obj
$!
$! Compile System.cpp to es40_System.obj
$ CXX System.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_System.obj
$!
$! Compile TraceEngine.cpp to es40_TraceEngine.obj
$ CXX TraceEngine.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_TraceEngine.obj
$!
$! Compile VGA.cpp to es40_VGA.obj
$ CXX VGA.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_VGA.obj
$!
$! Compile [.gui]gui.cpp to es40_gui.obj
$ CXX [.gui]gui.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_gui.obj
$!
$! Compile [.gui]gui_x11.cpp to es40_gui_x11.obj
$ CXX [.gui]gui_x11.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_gui_x11.obj
$!
$! Compile [.gui]keymap.cpp to es40_keymap.obj
$ CXX [.gui]keymap.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_keymap.obj
$!
$! Compile [.gui]scancodes.cpp to es40_scancodes.obj
$ CXX [.gui]scancodes.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_scancodes.obj
$!
$! Compile [.gui]sdl.cpp to es40_sdl.obj
$ CXX [.gui]sdl.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_sdl.obj
$!
$! Link es40
$!
$ SAY "Linking es40..."
$!
$ CXXLINK  es40_AliM1543C.obj es40_AliM1543C_ide.obj es40_AliM1543C_usb.obj es40_AlphaCPU.obj es40_AlphaCPU_ieeefloat.obj es40_AlphaCPU_vaxfloat.obj es40_AlphaCPU_vmspal.obj es40_AlphaSim.obj es40_Cirrus.obj es40_Configurator.obj es40_DEC21143.obj es40_Disk.obj es40_DiskController.obj es40_DiskDevice.obj es40_DiskFile.obj es40_DiskRam.obj es40_DMA.obj es40_DPR.obj es40_es40_debug.obj es40_Ethernet.obj es40_Exception.obj es40_Flash.obj es40_FloppyController.obj es40_Keyboard.obj es40_lockstep.obj es40_PCIDevice.obj es40_Port80.obj es40_S3Trio64.obj es40_SCSIBus.obj es40_SCSIDevice.obj es40_Serial.obj es40_StdAfx.obj es40_Sym53C810.obj es40_Sym53C895.obj es40_SystemComponent.obj es40_System.obj es40_TraceEngine.obj es40_VGA.obj es40_gui.obj es40_gui_x11.obj es40_keymap.obj es40_scancodes.obj es40_sdl.obj -
           /EXECUTABLE=es40.exe
$!
$! Compile sources for es40_idb
$!
$! Compile with the following defines: ES40,IDB
$!
$ SAY "Compiling es40_idb..."
$!
$! Compile AliM1543C.cpp to es40_idb_AliM1543C.obj
$ CXX AliM1543C.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_AliM1543C.obj
$!
$! Compile AliM1543C_ide.cpp to es40_idb_AliM1543C_ide.obj
$ CXX AliM1543C_ide.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_AliM1543C_ide.obj
$!
$! Compile AliM1543C_usb.cpp to es40_idb_AliM1543C_usb.obj
$ CXX AliM1543C_usb.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_AliM1543C_usb.obj
$!
$! Compile AlphaCPU.cpp to es40_idb_AlphaCPU.obj
$ CXX AlphaCPU.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_AlphaCPU.obj
$!
$! Compile AlphaCPU_ieeefloat.cpp to es40_idb_AlphaCPU_ieeefloat.obj
$ CXX AlphaCPU_ieeefloat.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_AlphaCPU_ieeefloat.obj
$!
$! Compile AlphaCPU_vaxfloat.cpp to es40_idb_AlphaCPU_vaxfloat.obj
$ CXX AlphaCPU_vaxfloat.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_AlphaCPU_vaxfloat.obj
$!
$! Compile AlphaCPU_vmspal.cpp to es40_idb_AlphaCPU_vmspal.obj
$ CXX AlphaCPU_vmspal.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_AlphaCPU_vmspal.obj
$!
$! Compile AlphaSim.cpp to es40_idb_AlphaSim.obj
$ CXX AlphaSim.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_AlphaSim.obj
$!
$! Compile Cirrus.cpp to es40_idb_Cirrus.obj
$ CXX Cirrus.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_Cirrus.obj
$!
$! Compile Configurator.cpp to es40_idb_Configurator.obj
$ CXX Configurator.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_Configurator.obj
$!
$! Compile DEC21143.cpp to es40_idb_DEC21143.obj
$ CXX DEC21143.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_DEC21143.obj
$!
$! Compile Disk.cpp to es40_idb_Disk.obj
$ CXX Disk.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_Disk.obj
$!
$! Compile DiskController.cpp to es40_idb_DiskController.obj
$ CXX DiskController.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_DiskController.obj
$!
$! Compile DiskDevice.cpp to es40_idb_DiskDevice.obj
$ CXX DiskDevice.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_DiskDevice.obj
$!
$! Compile DiskFile.cpp to es40_idb_DiskFile.obj
$ CXX DiskFile.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_DiskFile.obj
$!
$! Compile DiskRam.cpp to es40_idb_DiskRam.obj
$ CXX DiskRam.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_DiskRam.obj
$!
$! Compile DMA.cpp to es40_idb_DMA.obj
$ CXX DMA.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_DMA.obj
$!
$! Compile DPR.cpp to es40_idb_DPR.obj
$ CXX DPR.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_DPR.obj
$!
$! Compile es40_debug.cpp to es40_idb_es40_debug.obj
$ CXX es40_debug.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_es40_debug.obj
$!
$! Compile Ethernet.cpp to es40_idb_Ethernet.obj
$ CXX Ethernet.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_Ethernet.obj
$!
$! Compile Exception.cpp to es40_idb_Exception.obj
$ CXX Exception.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_Exception.obj
$!
$! Compile Flash.cpp to es40_idb_Flash.obj
$ CXX Flash.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_Flash.obj
$!
$! Compile FloppyController.cpp to es40_idb_FloppyController.obj
$ CXX FloppyController.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_FloppyController.obj
$!
$! Compile Keyboard.cpp to es40_idb_Keyboard.obj
$ CXX Keyboard.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_Keyboard.obj
$!
$! Compile lockstep.cpp to es40_idb_lockstep.obj
$ CXX lockstep.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_lockstep.obj
$!
$! Compile PCIDevice.cpp to es40_idb_PCIDevice.obj
$ CXX PCIDevice.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_PCIDevice.obj
$!
$! Compile Port80.cpp to es40_idb_Port80.obj
$ CXX Port80.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_Port80.obj
$!
$! Compile S3Trio64.cpp to es40_idb_S3Trio64.obj
$ CXX S3Trio64.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_S3Trio64.obj
$!
$! Compile SCSIBus.cpp to es40_idb_SCSIBus.obj
$ CXX SCSIBus.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_SCSIBus.obj
$!
$! Compile SCSIDevice.cpp to es40_idb_SCSIDevice.obj
$ CXX SCSIDevice.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_SCSIDevice.obj
$!
$! Compile Serial.cpp to es40_idb_Serial.obj
$ CXX Serial.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_Serial.obj
$!
$! Compile StdAfx.cpp to es40_idb_StdAfx.obj
$ CXX StdAfx.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_StdAfx.obj
$!
$! Compile Sym53C810.cpp to es40_idb_Sym53C810.obj
$ CXX Sym53C810.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_Sym53C810.obj
$!
$! Compile Sym53C895.cpp to es40_idb_Sym53C895.obj
$ CXX Sym53C895.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_Sym53C895.obj
$!
$! Compile SystemComponent.cpp to es40_idb_SystemComponent.obj
$ CXX SystemComponent.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_SystemComponent.obj
$!
$! Compile System.cpp to es40_idb_System.obj
$ CXX System.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_System.obj
$!
$! Compile TraceEngine.cpp to es40_idb_TraceEngine.obj
$ CXX TraceEngine.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_TraceEngine.obj
$!
$! Compile VGA.cpp to es40_idb_VGA.obj
$ CXX VGA.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_VGA.obj
$!
$! Compile [.gui]gui.cpp to es40_idb_gui.obj
$ CXX [.gui]gui.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_gui.obj
$!
$! Compile [.gui]gui_x11.cpp to es40_idb_gui_x11.obj
$ CXX [.gui]gui_x11.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_gui_x11.obj
$!
$! Compile [.gui]keymap.cpp to es40_idb_keymap.obj
$ CXX [.gui]keymap.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_keymap.obj
$!
$! Compile [.gui]scancodes.cpp to es40_idb_scancodes.obj
$ CXX [.gui]scancodes.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_scancodes.obj
$!
$! Compile [.gui]sdl.cpp to es40_idb_sdl.obj
$ CXX [.gui]sdl.cpp -
       /DEFINE=(ES40,IDB) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_idb_sdl.obj
$!
$! Link es40_idb
$!
$ SAY "Linking es40_idb..."
$!
$ CXXLINK  es40_idb_AliM1543C.obj es40_idb_AliM1543C_ide.obj es40_idb_AliM1543C_usb.obj es40_idb_AlphaCPU.obj es40_idb_AlphaCPU_ieeefloat.obj es40_idb_AlphaCPU_vaxfloat.obj es40_idb_AlphaCPU_vmspal.obj es40_idb_AlphaSim.obj es40_idb_Cirrus.obj es40_idb_Configurator.obj es40_idb_DEC21143.obj es40_idb_Disk.obj es40_idb_DiskController.obj es40_idb_DiskDevice.obj es40_idb_DiskFile.obj es40_idb_DiskRam.obj es40_idb_DMA.obj es40_idb_DPR.obj es40_idb_es40_debug.obj es40_idb_Ethernet.obj es40_idb_Exception.obj es40_idb_Flash.obj es40_idb_FloppyController.obj es40_idb_Keyboard.obj es40_idb_lockstep.obj es40_idb_PCIDevice.obj es40_idb_Port80.obj es40_idb_S3Trio64.obj es40_idb_SCSIBus.obj es40_idb_SCSIDevice.obj es40_idb_Serial.obj es40_idb_StdAfx.obj es40_idb_Sym53C810.obj es40_idb_Sym53C895.obj es40_idb_SystemComponent.obj es40_idb_System.obj es40_idb_TraceEngine.obj es40_idb_VGA.obj es40_idb_gui.obj es40_idb_gui_x11.obj es40_idb_keymap.obj es40_idb_scancodes.obj es40_idb_sdl.obj -
           /EXECUTABLE=es40_idb.exe
$!
$! Compile sources for es40_lss
$!
$! Compile with the following defines: ES40,IDB,LSS
$!
$ SAY "Compiling es40_lss..."
$!
$! Compile AliM1543C.cpp to es40_lss_AliM1543C.obj
$ CXX AliM1543C.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_AliM1543C.obj
$!
$! Compile AliM1543C_ide.cpp to es40_lss_AliM1543C_ide.obj
$ CXX AliM1543C_ide.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_AliM1543C_ide.obj
$!
$! Compile AliM1543C_usb.cpp to es40_lss_AliM1543C_usb.obj
$ CXX AliM1543C_usb.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_AliM1543C_usb.obj
$!
$! Compile AlphaCPU.cpp to es40_lss_AlphaCPU.obj
$ CXX AlphaCPU.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_AlphaCPU.obj
$!
$! Compile AlphaCPU_ieeefloat.cpp to es40_lss_AlphaCPU_ieeefloat.obj
$ CXX AlphaCPU_ieeefloat.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_AlphaCPU_ieeefloat.obj
$!
$! Compile AlphaCPU_vaxfloat.cpp to es40_lss_AlphaCPU_vaxfloat.obj
$ CXX AlphaCPU_vaxfloat.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_AlphaCPU_vaxfloat.obj
$!
$! Compile AlphaCPU_vmspal.cpp to es40_lss_AlphaCPU_vmspal.obj
$ CXX AlphaCPU_vmspal.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_AlphaCPU_vmspal.obj
$!
$! Compile AlphaSim.cpp to es40_lss_AlphaSim.obj
$ CXX AlphaSim.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_AlphaSim.obj
$!
$! Compile Cirrus.cpp to es40_lss_Cirrus.obj
$ CXX Cirrus.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_Cirrus.obj
$!
$! Compile Configurator.cpp to es40_lss_Configurator.obj
$ CXX Configurator.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_Configurator.obj
$!
$! Compile DEC21143.cpp to es40_lss_DEC21143.obj
$ CXX DEC21143.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_DEC21143.obj
$!
$! Compile Disk.cpp to es40_lss_Disk.obj
$ CXX Disk.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_Disk.obj
$!
$! Compile DiskController.cpp to es40_lss_DiskController.obj
$ CXX DiskController.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_DiskController.obj
$!
$! Compile DiskDevice.cpp to es40_lss_DiskDevice.obj
$ CXX DiskDevice.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_DiskDevice.obj
$!
$! Compile DiskFile.cpp to es40_lss_DiskFile.obj
$ CXX DiskFile.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_DiskFile.obj
$!
$! Compile DiskRam.cpp to es40_lss_DiskRam.obj
$ CXX DiskRam.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_DiskRam.obj
$!
$! Compile DMA.cpp to es40_lss_DMA.obj
$ CXX DMA.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_DMA.obj
$!
$! Compile DPR.cpp to es40_lss_DPR.obj
$ CXX DPR.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_DPR.obj
$!
$! Compile es40_debug.cpp to es40_lss_es40_debug.obj
$ CXX es40_debug.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_es40_debug.obj
$!
$! Compile Ethernet.cpp to es40_lss_Ethernet.obj
$ CXX Ethernet.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_Ethernet.obj
$!
$! Compile Exception.cpp to es40_lss_Exception.obj
$ CXX Exception.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_Exception.obj
$!
$! Compile Flash.cpp to es40_lss_Flash.obj
$ CXX Flash.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_Flash.obj
$!
$! Compile FloppyController.cpp to es40_lss_FloppyController.obj
$ CXX FloppyController.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_FloppyController.obj
$!
$! Compile Keyboard.cpp to es40_lss_Keyboard.obj
$ CXX Keyboard.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_Keyboard.obj
$!
$! Compile lockstep.cpp to es40_lss_lockstep.obj
$ CXX lockstep.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_lockstep.obj
$!
$! Compile PCIDevice.cpp to es40_lss_PCIDevice.obj
$ CXX PCIDevice.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_PCIDevice.obj
$!
$! Compile Port80.cpp to es40_lss_Port80.obj
$ CXX Port80.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_Port80.obj
$!
$! Compile S3Trio64.cpp to es40_lss_S3Trio64.obj
$ CXX S3Trio64.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_S3Trio64.obj
$!
$! Compile SCSIBus.cpp to es40_lss_SCSIBus.obj
$ CXX SCSIBus.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_SCSIBus.obj
$!
$! Compile SCSIDevice.cpp to es40_lss_SCSIDevice.obj
$ CXX SCSIDevice.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_SCSIDevice.obj
$!
$! Compile Serial.cpp to es40_lss_Serial.obj
$ CXX Serial.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_Serial.obj
$!
$! Compile StdAfx.cpp to es40_lss_StdAfx.obj
$ CXX StdAfx.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_StdAfx.obj
$!
$! Compile Sym53C810.cpp to es40_lss_Sym53C810.obj
$ CXX Sym53C810.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_Sym53C810.obj
$!
$! Compile Sym53C895.cpp to es40_lss_Sym53C895.obj
$ CXX Sym53C895.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_Sym53C895.obj
$!
$! Compile SystemComponent.cpp to es40_lss_SystemComponent.obj
$ CXX SystemComponent.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_SystemComponent.obj
$!
$! Compile System.cpp to es40_lss_System.obj
$ CXX System.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_System.obj
$!
$! Compile TraceEngine.cpp to es40_lss_TraceEngine.obj
$ CXX TraceEngine.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_TraceEngine.obj
$!
$! Compile VGA.cpp to es40_lss_VGA.obj
$ CXX VGA.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_VGA.obj
$!
$! Compile [.gui]gui.cpp to es40_lss_gui.obj
$ CXX [.gui]gui.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_gui.obj
$!
$! Compile [.gui]gui_x11.cpp to es40_lss_gui_x11.obj
$ CXX [.gui]gui_x11.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_gui_x11.obj
$!
$! Compile [.gui]keymap.cpp to es40_lss_keymap.obj
$ CXX [.gui]keymap.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_keymap.obj
$!
$! Compile [.gui]scancodes.cpp to es40_lss_scancodes.obj
$ CXX [.gui]scancodes.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_scancodes.obj
$!
$! Compile [.gui]sdl.cpp to es40_lss_sdl.obj
$ CXX [.gui]sdl.cpp -
       /DEFINE=(ES40,IDB,LSS) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lss_sdl.obj
$!
$! Link es40_lss
$!
$ SAY "Linking es40_lss..."
$!
$ CXXLINK  es40_lss_AliM1543C.obj es40_lss_AliM1543C_ide.obj es40_lss_AliM1543C_usb.obj es40_lss_AlphaCPU.obj es40_lss_AlphaCPU_ieeefloat.obj es40_lss_AlphaCPU_vaxfloat.obj es40_lss_AlphaCPU_vmspal.obj es40_lss_AlphaSim.obj es40_lss_Cirrus.obj es40_lss_Configurator.obj es40_lss_DEC21143.obj es40_lss_Disk.obj es40_lss_DiskController.obj es40_lss_DiskDevice.obj es40_lss_DiskFile.obj es40_lss_DiskRam.obj es40_lss_DMA.obj es40_lss_DPR.obj es40_lss_es40_debug.obj es40_lss_Ethernet.obj es40_lss_Exception.obj es40_lss_Flash.obj es40_lss_FloppyController.obj es40_lss_Keyboard.obj es40_lss_lockstep.obj es40_lss_PCIDevice.obj es40_lss_Port80.obj es40_lss_S3Trio64.obj es40_lss_SCSIBus.obj es40_lss_SCSIDevice.obj es40_lss_Serial.obj es40_lss_StdAfx.obj es40_lss_Sym53C810.obj es40_lss_Sym53C895.obj es40_lss_SystemComponent.obj es40_lss_System.obj es40_lss_TraceEngine.obj es40_lss_VGA.obj es40_lss_gui.obj es40_lss_gui_x11.obj es40_lss_keymap.obj es40_lss_scancodes.obj es40_lss_sdl.obj -
           /EXECUTABLE=es40_lss.exe
$!
$! Compile sources for es40_lsm
$!
$! Compile with the following defines: ES40,IDB,LSM
$!
$ SAY "Compiling es40_lsm..."
$!
$! Compile AliM1543C.cpp to es40_lsm_AliM1543C.obj
$ CXX AliM1543C.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_AliM1543C.obj
$!
$! Compile AliM1543C_ide.cpp to es40_lsm_AliM1543C_ide.obj
$ CXX AliM1543C_ide.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_AliM1543C_ide.obj
$!
$! Compile AliM1543C_usb.cpp to es40_lsm_AliM1543C_usb.obj
$ CXX AliM1543C_usb.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_AliM1543C_usb.obj
$!
$! Compile AlphaCPU.cpp to es40_lsm_AlphaCPU.obj
$ CXX AlphaCPU.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_AlphaCPU.obj
$!
$! Compile AlphaCPU_ieeefloat.cpp to es40_lsm_AlphaCPU_ieeefloat.obj
$ CXX AlphaCPU_ieeefloat.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_AlphaCPU_ieeefloat.obj
$!
$! Compile AlphaCPU_vaxfloat.cpp to es40_lsm_AlphaCPU_vaxfloat.obj
$ CXX AlphaCPU_vaxfloat.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_AlphaCPU_vaxfloat.obj
$!
$! Compile AlphaCPU_vmspal.cpp to es40_lsm_AlphaCPU_vmspal.obj
$ CXX AlphaCPU_vmspal.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_AlphaCPU_vmspal.obj
$!
$! Compile AlphaSim.cpp to es40_lsm_AlphaSim.obj
$ CXX AlphaSim.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_AlphaSim.obj
$!
$! Compile Cirrus.cpp to es40_lsm_Cirrus.obj
$ CXX Cirrus.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_Cirrus.obj
$!
$! Compile Configurator.cpp to es40_lsm_Configurator.obj
$ CXX Configurator.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_Configurator.obj
$!
$! Compile DEC21143.cpp to es40_lsm_DEC21143.obj
$ CXX DEC21143.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_DEC21143.obj
$!
$! Compile Disk.cpp to es40_lsm_Disk.obj
$ CXX Disk.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_Disk.obj
$!
$! Compile DiskController.cpp to es40_lsm_DiskController.obj
$ CXX DiskController.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_DiskController.obj
$!
$! Compile DiskDevice.cpp to es40_lsm_DiskDevice.obj
$ CXX DiskDevice.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_DiskDevice.obj
$!
$! Compile DiskFile.cpp to es40_lsm_DiskFile.obj
$ CXX DiskFile.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_DiskFile.obj
$!
$! Compile DiskRam.cpp to es40_lsm_DiskRam.obj
$ CXX DiskRam.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_DiskRam.obj
$!
$! Compile DMA.cpp to es40_lsm_DMA.obj
$ CXX DMA.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_DMA.obj
$!
$! Compile DPR.cpp to es40_lsm_DPR.obj
$ CXX DPR.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_DPR.obj
$!
$! Compile es40_debug.cpp to es40_lsm_es40_debug.obj
$ CXX es40_debug.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_es40_debug.obj
$!
$! Compile Ethernet.cpp to es40_lsm_Ethernet.obj
$ CXX Ethernet.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_Ethernet.obj
$!
$! Compile Exception.cpp to es40_lsm_Exception.obj
$ CXX Exception.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_Exception.obj
$!
$! Compile Flash.cpp to es40_lsm_Flash.obj
$ CXX Flash.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_Flash.obj
$!
$! Compile FloppyController.cpp to es40_lsm_FloppyController.obj
$ CXX FloppyController.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_FloppyController.obj
$!
$! Compile Keyboard.cpp to es40_lsm_Keyboard.obj
$ CXX Keyboard.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_Keyboard.obj
$!
$! Compile lockstep.cpp to es40_lsm_lockstep.obj
$ CXX lockstep.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_lockstep.obj
$!
$! Compile PCIDevice.cpp to es40_lsm_PCIDevice.obj
$ CXX PCIDevice.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_PCIDevice.obj
$!
$! Compile Port80.cpp to es40_lsm_Port80.obj
$ CXX Port80.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_Port80.obj
$!
$! Compile S3Trio64.cpp to es40_lsm_S3Trio64.obj
$ CXX S3Trio64.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_S3Trio64.obj
$!
$! Compile SCSIBus.cpp to es40_lsm_SCSIBus.obj
$ CXX SCSIBus.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_SCSIBus.obj
$!
$! Compile SCSIDevice.cpp to es40_lsm_SCSIDevice.obj
$ CXX SCSIDevice.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_SCSIDevice.obj
$!
$! Compile Serial.cpp to es40_lsm_Serial.obj
$ CXX Serial.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_Serial.obj
$!
$! Compile StdAfx.cpp to es40_lsm_StdAfx.obj
$ CXX StdAfx.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_StdAfx.obj
$!
$! Compile Sym53C810.cpp to es40_lsm_Sym53C810.obj
$ CXX Sym53C810.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_Sym53C810.obj
$!
$! Compile Sym53C895.cpp to es40_lsm_Sym53C895.obj
$ CXX Sym53C895.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_Sym53C895.obj
$!
$! Compile SystemComponent.cpp to es40_lsm_SystemComponent.obj
$ CXX SystemComponent.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_SystemComponent.obj
$!
$! Compile System.cpp to es40_lsm_System.obj
$ CXX System.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_System.obj
$!
$! Compile TraceEngine.cpp to es40_lsm_TraceEngine.obj
$ CXX TraceEngine.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_TraceEngine.obj
$!
$! Compile VGA.cpp to es40_lsm_VGA.obj
$ CXX VGA.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_VGA.obj
$!
$! Compile [.gui]gui.cpp to es40_lsm_gui.obj
$ CXX [.gui]gui.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_gui.obj
$!
$! Compile [.gui]gui_x11.cpp to es40_lsm_gui_x11.obj
$ CXX [.gui]gui_x11.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_gui_x11.obj
$!
$! Compile [.gui]keymap.cpp to es40_lsm_keymap.obj
$ CXX [.gui]keymap.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_keymap.obj
$!
$! Compile [.gui]scancodes.cpp to es40_lsm_scancodes.obj
$ CXX [.gui]scancodes.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_scancodes.obj
$!
$! Compile [.gui]sdl.cpp to es40_lsm_sdl.obj
$ CXX [.gui]sdl.cpp -
       /DEFINE=(ES40,IDB,LSM) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_lsm_sdl.obj
$!
$! Link es40_lsm
$!
$ SAY "Linking es40_lsm..."
$!
$ CXXLINK  es40_lsm_AliM1543C.obj es40_lsm_AliM1543C_ide.obj es40_lsm_AliM1543C_usb.obj es40_lsm_AlphaCPU.obj es40_lsm_AlphaCPU_ieeefloat.obj es40_lsm_AlphaCPU_vaxfloat.obj es40_lsm_AlphaCPU_vmspal.obj es40_lsm_AlphaSim.obj es40_lsm_Cirrus.obj es40_lsm_Configurator.obj es40_lsm_DEC21143.obj es40_lsm_Disk.obj es40_lsm_DiskController.obj es40_lsm_DiskDevice.obj es40_lsm_DiskFile.obj es40_lsm_DiskRam.obj es40_lsm_DMA.obj es40_lsm_DPR.obj es40_lsm_es40_debug.obj es40_lsm_Ethernet.obj es40_lsm_Exception.obj es40_lsm_Flash.obj es40_lsm_FloppyController.obj es40_lsm_Keyboard.obj es40_lsm_lockstep.obj es40_lsm_PCIDevice.obj es40_lsm_Port80.obj es40_lsm_S3Trio64.obj es40_lsm_SCSIBus.obj es40_lsm_SCSIDevice.obj es40_lsm_Serial.obj es40_lsm_StdAfx.obj es40_lsm_Sym53C810.obj es40_lsm_Sym53C895.obj es40_lsm_SystemComponent.obj es40_lsm_System.obj es40_lsm_TraceEngine.obj es40_lsm_VGA.obj es40_lsm_gui.obj es40_lsm_gui_x11.obj es40_lsm_keymap.obj es40_lsm_scancodes.obj es40_lsm_sdl.obj -
           /EXECUTABLE=es40_lsm.exe
$!
$! Compile sources for es40_cfg
$!
$! Compile with the following defines: ES40
$!
$ SAY "Compiling es40_cfg..."
$!
$! Compile es40-cfg.cpp to es40_cfg_es40-cfg.obj
$ CXX es40-cfg.cpp -
       /DEFINE=(ES40) -
       /INCLUDE=([]) -
       /STANDARD=GNU -
       /ARCHITECTURE=HOST -
       /OPTIMIZE=(LEVEL=4,INLINE=SPEED,TUNE=HOST) -
       /OUTPUT=es40_cfg_es40-cfg.obj
$!
$! Link es40_cfg
$!
$ SAY "Linking es40_cfg..."
$!
$ CXXLINK  es40_cfg_es40-cfg.obj -
           /EXECUTABLE=es40_cfg.exe
$!
$ SAY "That's all, folks!"
$!
$ EXIT

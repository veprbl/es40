################################################################################
# ES40 emulator.
# Copyright (C) 2007-2008 by the ES40 Emulator Project
#
# Website: http://sourceforge.net/projects/es40
# E-mail : camiel@camicom.com
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
# 02110-1301, USA.
# 
# Although this is not required, the author would appreciate being notified of, 
# and receiving any modifications you may make to the source code that might serve
# the general public.
#
################################################################################
#
# $Id: make_vms.sh,v 1.1 2008/03/30 15:53:19 iamcamiel Exp $
#
# X-1.1	     Camiel Vanderhoeven                      20-MAR-2008
#      File Created.
#
################################################################################

es40_SOURCES="AliM1543C.cpp AliM1543C_ide.cpp AliM1543C_usb.cpp AlphaCPU.cpp AlphaCPU_ieeefloat.cpp AlphaCPU_vaxfloat.cpp AlphaCPU_vmspal.cpp AlphaSim.cpp Cirrus.cpp Configurator.cpp  DEC21143.cpp Disk.cpp DiskController.cpp DiskDevice.cpp DiskFile.cpp DiskRam.cpp DMA.cpp DPR.cpp es40_debug.cpp Ethernet.cpp Exception.cpp Flash.cpp FloppyController.cpp Keyboard.cpp lockstep.cpp PCIDevice.cpp Port80.cpp S3Trio64.cpp SCSIBus.cpp SCSIDevice.cpp Serial.cpp StdAfx.cpp Sym53C810.cpp Sym53C895.cpp SystemComponent.cpp System.cpp TraceEngine.cpp VGA.cpp gui/gui.cpp gui/gui_x11.cpp gui/keymap.cpp gui/scancodes.cpp gui/sdl.cpp"

es40_CONFIGS="es40 es40_idb es40_lss es40_lsm es40_cfg"

cat > make_vms.com << VMS_EOF
\$ SET VERIFY
\$!
\$! ES40 Emulator
\$! Copyright (C) 2007-2008 by the ES40 Emulator Project
\$!
\$! This file was created by make_vms.sh. Please refer to that file
\$! for more information.
\$!
\$ SAY = "WRITE SYS\$OUTPUT"
VMS_EOF
for current_CONFIG in $es40_CONFIGS; do

  es40_DEFINES=ES40
  es40_INCLUDE="[]"
  es40_OPTIMIZE="LEVEL=4,INLINE=SPEED,TUNE=HOST"
  es40_ARCH="HOST"
  es40_STANDARD="GNU"

  if test "$current_CONFIG" = "es40_idb"; then
    es40_DEFINES=$es40_DEFINES,IDB
  elif test "$current_CONFIG" = "es40_lss"; then
    es40_DEFINES=$es40_DEFINES,IDB,LSS
  elif test "$current_CONFIG" = "es40_lsm"; then
    es40_DEFINES=$es40_DEFINES,IDB,LSM
  elif test "$current_CONFIG" = "es40_cfg"; then
    es40_SOURCES="es40-cfg.cpp"
  fi

  cat >> make_vms.com << VMS_EOF
\$!
\$! Compile sources for $current_CONFIG
\$!
\$! Compile with the following defines: $es40_DEFINES
\$!
\$ SAY "Compiling $current_CONFIG..."
VMS_EOF

  es40_OBJECTS=""
  for source_FILE in $es40_SOURCES; do
    object_FILE=${source_FILE#gui/}
    object_FILE=${object_FILE%.cpp}.obj
    object_FILE=${current_CONFIG}_${object_FILE}

    if test "${source_FILE:0:4}" = "gui/"; then
      source_FILE=${source_FILE#gui/}
      source_FILE="[.gui]$source_FILE"
    fi
    cat >> make_vms.com << VMS_EOF
\$!
\$! Compile $source_FILE to $object_FILE
\$ CXX $source_FILE -
       /DEFINE=($es40_DEFINES) -
       /INCLUDE=($es40_INCLUDE) -
       /STANDARD=$es40_STANDARD -
       /ARCHITECTURE=$es40_ARCH -
       /OPTIMIZE=($es40_OPTIMIZE) -
       /OUTPUT=$object_FILE
VMS_EOF

    es40_OBJECTS="$es40_OBJECTS $object_FILE"
  done

  es40_OUTPUT=${current_CONFIG}.exe

  cat >> make_vms.com << VMS_EOF
\$!
\$! Link $current_CONFIG
\$!
\$ SAY "Linking $current_CONFIG..."
\$!
\$ CXXLINK $es40_OBJECTS -
           /EXECUTABLE=$es40_OUTPUT
VMS_EOF

done

cat >> make_vms.com << VMS_EOF
\$!
\$ SAY "That's all, folks!"
\$!
\$ EXIT
VMS_EOF



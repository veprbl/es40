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
# $Id: Makefile.am,v 1.1 2008/03/20 07:49:51 iamcamiel Exp $
#
# X-1.1	     Camiel Vanderhoeven                      20-MAR-2008
#      File Created.
#
################################################################################

bin_PROGRAMS = es40 es40_idb es40_lss es40_lsm

es40_SOURCES = AliM1543C.cpp \
       AliM1543C_ide.cpp \
       AliM1543C_usb.cpp \
       AlphaCPU.cpp \
       AlphaCPU_ieeefloat.cpp \
       AlphaCPU_vaxfloat.cpp \
       AlphaCPU_vmspal.cpp \
       AlphaSim.cpp \
       Cirrus.cpp \
       Configurator.cpp \
       DEC21143.cpp \
       Disk.cpp \
       DiskController.cpp \
       DiskDevice.cpp \
       DiskFile.cpp \
       DiskRam.cpp \
       DMA.cpp \
       DPR.cpp \
       es40_debug.cpp \
       Ethernet.cpp \
       Exception.cpp \
       Flash.cpp \
       FloppyController.cpp \
       Keyboard.cpp \
       lockstep.cpp \
       PCIDevice.cpp \
       Port80.cpp \
       S3Trio64.cpp \
       SCSIBus.cpp \
       SCSIDevice.cpp \
       Serial.cpp \
       StdAfx.cpp \
       Sym53C810.cpp \
       Sym53C895.cpp \
       SystemComponent.cpp \
       System.cpp \
       TraceEngine.cpp \
       VGA.cpp \
       gui/gui.cpp \
       gui/gui_x11.cpp \
       gui/keymap.cpp \
       gui/scancodes.cpp \
       gui/sdl.cpp

es40_idb_SOURCES = $(es40_SOURCES)
es40_lss_SOURCES = $(es40_SOURCES)
es40_lsm_SOURCES = $(es40_SOURCES)

es40_idb_CXXFLAGS = "-DIDB"
es40_lss_CXXFLAGS = "-DIDB -DLSS"
es40_lsm_CXXFLAGS = "-DIDB -DLSM"

LDADD = -lPocoFoundation

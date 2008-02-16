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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# 
# Although this is not required, the author would appreciate being notified of, 
# and receiving any modifications you may make to the source code that might serve
# the general public.
#
################################################################################
#
# Makefile for GNU Linux. (And possibly many UNIX'es).
#
# $Id: Makefile,v 1.23 2008/02/16 13:00:15 iamcamiel Exp $
#
# X-1.22        Camiel Vanderhoeven             16-FEB-2008
#      Added Symbios 53C810 controller.
#
# X-1.21		Brian Wheeler      				07-FEB-2008
#	Add DEBUG_BACKTRACE flag.
#
# X-1.20		Brian Wheeler      				02-FEB-2008
#	Add DEBUG_LPT flag.
#
# X-1.19		Brian Wheeler      				29-JAN-2008
#	More information about available flags, and make depend
#   works now.
#
# X-1.18        Camiel Vanderhoeven             26-JAN-2008
#   Added new floating point cpp's.
#
# X-1.17        Camiel Vanderhoeven             20-JAN-2008
#   Added X11 GUI.
#
# X-1.16		Brian Wheeler      				12-JAN-2008
#	Added SCSIDevice and SCSIDisk.
#
# X-1.15		Brian Wheeler      				08-JAN-2008
#	Added NewIde and DiskDevice.
#
# X-1.14		Camiel Vanderhoeven				02-JAN-2008
#	Brought up-to-date.
#
# X-1.13		Brian Wheeler      				16-DEC-2007
#	Added SCSI support.
#
# X-1.11		Camiel Vanderhoeven				18-NOV-2007
#	Added linker option to include libpcap.
#
# X-1.10		Camiel Vanderhoeven				17-NOV-2007
#	Removed Network.o (no longer needed).
#
# X-1.9		Camiel Vanderhoeven				14-NOV-2007
#	Added files for network support.
#
# X-1.8		Camiel Vanderhoeven				14-APR-2007
#	Added forgotten es40_lss and es40_lsm to make clean.
#
# X-1.7		Camiel Vanderhoeven				1-APR-2007
#	Added old changelog comments.
#
# X-1.6		Camiel Vanderhoeven				28-FEB-2007
#	Added lockstep targets (es40_lss and es40_lsm).
#
# X-1.5		Brian Wheeler					16-FEB-2007
#	Added Interactive Debugger target (es40_idb).
#
# X-1.4		Brian Wheeler					13-FEB-207
#	Added Doxygen target (doc).
#
# X-1.3		Brian Wheeler					3-FEB-2007
#	Make clean now also removes old trace files.
#
# X-1.2		Brian wheeler					3-FEB-2007
#	Added Serial.o.
#
# X-1.1		Brian Wheeler					3-FEB-2007
#	Created this file for the Linux port.
#
# \author Brian Wheeler
#
################################################################################

# use G++
CC = g++
LD = g++

#
# CTUNINGFLAGS - tuning options for the compiler
#
# Samples for g++ on x86:
#  -O3   -- optimize at level 3
#  -mtune=<cpu>   -- cpu is one of:  generic, core2, athlon64, pentium4, etc
#
CTUNINGFLAGS = -O3 -mtune=generic 

#
# CDEBUGFLAGS - turn on debugging in ES40
#
# Supported flags:
#   -g  		        Include information for gdb
#   -DHIDE_COUNTER	    Do not show the cycle counter
#   -DDEBUG_VGA		    Turn on VGA Debugging
#   -DDEBUG_SERIAL	    Turn on Serial Debugging
#   -DDEBUG_IDE		    Turn on all IDE Debugging.
#   -DDEBUG_IDE_<opt>	Turn on specific IDE debugging.  Options are:
#	            		BUSMASTER, COMMAND, DMA, INTERRUPT, REG_COMMAND,
#           			REG_CONTROL, PACKET
#   -DDEBUG_UNKMEM	    Turn on unknown memory access debugging
#   -DDEBUG_PCI		    Turn on PCI Debugging
#   -DDEBUG_TB		    Turn on Translation Buffer debugging
#   -DDEBUG_PORTACCESS	Turn on i/o port access debugging
#   -DDEBUG_SCSI	    Turn on SCSI debugging
#   -DDEBUG_KBD		    Turn on keyboard debugging
#   -DDEBUG_PIC		    Turn on Programmable Interrupt Controller debugging
#   -DDEBUG_LPT         Turn on debugging for LPT Port
#   -DDEBUG_BACKTRACE   Turn on backtrace dump on SIGSEGV
#
CDEBUGFLAGS = -g -DHIDE_COUNTER -DDEBUG_BACKTRACE

#
# ES40 Options
#
# -DHAVE_SDL 		Use the SDL Library for GUI
# -DHAVE_X11		Use X11 for GUI
# -DHAVE_PCAP		Use Networking via PCAP
# -DHAVE_NEW_IDE	Use the 'new' IDE Driver
# -DHAVE_NEW_FP     Use the new floating-point code (with traps,
#                   ut unfortunately, also with bugs)
#
OPTIONS = -DHAVE_SDL -DHAVE_X11 -DHAVE_PCAP -DHAVE_NEW_IDE


####################################
# No user servicable parts inside.
####################################

ifneq ($(findstring "-DHAVE_SDL",$(OPTIONS)),"")
COPTIONFLAGS += -I/usr/include/SDL
LDOPTIONFLAGS += -lSDL
endif

ifneq ($(findstring "-DHAVE_X11",$(OPTIONS)),"")
LDOPTIONFLAGS += -lX11
endif

ifneq ($(findstring "-DHAVE_PCAP",$(OPTIONS)),"")
LDOPTIONFLAGS += -lpcap
endif


CFLAGS =  -I. -I..  \
	$(CDEBUGFLAGS) $(CTUNINGFLAGS) $(OPTIONS) $(COPTIONFLAGS)
LDFLAGS = $(LDOPTIONFLAGS)

IDB_CFLAGS = -DIDB 
LSM_CFLAGS = -DIDB -DLS_MASTER
LSS_CFLAGS = -DIDB -DLS_SLAVE


DEPEND = makedepend $(CFLAGS)

OBJS = AliM1543C.o \
       AliM1543C_ide.o \
       AliM1543C_usb.o \
       AlphaCPU.o \
       AlphaCPU_ieeefloat.o \
       AlphaCPU_vaxfloat.o \
       AlphaCPU_vmspal.o \
       AlphaSim.o \
       Cirrus.o \
       Configurator.o \
       DEC21143.o \
       Disk.o \
       DiskController.o \
       DiskDevice.o \
       DiskFile.o \
       DiskRam.o \
       DPR.o \
       es40_debug.o \
       Flash.o \
       FloppyController.o \
       lockstep.o \
       NewIde.o \
       PCIDevice.o \
       Port80.o \
       S3Trio64.o \
       SCSIBus.o \
       SCSIDevice.o \
       Serial.o \
       StdAfx.o \
       Sym53C895.o \
       SystemComponent.o \
       System.o \
       TraceEngine.o \
       VGA.o \
       gui/gui.o \
       gui/gui_x11.o \
       gui/keymap.o \
       gui/scancodes.o \
       gui/sdl.o

IDB_OBJS = $(subst .o,.do,$(OBJS))
LSM_OBJS = $(subst .o,.mao,$(OBJS))
LSS_OBJS = $(subst .o,.slo,$(OBJS))
SRCS = $(subst .o,.cpp,$(OBJS))

.SUFFIXES: .do .mao .slo

.cpp.o:
	$(CC) $(CFLAGS) -c $<  -o $@

.cpp.do:
	$(CC) $(CFLAGS) $(IDB_CFLAGS) -c $< -o $@

.cpp.mao:
	$(CC) $(CFLAGS) $(LSM_CFLAGS) -c $< -o $@

.cpp.slo:
	$(CC) $(CFLAGS) $(LSS_CFLAGS) -c $< -o $@

all: es40

debug: es40_idb es40_lsm es40_lss

es40:   $(OBJS)
	$(LD) $(LDFLAGS) -o es40 $(OBJS)

es40_idb: $(IDB_OBJS)
	$(LD) $(LDFLAGS) -o es40_idb $(IDB_OBJS)

es40_lsm: $(LSM_OBJS)
	$(LD) $(LDFLAGS) -o es40_lsm $(LSM_OBJS)

es40_lss: $(LSS_OBJS)
	$(LD) $(LDFLAGS) -o es40_lss $(LSS_OBJS)

depend: $(SRCS)
	$(DEPEND) $(SRCS)
	$(DEPEND) -o.do -a -- $(IDB_CFLAGS) -- $(SRCS)
clean:
	rm -f es40 es40_idb es40_lss es40_lsm *.o *.do *.mao *.slo *.trc gui/*.o gui/*.mao gui/*.slo gui/*.do


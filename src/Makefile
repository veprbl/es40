################################################################################
# ES40 emulator.
# Copyright (C) 2007 by Camiel Vanderhoeven
#
# Website: www.camicom.com
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

#to use G++
CC = g++
LDFLAGS =

#to use Intel's ICC
# CC = /opt/intel/cc/9.1.046/bin/icc
#LDFLAGS = -L/opt/intel/cc/9.1.046/lib -lirc

CFLAGS = -O2 -g -I.
IDB_CFLAGS = -DIDB 
LSM_CFLAGS = -DIDB -DLS_MASTER
LSS_CFLAGS = -DIDB -DLS_SLAVE
LD = g++

DEPEND = makedepend $(CFLAGS)

OBJS =  AliM1543C.o \
        AlphaCPU.o \
        AlphaSim.o \
        DPR.o \
        Flash.o \
        FloppyController.o \
        Port80.o \
	  Serial.o \
        StdAfx.o \
        SystemComponent.o \
        System.o \
        TraceEngine.o \
        TranslationBuffer.o \
	  lockstep.o \
        DEC21143.o \
        es40_debug.o

IDB_OBJS = $(subst .o,.do,$(OBJS))
LSM_OBJS = $(subst .o,.mao,$(OBJS))
LSS_OBJS = $(subst .o,.slo,$(OBJS))

.SUFFIXES: .do .mao .slo

.cpp.o:
	$(CC) $(CFLAGS) -c $< 

.cpp.do:
	$(CC) $(CFLAGS) $(IDB_CFLAGS) -c $< -o $@

.cpp.mao:
	$(CC) $(CFLAGS) $(LSM_CFLAGS) -c $< -o $@

.cpp.slo:
	$(CC) $(CFLAGS) $(LSS_CFLAGS) -c $< -o $@

all: es40 es40_idb es40_lsm es40_lss

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
	rm -f es40 es40_idb es40_lss es40_lsm *.o *.do *.mao *.slo *.trc


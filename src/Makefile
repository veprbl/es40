#to use G++
CC = g++
LDFLAGS =

#to use Intel's ICC
# CC = /opt/intel/cc/9.1.046/bin/icc
#LDFLAGS = -L/opt/intel/cc/9.1.046/lib -lirc

CFLAGS = -O2 -g -I.
IDB_CFLAGS = -DIDB 
LD = g++

DEPEND = makedepend $(CFLAGS)

OBJS = AliM1543C.o \
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
        TranslationBuffer.o 

IDB_OBJS = $(subst .o,.do,$(OBJS))

.SUFFIXES: .do

.cpp.o:
	$(CC) $(CFLAGS) -c $< 

.cpp.do:
	$(CC) $(CFLAGS) $(IDB_CFLAGS) -c $< -o $@

all: es40 es40_idb

es40:   $(OBJS)
	$(LD) $(LDFLAGS) -o es40 $(OBJS)

es40_idb: $(IDB_OBJS)
	$(LD) $(LDFLAGS) -o es40_idb $(IDB_OBJS)

depend: $(SRCS)
	$(DEPEND) $(SRCS)
	$(DEPEND) -o.do -a -- $(IDB_CFLAGS) -- $(SRCS)
clean:
	rm -f es40 es40_idb *.o *.do *.trc


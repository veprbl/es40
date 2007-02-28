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
        TranslationBuffer.o \
	lockstep.o

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
	rm -f es40 es40_idb *.o *.do *.mao *.slo *.trc


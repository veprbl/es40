CC = g++
CFLAGS = -O2 -g -I. 
LD = g++
LDFLAGS =


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

.cpp.o:
	$(CC) $(CFLAGS) -c $< 

all: $(OBJS)
	$(LD) $(LDFLAGS) -o es40 $(OBJS)

clean:
	rm -f es40 *.o *.trc


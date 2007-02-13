CC = g++
#CC = /opt/intel/cc/9.1.046/bin/icc
#CFLAGS =  -g -I. -Wall -DIDB -DMINI_DEBUG_XX
CFLAGS =  -O2 -I.
LD = g++
#LDFLAGS =  -L/opt/intel/cc/9.1.046/lib -lirc
LDFLAGS = 
DOXYGEN = doxygen
DEPEND = makedepend $(CFLAGS)

SRCS = $(shell echo *.cpp)

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

all: es40



es40: $(OBJS)
	$(LD) $(LDFLAGS) -o es40 $(OBJS)

doc: Doxyfile $(SRCS)
	if [ ! -e docs ]; then  mkdir docs ; fi
	$(DOXYGEN)

Doxyfile:
	$(DOXYGEN) -g > /dev/null
	echo "PROJECT_NAME=ES40" >> Doxyfile
	echo "OUTPUT_DIRECTORY=docs" >> Doxyfile

depend: $(SRCS)
	$(DEPEND) $(SRCS)

clean:
	rm -rf es40 *.o *.trc Doxyfile docs


cpufeature = $(if $(findstring $(1),$(shell cat /proc/cpuinfo)),$(2))
PARAMS_SSE = $(call cpufeature,sse,-msse) $(call cpufeature,sse2,-msse2) $(call cpufeature,sse3,-msse3) $(call cpufeature,sse4a,-msse4a) $(call cpufeature,sse4_1,-msse4.1) $(call cpufeature,sse4_2,-msse4.2 -msse4) -mfpmath=sse 
PARAMS_NEON = -mfloat-abi=hard -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mvectorize-with-neon-quad -funsafe-math-optimizations -Wformat=0 -DNEON_OPTS
#tnx Jan Szumiec for the Raspberry Pi support
PARAMS_RASPI = -mfloat-abi=hard -mcpu=arm1176jzf-s -mfpu=vfp -funsafe-math-optimizations -Wformat=0
PARAMS_ARM = $(if $(call cpufeature,BCM2708,dummy-text),$(PARAMS_RASPI),$(PARAMS_NEON))
PARAMS_SIMD = $(if $(call cpufeature,sse,dummy-text),$(PARAMS_SSE),$(PARAMS_ARM))
PARAMS_LOOPVECT = -O3 -ffast-math -fdump-tree-vect-details -dumpbase dumpvect
PARAMS_LIBS = -g -lm -lrt -lfftw3f -DUSE_FFTW -DLIBCSDR_GPL -DUSE_IMA_ADPCM
PARAMS_SO = -fpic  
PARAMS_MISC = -Wno-unused-result
#DEBUG_ON = 0 #debug is always on by now (anyway it could be compiled with `make DEBUG_ON=1`)
#PARAMS_DEBUG = $(if $(DEBUG_ON),-g,)
FFTW_PACKAGE = fftw-3.3.3

CC=gcc
#CFLAGS= -O0 -c -Wall -DLE_MACHINE $(PARMS_LOOPVECT) $(PARMS_SIMD) $(PARAMS_LIBS)
CFLAGS= -O0 -c -Wall -DLE_MACHINE 
LDFLAGS=

SOURCES=$ dspp.c convert_byteLE_int16.c convert_aByte_f.c convert_aUnsignedByte_f.c
OBJECTS=$(SOURCES:.c=.o)
#LIBS=$ -lX11
DEPTS=$ 

EXECUTABLE=dspp

all: $(DEPTS) $(SOURCES) $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o dspp -lm $(LIBS)
$(SOURCES):
	$(CC) $(CFLAGS) $< -lm -o $@

test:
	$(CC) $(CFLAGS) testIn1.c -o testIn1.o
	$(CC) $(CFLAGS) testOut1.c -o testOut1.o
	$(CC) $(LDFLAGS) testIn1.o -o testIn1 -lm $(LIBS)
	$(CC) $(LDFLAGS) testOut1.o -o testOut1 -lm $(LIBS)
	$(CC) $(CFLAGS) testIn2.c -o testIn2.o
	$(CC) $(CFLAGS) testOut2.c -o testOut2.o
	$(CC) $(CFLAGS) findDiff.c -o findDiff.o
	$(CC) $(LDFLAGS) testIn2.o -o testIn2 -lm $(LIBS)
	$(CC) $(LDFLAGS) testOut2.o -o testOut2 -lm $(LIBS)
	$(CC) $(LDFLAGS) findDiff.o -o findDiff -lm $(LIBS)
clean:
	rm -fr $(OBJECTS) $(EXECUTABLE)

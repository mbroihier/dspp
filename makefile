cpufeature = $(if $(findstring $(1),$(shell cat /proc/cpuinfo)),$(2))
PARAMS_SSE = $(call cpufeature,sse,-msse) $(call cpufeature,sse2,-msse2) $(call cpufeature,sse3,-msse3) $(call cpufeature,sse4a,-msse4a) $(call cpufeature,sse4_1,-msse4.1) $(call cpufeature,sse4_2,-msse4.2 -msse4) -mfpmath=sse 
PARAMS_NEON = -mfloat-abi=hard -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mvectorize-with-neon-quad -funsafe-math-optimizations -Wformat=0 -DNEON_OPTS
#tnx Jan Szumiec for the Raspberry Pi support
PARAMS_RASPI = -mfloat-abi=hard -mcpu=arm1176jzf-s -mfpu=vfp -funsafe-math-optimizations -Wformat=0
PARAMS_ARM = $(if $(call cpufeature,BCM2708,dummy-text),$(PARAMS_RASPI),$(PARAMS_NEON))
PARAMS_SIMD = $(if $(call cpufeature,sse,dummy-text),$(PARAMS_SSE),$(PARAMS_ARM))
PARAMS_LOOPVECT = -O3 -ffast-math -fdump-tree-vect-details -dumpbase dumpvect
#PARAMS_LIBS = -g -lm -lrt -lfftw3f -lstdc++ -DUSE_FFTW -DLIBCSDR_GPL -DUSE_IMA_ADPCM
PARAMS_LIBS = -g -lm -lstdc++ 
PARAMS_SO = -fpic  
PARAMS_MISC = -Wno-unused-result
#DEBUG_ON = 0 #debug is always on by now (anyway it could be compiled with `make DEBUG_ON=1`)
#PARAMS_DEBUG = $(if $(DEBUG_ON),-g,)
FFTW_PACKAGE = fftw-3.3.3

CC=gcc
CFLAGS= -O0 -c -Wall -DLE_MACHINE -D_GNU_SOURCE $(PARAMS_LOOPVECT) $(PARAMS_SIMD) $(PARAMS_MISC)
CXX = $(CC)
CXXFLAGS = $(CFLAGS) # set these flags for use of suffix rules for cc
LDFLAGS= $(PARAMS_LIBS)

SOURCES= dspp.cc convert_byteLE_int16.cc convert_aByte_f.cc convert_aUnsignedByte_f.cc shift_frequency_cc.cc decimate_cc.cc fmdemod_cf.cc decimate_ff.cc convert_f_unsignedShort.cc convert_f_signedShort.cc convert_tcp_aUnsignedByte.cc
OBJECTS=$(SOURCES:.cc=.o)

RTLTCPSRC = RTLTCPClient.cc RTLTCPClient.h
FIRFILTSRC = FIRFilter.cc FIRFilter.h
RTLTCPOBJ = RTLTCPClient.o
FIRFILTOBJ = FIRFilter.o

EXECUTABLE=dspp

all: $(EXECUTABLE)

test:
	$(CC) $(CFLAGS) testIn1.cc -lm -o testIn1.o
	$(CC) $(CFLAGS) testOut1.cc -o testOut1.o
	$(CC) $(LDFLAGS) testIn1.o -o testIn1 -lm 
	$(CC) $(LDFLAGS) testOut1.o -o testOut1 -lm
	$(CC) $(CFLAGS) testIn2.cc -o testIn2.o
	$(CC) $(CFLAGS) testOut2.cc -o testOut2.o
	$(CC) $(LDFLAGS) testIn2.o -o testIn2 -lm
	$(CC) $(LDFLAGS) testOut2.o -o testOut2 -lm
	$(CC) $(CFLAGS) testIn5.cc -o testIn5.o
	$(CC) $(CFLAGS) testIn5a.cc -o testIn5a.o
	$(CC) $(CFLAGS) testOut5.cc -o testOut5.o
	$(CC) $(LDFLAGS) testIn5.o -o testIn5 -lm
	$(CC) $(LDFLAGS) testIn5a.o -o testIn5a -lm
	$(CC) $(LDFLAGS) testOut5.o -o testOut5 -lm
	$(CC) $(CFLAGS) findDiff.cc -o findDiff.o
	$(CC) $(LDFLAGS) findDiff.o -o findDiff -lm
	$(CC) $(CFLAGS) testIn5b.cc -o testIn5b.o
	$(CC) $(CFLAGS) testIn5c.cc -o testIn5c.o
	$(CC) $(CFLAGS) testIn6.cc -o testIn6.o
	$(CC) $(LDFLAGS) testIn5b.o -o testIn5b -lm 
	$(CC) $(LDFLAGS) testIn5c.o -o testIn5c -lm 
	$(CC) $(LDFLAGS) testIn6.o -o testIn6 -lm 

$(EXECUTABLE): $(SOURCES) $(OBJECTS) $(FIRFILTOBJ) $(RTLTCPOBJ)
	$(CC) $(LDFLAGS) $(OBJECTS) $(FIRFILTOBJ) $(RTLTCPOBJ) -o dspp

$(RTLTCPOBJ) : $(RTLTCPSRC)
	$(CC) $(CFLAGS) $*.cc -o $@
$(FIRFILTOBJ) : $(FIRFILTSRC)
	$(CC) $(CFLAGS) $*.cc -o $@

clean:
	rm -fr $(OBJECTS) $(EXECUTABLE) *.o

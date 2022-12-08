cpufeature = $(if $(findstring $(1),$(shell cat /proc/cpuinfo)),$(2))
PARAMS_SSE = $(call cpufeature,sse,-msse) $(call cpufeature,sse2,-msse2) $(call cpufeature,sse3,-msse3) $(call cpufeature,sse4a,-msse4a) $(call cpufeature,sse4_1,-msse4.1) $(call cpufeature,sse4_2,-msse4.2 -msse4) -mfpmath=sse 
PARAMS_NEON = -mfloat-abi=hard -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mvectorize-with-neon-quad -funsafe-math-optimizations -Wformat=0 -DNEON_OPTS
#tnx Jan Szumiec for the Raspberry Pi support
PARAMS_RASPI = -mfloat-abi=hard -mcpu=arm1176jzf-s -mfpu=vfp -funsafe-math-optimizations -Wformat=0
PARAMS_ARM = $(if $(call cpufeature,BCM2708,dummy-text),$(PARAMS_RASPI),$(PARAMS_NEON))
PARAMS_SIMD = $(if $(call cpufeature,sse,dummy-text),$(PARAMS_SSE),$(PARAMS_ARM))
PARAMS_LOOPVECT = -O3 -ffast-math -fdump-tree-vect-details -dumpbase dumpvect
#PARAMS_LIBS = -g -lm -lrt -lfftw3f -lstdc++ -DUSE_FFTW -DLIBCSDR_GPL -DUSE_IMA_ADPCM
PARAMS_LIBS = -g -lm -lstdc++ -lfftw3 
PARAMS_SO = -fpic  
PARAMS_MISC = -Wno-unused-result
#DEBUG_ON = 0 #debug is always on by now (anyway it could be compiled with `make DEBUG_ON=1`)
#PARAMS_DEBUG = $(if $(DEBUG_ON),-g,)
FFTW_PACKAGE = fftw-3.3.3

CC=gcc

CFLAGS= $(if $(shell uname -a | grep -i armv), -c -Wall -DLE_MACHINE -D_GNU_SOURCE $(PARAMS_LOOPVECT) $(PARAMS_SIMD) $(PARAMS_MISC), -c -Wall -DLE_MACHINE -D_GNU_SOURCE )
CXX = $(CC)
CXXFLAGS = $(CFLAGS) # set these flags for use of suffix rules for cc
LDFLAGS= $(PARAMS_LIBS)

#SOURCES= dspp.cc convert_byteLE_int16.cc convert_aByte_f.cc convert_aUnsignedByte_f.cc shift_frequency_cc.cc decimate_cc.cc fmdemod_cf.cc decimate_ff.cc convert_f_unsignedShort.cc convert_f_signedShort.cc convert_tcp_aUnsignedByte.cc convert_byte_tcp.cc
SOURCES= dspp.cc 
OBJECTS=$(SOURCES:.cc=.o)

FNLFSRC = FindNLargestF.cc FindNLargestF.h
AGCSRC = AGC.cc AGC.h
RTLTCPSRC = RTLTCPClient.cc RTLTCPClient.h RTLTCPServer.cc RTLTCPServer.h
FIRFILTSRC = FIRFilter.cc FIRFilter.h SFIRFilter.cc SFIRFilter.h CFilter.cc CFilter.h Poly.cc Poly.h
MODSRC = FMMod.cc FMMod.h
FFTSRC = DsppFFT.cc DsppFFT.h
FNLFOBJ = FindNLargestF.o
AGCOBJ = AGC.o
RTLTCPOBJ = RTLTCPClient.o RTLTCPServer.o
FIRFILTOBJ = FIRFilter.o SFIRFilter.o CFilter.o Poly.o
MODOBJ = FMMod.o
FFTOBJ = DsppFFT.o

EXECUTABLE=dspp

all: $(EXECUTABLE)

test:
	$(CC) $(CFLAGS) testIn1.cc -lm -o testIn1.o
	$(CC) $(LDFLAGS) testIn1.o -o testIn1 -lm 
	$(CC) $(CFLAGS) testIn2.cc -o testIn2.o
	$(CC) $(LDFLAGS) testIn2.o -o testIn2 -lm
	$(CC) $(CFLAGS) testIn5.cc -o testIn5.o
	$(CC) $(CFLAGS) testIn5a.cc -o testIn5a.o
	$(CC) $(LDFLAGS) testIn5.o -o testIn5 -lm
	$(CC) $(LDFLAGS) testIn5a.o -o testIn5a -lm
	$(CC) $(CFLAGS) testIn5b.cc -o testIn5b.o
	$(CC) $(LDFLAGS) testIn5b.o -o testIn5b -lm 
	$(CC) $(CFLAGS) testIn5c.cc -o testIn5c.o
	$(CC) $(LDFLAGS) testIn5c.o -o testIn5c -lm 
	$(CC) $(CFLAGS) testIn5d.cc -o testIn5d.o
	$(CC) $(LDFLAGS) testIn5d.o -o testIn5d -lm 
	$(CC) $(CFLAGS) testIn5e.cc -o testIn5e.o
	$(CC) $(LDFLAGS) testIn5e.o -o testIn5e -lm 
	$(CC) $(CFLAGS) testIn5f.cc -o testIn5f.o
	$(CC) $(LDFLAGS) testIn5f.o -o testIn5f -lm 
	$(CC) $(CFLAGS) testIn6.cc -o testIn6.o
	$(CC) $(LDFLAGS) testIn6.o -o testIn6 -lm
	$(CC) $(CFLAGS) testIn7.cc -o testIn7.o
	$(CC) $(LDFLAGS) testIn7.o -o testIn7 -lm
	$(CC) $(CFLAGS) testIn7a.cc -o testIn7a.o
	$(CC) $(LDFLAGS) testIn7a.o -o testIn7a -lm
	$(CC) $(CFLAGS) testIn8.cc -o testIn8.o
	$(CC) $(LDFLAGS) testIn8.o -o testIn8 -lm
	$(CC) $(CFLAGS) testIn14.cc -o testIn14.o
	$(CC) $(LDFLAGS) testIn14.o -o testIn14 -lm
	$(CC) $(CFLAGS) testIn14a.cc -o testIn14a.o
	$(CC) $(LDFLAGS) testIn14a.o -o testIn14a -lm
	$(CC) $(CFLAGS) testIn15.cc -o testIn15.o
	$(CC) $(LDFLAGS) testIn15.o -o testIn15 -lm

	$(CC) $(CFLAGS) testOut1.cc -o testOut1.o
	$(CC) $(LDFLAGS) testOut1.o -o testOut1 -lm
	$(CC) $(CFLAGS) testOut2.cc -o testOut2.o
	$(CC) $(LDFLAGS) testOut2.o -o testOut2 -lm
	$(CC) $(CFLAGS) testOut5.cc -o testOut5.o
	$(CC) $(LDFLAGS) testOut5.o -o testOut5 -lm
	$(CC) $(CFLAGS) testOut5d.cc -o testOut5d.o
	$(CC) $(LDFLAGS) testOut5d.o -o testOut5d -lm 
	$(CC) $(CFLAGS) testOut5f.cc -o testOut5f.o
	$(CC) $(LDFLAGS) testOut5f.o -o testOut5f -lm 
	$(CC) $(CFLAGS) testOut14.cc -o testOut14.o
	$(CC) $(LDFLAGS) testOut14.o -o testOut14 -lm

	$(CC) $(CFLAGS) findDiff.cc -o findDiff.o
	$(CC) $(LDFLAGS) findDiff.o -o findDiff -lm

	$(CC) $(CFLAGS) FFTToOctave.cc -o FFTToOctave.o
	$(CC) $(LDFLAGS) FFTToOctave.o -o FFTToOctave -lm

	$(CC) $(CFLAGS) FFTToOctavePipe.cc -o FFTToOctavePipe.o
	$(CC) $(LDFLAGS) FFTToOctavePipe.o -o FFTToOctavePipe -lm

	$(CC) $(CFLAGS) FFTOverTimeToOctave.cc -o FFTOverTimeToOctave.o
	$(CC) $(LDFLAGS) FFTOverTimeToOctave.o -o FFTOverTimeToOctave -lm

	$(CC) $(CFLAGS) SignalToOctave.cc -o SignalToOctave.o
	$(CC) $(LDFLAGS) SignalToOctave.o -o SignalToOctave -lm

	$(CC) $(CFLAGS) 10MHz.cc -o 10MHz.o
	$(CC) $(LDFLAGS) 10MHz.o -o 10MHz -lm

	$(CC) $(CFLAGS) RFRealDig.cc -o RFRealDig.o
	$(CC) $(LDFLAGS) RFRealDig.o -o RFRealDig -lm

	$(CC) $(CFLAGS) testSmooth.cc -o testSmooth.o
	$(CC) $(LDFLAGS) testSmooth.o $(FIRFILTOBJ) -o testSmooth -lm

	$(CC) $(CFLAGS) whiteNoise.cc -o whiteNoise.o
	$(CC) $(LDFLAGS) whiteNoise.o -o whiteNoise -lm

	$(CC) $(CFLAGS) impulse.cc -o impulse.o
	$(CC) $(LDFLAGS) impulse.o -o impulse -lm


$(EXECUTABLE): $(SOURCES) $(OBJECTS) $(FIRFILTOBJ) $(RTLTCPOBJ) $(MODOBJ) $(FFTOBJ) $(AGCOBJ) $(FNLFOBJ)
	$(CC) $(LDFLAGS) $(OBJECTS) $(FIRFILTOBJ) $(RTLTCPOBJ) $(MODOBJ) $(FFTOBJ) $(AGCOBJ) $(FNLFOBJ) -o dspp

$(FNLFOBJ) : $(FNLFSRC)
	$(CC) $(CFLAGS) $*.cc -o $@
$(AGCOBJ) : $(AGCSRC)
	$(CC) $(CFLAGS) $*.cc -o $@
$(RTLTCPOBJ) : $(RTLTCPSRC)
	$(CC) $(CFLAGS) $*.cc -o $@
$(FIRFILTOBJ) : $(FIRFILTSRC)
	$(CC) $(CFLAGS) $*.cc -o $@
$(MODOBJ) : $(MODSRC)
	$(CC) $(CFLAGS) $*.cc -o $@

clean:
	rm -fr $(OBJECTS) $(EXECUTABLE) *.o

cpufeature = $(if $(findstring $(1),$(shell cat /proc/cpuinfo)),$(2))
PARAMS_SSE = $(call cpufeature,sse,-msse) $(call cpufeature,sse2,-msse2) $(call cpufeature,sse3,-msse3) $(call cpufeature,sse4a,-msse4a) $(call cpufeature,sse4_1,-msse4.1) $(call cpufeature,sse4_2,-msse4.2 -msse4) -mfpmath=sse 
PARAMS_NEON = -mfloat-abi=hard -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mvectorize-with-neon-quad -funsafe-math-optimizations -Wformat=0 -DNEON_OPTS
#tnx Jan Szumiec for the Raspberry Pi support
PARAMS_RASPI = -mfloat-abi=hard -mcpu=arm1176jzf-s -mfpu=vfp -funsafe-math-optimizations -Wformat=0
PARAMS_ARM = $(if $(call cpufeature,BCM2708,dummy-text),$(PARAMS_RASPI),$(PARAMS_NEON))
PARAMS_SIMD = $(if $(call cpufeature,sse,dummy-text),$(PARAMS_SSE),$(PARAMS_ARM))
PARAMS_LOOPVECT = -O3 -ffast-math -fdump-tree-vect-details -dumpbase dumpvect
PARAMS_LIBS = -g -lm -lstdc++ -lfftw3 -lcurl -l pthread
PARAMS_SO = -fpic  
PARAMS_MISC = -Wno-unused-result
FFTW_PACKAGE = fftw-3.3.3

CC=gcc

CFLAGS= $(if $(shell uname -a | grep -i armv), -c -Wall -DLE_MACHINE -D_GNU_SOURCE $(PARAMS_LOOPVECT) $(PARAMS_SIMD) $(PARAMS_MISC), -c -Wall -DLE_MACHINE -D_GNU_SOURCE )
CXX = $(CC)
CXXFLAGS = $(CFLAGS) # set these flags for use of suffix rules for cc
LDFLAGS= $(PARAMS_LIBS)

SOURCES= dspp.cc 
OBJECTS=$(SOURCES:.cc=.o)

WSPRSRC = WSPRUtilities.cc WSPRUtilities.h WindowSample.cc WindowSample.h WSPRWindow.cc WSPRWindow.h Fano.cc Fano.h SpotCandidate.cc SpotCandidate.h
AGCSRC = AGC.cc AGC.h
RTLTCPSRC = RTLTCPClient.cc RTLTCPClient.h RTLTCPServer.cc RTLTCPServer.h
FIRFILTSRC = FIRFilter.cc FIRFilter.h SFIRFilter.cc SFIRFilter.h CFilter.cc CFilter.h Poly.cc Poly.h
MODSRC = FMMod.cc FMMod.h
FFTSRC = DsppFFT.cc DsppFFT.h
BASICSRC = Regression.cc Regression.h
WSPROBJ = WSPRUtilities.o WindowSample.o WSPRWindow.o Fano.o SpotCandidate.o
AGCOBJ = AGC.o
RTLTCPOBJ = RTLTCPClient.o RTLTCPServer.o
FIRFILTOBJ = FIRFilter.o SFIRFilter.o CFilter.o Poly.o
MODOBJ = FMMod.o
FFTOBJ = DsppFFT.o
BASICOBJ = Regression.o

EXECUTABLE=dspp

all: $(EXECUTABLE)

tools:
	$(CC) $(CFLAGS) FFTToOctave.cc -o FFTToOctave.o
	$(CC) $(LDFLAGS) FFTToOctave.o -o FFTToOctave -lm

	$(CC) $(CFLAGS) FFTToOctavePipe.cc -o FFTToOctavePipe.o
	$(CC) $(LDFLAGS) FFTToOctavePipe.o -o FFTToOctavePipe -lm

	$(CC) $(CFLAGS) FFTOverTimeToOctave.cc -o FFTOverTimeToOctave.o
	$(CC) $(LDFLAGS) FFTOverTimeToOctave.o -o FFTOverTimeToOctave -lm

	$(CC) $(CFLAGS) FFTOverTimeToOctavePipe.cc -o FFTOverTimeToOctavePipe.o
	$(CC) $(LDFLAGS) FFTOverTimeToOctavePipe.o -o FFTOverTimeToOctavePipe -lm

	$(CC) $(CFLAGS) baseBandWSPR.cc -o baseBandWSPR.o
	$(CC) $(LDFLAGS) baseBandWSPR.o -o baseBandWSPR -lm

	$(CC) $(CFLAGS) MorseDecoder.cc -o MorseDecoder.o
	$(CC) $(LDFLAGS) MorseDecoder.o -o MorseDecoder -lm


$(EXECUTABLE): $(SOURCES) $(OBJECTS) $(FIRFILTOBJ) $(RTLTCPOBJ) $(MODOBJ) $(FFTOBJ) $(AGCOBJ) $(WSPROBJ) $(BASICOBJ)
	$(CC) $(OBJECTS) $(FIRFILTOBJ) $(RTLTCPOBJ) $(MODOBJ) $(FFTOBJ) $(AGCOBJ) $(WSPROBJ) $(BASICOBJ) -o dspp $(LDFLAGS)

$(BASICOBJ) : $(BASICSRC)
	$(CC) $(CFLAGS) $*.cc -o $@
$(WSPROBJ) : $(WSPRSRC)
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

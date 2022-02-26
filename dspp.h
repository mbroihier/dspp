/*
 *      dspp.h - DSP Pipes class
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "FIRFilter.h"
#include "DsppFFT.h"
#include "FMMod.h"
#include "RTLTCPClient.h"
#include "RTLTCPServer.h"
#include "SFIRFilter.h"
/* ---------------------------------------------------------------------- */
class dspp {

  private:

  public:

  int convert_byte_sInt16();
  int convert_byte_f();
  int convert_uByte_f();
  int shift_frequency_cc(float cyclesPerSample);
  int decimate_cc(float cutOffFrequency, int M, int amount, int N, const char * window);
  int fmdemod_cf();
  int decimate_ff(float cutOffFrequency, int M, int amount, int N, const char * window);
  int convert_f_uInt16();
  int convert_f_sInt16();
  int convert_tcp_byte(const char * IPAddress, int port, int frequency, int sampleRate);
  int convert_byte_tcp(const char * IPAddress, int port);
  int custom_fir_ff(const char * filePath, int M, int N, FIRFilter::WindowType window);
  int custom_fir_cc(const char * filePath, int M, int N, FIRFilter::WindowType window);
  int real_to_complex_fc();
  int real_of_complex_cf();
  int fmmod_fc(float sampleRate);
  int head(int amount);
  int tail(int amount);
  int convert_sInt16_f();
  int fft_cc(int numberOfComplexSamples);
  int tee(char * otherStream);
  int direct_to_iq();

  //dspp(void);

  //~dspp(void);
    
};


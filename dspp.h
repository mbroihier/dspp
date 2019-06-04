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
#include "RTLTCPClient.h"
#include "RTLTCPServer.h"
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

  //dspp(void);

  //~dspp(void);
    
};


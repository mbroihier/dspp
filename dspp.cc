/*
 *      dspp.c -- DSP Pipe - process signals via piped commands
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>

#include <sys/ioctl.h>

#include "dspp.h"
/* ---------------------------------------------------------------------- */


static const char USAGE_STR[] = "\n"
        "Usage: %s <command> [ parameter 1 [ ... parameter n]]\n"
        "  -h                       : help\n"
        "  convert_byte_sInt16      : convert little endian byte stream to internal short ints\n"
        "  convert_byte_f           : convert a signed byte stream to internal floating point\n"
        "  convert_uByte_f          : convert a unsigned byte stream to internal floating point\n"
        "  shift_frequency_cc       : recenter a signal by x cycles per sample\n"
        "  decimate_cc              : replace every n samples with 1 (complex)\n"
        "  fmdemod_cf               : demodulate FM signal\n"
        "  decimate_ff              : replace every n samples with 1 (real)\n"
        "  convert_f_uInt16         : convert a float(real) stream into an unsigned short stream\n"
        "  convert_f_sInt16         : convert a float(real) stream into an signed short stream\n"
        "  convert_tcp_byte         : convert a tcp stream into an unsigned/generic byte stream\n"
        "  convert_byte_tcp         : convert a byte stream to a tcp byte stream\n"
        "  custom_fir_ff            : FIR filter a real stream\n"
        "  custom_fir_cc            : FIR filter a complex stream\n"
        "  sfir_cc                  : Smooth FIR filter a complex stream\n"
        "  comb_cc                  : Comb filter a complex stream\n"
        "  sfir_ff                  : Smooth FIR filter a real stream\n"
        "  real_to_complex_fc       : real stream to complex stream\n"
        "  real_of_complex_fc       : real part of complex stream\n"
        "  fmmod_fc                 : real stream FM modulated quadrature (I/Q) stream\n"
        "  head                     : take first n bytes of stream\n"
        "  tail                     : take bytes after n bytes of stream\n"
        "  convert_sInt16_f         : convert a signed short stream to a float(real) stream\n"
        "  fft_cc                   : convert a complex stream to a complex stream in the frequency domain\n"
        "  tee                      : tee stream to another stream\n"
        "  sfir_cc                  : smooth fir filter, complex stream to complex stream\n"
        "  sfir_ff                  : smooth fir filter, float(real) stream to float stream\n"
        "  real_of_complex_cf       : real(float) part of complex stream to float stream\n"
        "  direct_to_iq             : direct stream of bytes to iq bytes\n"
        "  comb_cc                  : comb filter complex stream to complex stream\n"
        "  mag_cf                   : magnitude of complex number\n"
        "  gain                     : multiply float/complex by scalar\n"
        "  limit_real_stream        : limit a floating point stream between -1.0 and 1.0\n"
        "  dc_removal               : remove average value of the stream\n"
        "  agc                      : automatic gain control, sustain a fixed average level\n";

static struct option longOpts[] = {
  { "convert_byte_sInt16"      , no_argument, NULL, 1 },
  { "convert_byte_f"           , no_argument, NULL, 2 },
  { "convert_uByte_f"          , no_argument, NULL, 3 },
  { "shift_frequency_cc"       , no_argument, NULL, 4 },
  { "decimate_cc"              , no_argument, NULL, 5 },
  { "fmdemod_cf"               , no_argument, NULL, 6 },
  { "decimate_ff"              , no_argument, NULL, 7 },
  { "convert_f_uInt16"         , no_argument, NULL, 8 },
  { "convert_f_sInt16"         , no_argument, NULL, 9 },
  { "convert_tcp_byte"         , no_argument, NULL, 10 },
  { "convert_byte_tcp"         , no_argument, NULL, 11 },
  { "custom_fir_ff"            , no_argument, NULL, 12 },
  { "custom_fir_cc"            , no_argument, NULL, 13 },
  { "real_to_complex_fc"       , no_argument, NULL, 14 },
  { "fmmod_fc"                 , no_argument, NULL, 15 },
  { "head"                     , no_argument, NULL, 16 },
  { "tail"                     , no_argument, NULL, 17 },
  { "convert_sInt16_f"         , no_argument, NULL, 18 },
  { "fft_cc"                   , no_argument, NULL, 19 },
  { "tee"                      , no_argument, NULL, 20 },
  { "sfir_cc"                  , no_argument, NULL, 21 },
  { "sfir_ff"                  , no_argument, NULL, 22 },
  { "real_of_complex_cf"       , no_argument, NULL, 23 },
  { "direct_to_iq"             , no_argument, NULL, 24 },
  { "comb_cc"                  , no_argument, NULL, 25 },
  { "mag_cf"                   , no_argument, NULL, 26 },
  { "gain"                     , no_argument, NULL, 27 },
  { "limit_real_stream"        , no_argument, NULL, 28 },
  { "dc_removal"               , no_argument, NULL, 29 },
  { "agc"                      , no_argument, NULL, 30 },
  { NULL, 0, NULL, 0 }
};

/* ---------------------------------------------------------------------- */
/*
 *      convert_byte_sInt16.c -- DSP Pipe - byte stream to short int stream
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#ifdef LE_MACHINE
int dspp::convert_byte_sInt16() {

  union encode {
    char bytes[2];
    short integer;
  } piece;

  const int byteBufferSize = sizeof(piece.bytes);
  for (;;) {
    fread(&piece.bytes, sizeof(char), byteBufferSize, stdin);
    fwrite(&piece.integer, sizeof(short), 1, stdout);
  }

  return 0;

}

/* ---------------------------------------------------------------------- */
#else

int dspp::convert_byte_sInt16() {

  union encode {
    char bytes[2];
    short integer;
  } piece;

  for (;;) {
    fread(&piece.bytes[1], sizeof(char), 1, stdin);
    fread(&piece.bytes[0], sizeof(char), 1, stdin);
    fwrite(&piece.integer, sizeof(short), 1, stdout);
  }

  return 0;

}
#endif

/* ---------------------------------------------------------------------- */
/*
 *      convert_byte_f.c -- DSP Pipe - byte(signed) stream to float
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

int dspp::convert_byte_f() {
  const int BUFFER_SIZE = 4096;
  signed char c[BUFFER_SIZE];
  float f[BUFFER_SIZE];
  int count;
  float * fptr;
  signed char * cptr;
  for (;;) {
    count = fread(&c, sizeof(char), BUFFER_SIZE, stdin);
    if(count < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream, convert_byte_f\n");
      fclose(stdout);
      return 0;
    }
    cptr = c;
    fptr = f;
    for (int i=0; i < BUFFER_SIZE; i++) {
      *fptr++ = *cptr++/128.0 ;
    }
    fwrite(&f, sizeof(float), BUFFER_SIZE, stdout);
  }

  return 0;

}

/* ---------------------------------------------------------------------- */
/*
 *      convert_aUnsignedByte_f.c -- DSP Pipe - byte(unsigned) stream to float
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

int dspp::convert_uByte_f() {
  const int BUFFER_SIZE = 4096;
  unsigned char c[BUFFER_SIZE];
  float f[BUFFER_SIZE];
  int count;
  float * fptr;
  unsigned char * cptr;
  for (;;) {
    count = fread(&c, sizeof(char), BUFFER_SIZE, stdin);
    if(count < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream, convert_uByte_f\n");
      fclose(stdout);
      return 0;
    }
    cptr = c;
    fptr = f;
    for (int i=0; i < BUFFER_SIZE; i++) {
      *fptr++ = (*cptr++ - 128)/128.0;
    }
    fwrite(&f, sizeof(float), BUFFER_SIZE, stdout);
  }

  return 0;

}

/* ---------------------------------------------------------------------- */

/*
 *      shift_frequency_cc.cc -- DSP Pipe - shift the frequency of a quadature
 *                              by amount cycles per sample
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

int dspp::shift_frequency_cc(float amount) {
  const int BUFFER_SIZE = 4096;
  float f[BUFFER_SIZE];
  float of[BUFFER_SIZE];
  int count;
  float * fptr, * ofptr;
  float sinDeltaAmount = sin(amount*2.0*M_PI);
  float cosDeltaAmount = cos(amount*2.0*M_PI);
  float cosAmount = 1.0;
  float sinAmount = 0.0;
  float newCosAmount;
  float newSinAmount;

  float I;
  float Q;

  fprintf(stderr, "cycles per sample correction: %f\n", amount);
  for (;;) {
    count = fread(&f, sizeof(float), BUFFER_SIZE, stdin);
    if(count < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream, shift_frequency_cc\n");
      fclose(stdout);
      return 0;
    }
    fptr = f;
    ofptr = of;

    for (int i=0; i < BUFFER_SIZE; i+=2) {
      /*
        Assuming I is r*cos(2*PI*fc*t) and
                 Q is -r*sin(2*PI*fc*t)  where fc is the center frequency the signal r is sampled at

        Then to to recenter to a new frequency fs, use the trigonometric identities:
          cos(a + b) = cos(a)*cos(b) - sin(a)*sin(b)
          sin(a + b) = sin(a)*cos(b) + cos(a)*sin(b)

        shifted I = I * cos(2*PI*fs*t) + Q * sin(2*PI*fs*t)
        shifted Q = Q * cos(2*PI*fs*t) - I * sin(2*PI*fs*t)

        To advance the sin/cos 2*PI*fs*t functions, the sin/cos of sum of angles identities are again 
        used. Since each sample is separated by a fixed delta of time, successively applying the 
        identities advances the angle/time.

       */
      I = *fptr++;
      Q = *fptr++;
      *ofptr++ = I * cosAmount + Q * sinAmount;
      *ofptr++ = Q * cosAmount - I * sinAmount;
      newCosAmount = cosAmount * cosDeltaAmount - sinAmount * sinDeltaAmount;
      newSinAmount = sinAmount * cosDeltaAmount + cosAmount * sinDeltaAmount;
      cosAmount = newCosAmount;
      sinAmount = newSinAmount;
    }
    fwrite(&of, sizeof(float), BUFFER_SIZE, stdout);
  }

  return 0;

}

/* ---------------------------------------------------------------------- */
/*
 *      decimate_cc.cc -- DSP Pipe - shift the frequency of a quadature
 *                              by amount cycles per sample
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int dspp::decimate_cc(float cutOffFrequency, int M, int amount, int N, const char * window) {

  FIRFilter filter(cutOffFrequency, M, amount, N, FIRFilter::HAMMING);
  filter.filterSignal();
  
  return 0;

}

/* ---------------------------------------------------------------------- */
/*
 *      fmdemod_cf.cc -- DSP Pipe - demodulate a FM signal
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 *
 *  This assumes FM modulation has the form of:
 *  y(t) = Ac*cos(2*PI*fc*t + 2*PI*fd Integral of x(tau)d(tau))
 *         Ac - carrier amplitude
 *         fc - carrier frequency
 *         fd - maximum frequency deviation
 *
 *  If we let s(t) = 2*PI*fd Integral of x(tau)d(tau) and we use the
 *  trigonometric formula for cos of the sum of two angles, then
 *  y(t) = Ac[cos(2*PI*fc*t)*cos(s(t)) - sin(2*PI*fc*t)*sin(s(t))]
 *
 *  At the radio receiver we'll have a signal such as:
 *  r(t) = Ar[cos(2*PI*fc*t)*cos(s(t)) - sin(2*PI*fc*t)*sin(s(t))]
 *
 *  Assuming the receiver processes the RF signal into a quadrature 
 *  signal of I and Q about the carrier frequency fc, then 
 *  I = cos(s(t)) and Q = sin(s(t))
 *
 *  To recover x(tau) we can do the following:
 *  Q / I = sin(s(t))/cos(s(t)) = tan(s(t))
 *  so,
 *        arctan(Q/I) = 2*PI*fd Integral x(tau)d(tau)
 *  Take the derivative
 *        arctan(Q/I)' = 2*PI*fd*x(tau)
 *        x(tau) = 1/(2*PI*fd) arctan(Q/I)' 
 *               = 1/(2*PI*fd) [1/(1 + (Q/I) * (Q/I))]*[I*Q' - Q*I']/(I*I)
 *
 *        The constant, 1/(2*PI*fd), is changed to 1 assuming gain further
 *        down the processing line.
 *
 *        I' is approximated by I - last value of I and 
 *        Q' is approximated by Q - last value of Q
 *
 *        To avoid a singularity, if any I is less than epsilon,
 *        the last value of x(t) is used.
 */

/* ---------------------------------------------------------------------- */
int dspp::fmdemod_cf() {
  const int BUFFER_SIZE = 2048;
  float f[BUFFER_SIZE];
  const int HALF_BUFFER_SIZE = BUFFER_SIZE / 2;
  float of[HALF_BUFFER_SIZE];
  float * fptr, * ofptr;
  int fSize = sizeof(f);
  int ofSize = sizeof(of);
  const float EPSILON = 1.0 / 128.0;

  int count = 0;

  fprintf(stderr, "demod buffer input size: %d\n", fSize);

  float I, Q;
  float lastI = 0.0;
  float lastQ = 0.0;
  float lastOutput = 0.0;
  float Isquared;
  float Qsquared;

  for (;;) {
    count = fread(&f, sizeof(char), fSize, stdin);
    if(count < fSize) {
      fprintf(stderr, "Short data stream, fmdemod_cf\n");
      fclose(stdout);
      return 0;
    }
    ofptr = of;
    fptr = f;
    for (int i = 0; i < HALF_BUFFER_SIZE; i++) {
      I = *fptr++;
      Q = *fptr++;
      if ((fabs(I) > EPSILON) || (fabs(Q) > EPSILON)) {
        Isquared = I * I;
	Qsquared = Q * Q;
        //lastOutput = (I * (Q - lastQ) - Q * (I - lastI)) / ((1.0 + ratio * ratio) * I * I);
        lastOutput = (Q*lastI - I*lastQ) / (Isquared + Qsquared);
      }
      *ofptr++ = lastOutput;
      lastI = I;
      lastQ = Q;
    }
    fwrite(&of, sizeof(char), ofSize, stdout);
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
/*
 *      decimate_ff.cc -- DSP Pipe - decimate real signal
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

int dspp::decimate_ff(float cutOffFrequency, int M, int amount, int N, const char * window) {

  FIRFilter filter(cutOffFrequency, M, amount, N, FIRFilter::HAMMING, true);
  filter.filterReal();
  
  return 0;

}

/* ---------------------------------------------------------------------- */
/*
 *      convert_f_uInt16.cc -- DSP Pipe - float (-1.0 to 1.0)
 *                                    to 0 to 65535
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

int dspp::convert_f_uInt16() {
  const int BUFFER_SIZE = 4096;
  unsigned short uin[BUFFER_SIZE];
  float f[BUFFER_SIZE];
  int count;
  float * fptr;
  unsigned short * iptr;
  for (;;) {
    count = fread(&f, sizeof(float), BUFFER_SIZE, stdin);
    if(count < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream, convert_f_uInt16\n");
      fclose(stdout);
      return 0;
    }
    iptr = uin;
    fptr = f;
    for (int i=0; i < BUFFER_SIZE; i++) {
      *iptr++ = (*fptr++ + 1.0) * 32767.0;
    }
    fwrite(&uin, sizeof(short), BUFFER_SIZE, stdout);
  }

  return 0;

}

/* ---------------------------------------------------------------------- */
/*
 *      convert_f_sInt16.cc -- DSP Pipe - float (-1.0 to 1.0)
 *                                    to -32767 to 32767
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

int dspp::convert_f_sInt16() {
  const int BUFFER_SIZE = 4096;
  short sin[BUFFER_SIZE];
  float f[BUFFER_SIZE];
  int count;
  float * fptr;
  short * iptr;
  for (;;) {
    count = fread(&f, sizeof(float), BUFFER_SIZE, stdin);
    if(count < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream, convert_f_sInt16\n");
      fclose(stdout);
      return 0;
    }
    iptr = sin;
    fptr = f;
    for (int i=0; i < BUFFER_SIZE; i++) {
      *iptr++ = *fptr++ * 32767.0;
    }
    fwrite(&sin, sizeof(short), BUFFER_SIZE, stdout);
  }

  return 0;

}
/* ---------------------------------------------------------------------- */
/*
 *      convert_sInt16_f -- DSP Pipe - signed int -32767 to 32767
 *                                    float (-1.0 to 1.0)
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

int dspp::convert_sInt16_f() {
  const int BUFFER_SIZE = 4096;
  short sin[BUFFER_SIZE];
  float f[BUFFER_SIZE];
  int count;
  float * fptr;
  short * iptr;
  float scale = 1.0 / 32767.0;
  for (;;) {
    count = fread(&sin, sizeof(short), BUFFER_SIZE, stdin);
    if(count < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream, convert_sInt16_f\n");
      fprintf(stderr, "shorts: %d\n", count);
      fclose(stdout);
      return 0;
    }
    iptr = sin;
    fptr = f;
    for (int i=0; i < BUFFER_SIZE; i++) {
      *fptr++ = *iptr++ * scale;
    }
    fwrite(&f, sizeof(float), BUFFER_SIZE, stdout);
  }

  return 0;

}

/* ---------------------------------------------------------------------- */
/*
 *      convert_tcp_byte.cc -- DSP Pipe - convert a TCP stream connection to a 
 *                                      byte(generic) stream
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int dspp::convert_tcp_byte(const char * IPAddress, int port, int frequency, int sampleRate, int mode, int gain) {
  fprintf(stderr, "Creating a client object\n");
  RTLTCPClient client(IPAddress, port, frequency, sampleRate, mode, gain);
  fprintf(stderr, "Client terminated\n");
  return 0;
}

/* ---------------------------------------------------------------------- */
/*
 *      convert_byte_tcp.cc -- DSP Pipe - convert a byte stream to a TCP stream
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int dspp::convert_byte_tcp(const char * IPAddress, int port) {
  fprintf(stderr, "Creating a TCP server object\n");
  RTLTCPServer client(IPAddress, port);
  fprintf(stderr, "Server terminated\n");
  return 0;
}

/* ---------------------------------------------------------------------- */
/*
 *      custom_fir_ff.cc -- DSP Pipe - custom FIR filter
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int dspp::custom_fir_ff(const char * filePath, int M, int N, FIRFilter::WindowType window) {
  fprintf(stderr, "Creating a custom FIR filter object\n");
  FIRFilter customFilter(filePath, M, N, FIRFilter::CUSTOM, true);
  customFilter.filterReal();
  fprintf(stderr, "Custom FIR filter terminated\n");
  return 0;
}

/* ---------------------------------------------------------------------- */
/*
 *      custom_fir_cc.cc -- DSP Pipe - custom FIR filter
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int dspp::custom_fir_cc(const char * filePath, int M, int N, FIRFilter::WindowType window) {
  fprintf(stderr, "Creating a custom FIR filter object\n");
  FIRFilter customFilter(filePath, M, N, FIRFilter::CUSTOM);
  customFilter.filterSignal();
  fprintf(stderr, "Custom FIR filter terminated\n");
  return 0;
}
/* ---------------------------------------------------------------------- */
/*
 *      real_to_complex_fc.cc -- DSP Pipe - real stream to complex quadrature
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int dspp::real_to_complex_fc() {
  const int BUFFER_SIZE = 4096;
  float signal[BUFFER_SIZE];
  float complexSignal[BUFFER_SIZE*2];
  int numberRead = 0;

  float * signalPtr;
  float * complexSignalPtr;
  for (;;) {
    numberRead = fread(&signal, sizeof(float), BUFFER_SIZE, stdin);
    if(numberRead < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream, real_to_complex_fc\n");
      fclose(stdout);
      return 0;
    }
    signalPtr = signal;
    complexSignalPtr = complexSignal;
    for (int i=0; i < BUFFER_SIZE; i++) {
      *complexSignalPtr++ = *signalPtr++;
      *complexSignalPtr++ = 0.0;
    }
    fwrite(&complexSignal, sizeof(float), BUFFER_SIZE*2, stdout);
  }

  return 0;
}
/* ---------------------------------------------------------------------- */
/*
 *      real_of_complex_fc.cc -- DSP Pipe - real part of complex stream
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int dspp::real_of_complex_cf() {
  const int BUFFER_SIZE = 4096;
  float signal[BUFFER_SIZE];
  float complexSignal[BUFFER_SIZE*2];
  int numberRead = 0;

  float * signalPtr;
  float * complexSignalPtr;
  for (;;) {
    numberRead = fread(&complexSignal, sizeof(float), BUFFER_SIZE*2, stdin);
    if(numberRead < BUFFER_SIZE*2) {
      fprintf(stderr, "Short data stream, real_of_complex_cf\n");
      fclose(stdout);
      return 0;
    }
    signalPtr = signal;
    complexSignalPtr = complexSignal;
    for (int i=0; i < BUFFER_SIZE; i++) {
      *signalPtr++ = *complexSignalPtr++;
      complexSignalPtr++;
    }
    fwrite(&signal, sizeof(float), BUFFER_SIZE, stdout);
  }

  return 0;
}

/* ---------------------------------------------------------------------- */
/*
 *      mag_cf.cc -- DSP Pipe - magnitude of complex stream
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int dspp::mag_cf() {
  const int BUFFER_SIZE = 4096;
  float signal[BUFFER_SIZE*2];
  float mag[BUFFER_SIZE];
  int numberRead = 0;

  float * magPtr;
  float  * complexSignalPtr;
  for (;;) {
    numberRead = fread(&signal, sizeof(float), BUFFER_SIZE*2, stdin);
    if(numberRead < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream, mag_cf\n");
      fclose(stdout);
      return 0;
    }
    magPtr = mag;
    complexSignalPtr = signal;
    for (int i=0; i < BUFFER_SIZE; i++) {
      float r = *complexSignalPtr++;
      float j = *complexSignalPtr++;
      *magPtr++ = sqrt(r * r + j * j);
    }
    fwrite(&mag, sizeof(float), BUFFER_SIZE, stdout);
  }

  return 0;
}

/* ---------------------------------------------------------------------- */
/*
 *      gain.cc -- DSP Pipe - multiply float/complex stream by scalar
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int dspp::gain(float gain) {
  const int BUFFER_SIZE = 4096;
  float signal[BUFFER_SIZE];
  float amplifiedSignal[BUFFER_SIZE];
  int numberRead = 0;

  float * amplifiedSignalPtr;
  float  * signalPtr;
  for (;;) {
    numberRead = fread(&signal, sizeof(float), BUFFER_SIZE, stdin);
    if(numberRead < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream, signal\n");
      fclose(stdout);
      return 0;
    }
    signalPtr = signal;
    amplifiedSignalPtr = amplifiedSignal;
    for (int i=0; i < BUFFER_SIZE; i++) {
      *amplifiedSignalPtr++ = *signalPtr++ * gain;
    }
    fwrite(&amplifiedSignal, sizeof(float), BUFFER_SIZE, stdout);
  }

  return 0;
}

/* ---------------------------------------------------------------------- */
/*
 *      limit_real_stream.cc -- DSP Pipe - limit floating point values to
 *                              -1.0 to 1.0 range
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int dspp::limit_real_stream() {
  const int BUFFER_SIZE = 4096;
  float signal[BUFFER_SIZE];
  float output[BUFFER_SIZE];
  int numberRead = 0;

  float * outputPtr;
  float  * signalPtr;
  for (;;) {
    numberRead = fread(&signal, sizeof(float), BUFFER_SIZE, stdin);
    if(numberRead < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream, signal\n");
      fclose(stdout);
      return 0;
    }
    signalPtr = signal;
    outputPtr = output;
    for (int i=0; i < BUFFER_SIZE; i++) {
      float r = *signalPtr++;
      if (r > 1.0) {
        r = 1.0;
      } else {
        if (r < -1.0) {
          r = -1.0;
        }
      }
      *outputPtr++ = r;
    }
    fwrite(&output, sizeof(float), BUFFER_SIZE, stdout);
  }

  return 0;
}

/* ---------------------------------------------------------------------- */
/*
 *      dc_removal.cc -- DSP Pipe - remove the average value from the signal
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int dspp::dc_removal(float * buffer, int size) {
  float * signal = reinterpret_cast<float *>(malloc(sizeof(float)* size));
  float * output = reinterpret_cast<float *>(malloc(sizeof(float)* size));
  float accumulator = 0.0;
  int numberRead = 0;

  float * signalPtr;
  float * outputPtr;
  float * bufferPtr;
  float scale = 1.0 / size;
  for (;;) {
    numberRead = fread(signal, sizeof(float), size, stdin);
    if(numberRead < size) {
      fprintf(stderr, "Short data stream, dc_removal\n");
      fclose(stdout);
      free(signal);
      free(output);
      return 0;
    }
    signalPtr = signal;
    bufferPtr = buffer;
    outputPtr = output;
    for (int i=0; i < size; i++) {
      accumulator -= *bufferPtr;  // remove old value
      accumulator += *signalPtr;  // add new value
      *outputPtr++ = *signalPtr - accumulator * scale;  // remove average
      *bufferPtr++ = *signalPtr++;
    }
    fwrite(output, sizeof(float), size, stdout);
    //fprintf(stderr, "bias = %f\n", accumulator * scale);
  }

  return 0;
}

/* ---------------------------------------------------------------------- */
/*
 *      agc.cc -- DSP Pipe - automatic gain control
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int dspp::agc(float target) {
  AGC * agcObject;
  agcObject = new AGC(target);
  if (! agcObject) {
    fprintf(stderr, "AGC object creation failed\n");
  } else {
    agcObject->doWork();
  }
  return 0;
}

/* ---------------------------------------------------------------------- */
/*
 *      head.cc -- DSP Pipe - take the first n bytes of a stream
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

int dspp::head(int amount) {
  const int BUFFER_SIZE = 4096;
  int count = 0;
  unsigned char bytes[BUFFER_SIZE];
  for (;;) {
    count = fread(&bytes, sizeof(unsigned char), BUFFER_SIZE, stdin);
    if(count < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream, head\n");
      fwrite(&bytes, sizeof(unsigned char), count, stdout);
      fclose(stdout);
      return 0;
    }
    if (amount > count) {
      fwrite(&bytes, sizeof(unsigned char), count, stdout);
    } else {
      fwrite(&bytes, sizeof(unsigned char), amount, stdout);
    }
    amount -= BUFFER_SIZE;
    if (amount < 1) {
      break;
    }
  }
  fclose(stdout);
  return 0;
}
/* ---------------------------------------------------------------------- */
/*
 *      tail.cc -- DSP Pipe - take bytes after the first n bytes of a stream
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

int dspp::tail(int amount) {
  const int BUFFER_SIZE = 4096;
  int count = 0;
  int offset = 0;
  unsigned char bytes[BUFFER_SIZE];
  bool skipPhase = true;
  for (;;) {
    count = fread(&bytes, sizeof(unsigned char), BUFFER_SIZE, stdin);
    if(count < BUFFER_SIZE) {
      if (count == 0) {
        fprintf(stderr, "Short data stream, tail\n");
        fwrite(&bytes, sizeof(unsigned char), count, stdout);
        fclose(stdout);
        return 0;
      }
    }
    if (skipPhase) {
      amount -= count;
      if (amount < 1) {
        offset = abs(amount);
        count -= offset;
        fwrite(&bytes[offset], sizeof(unsigned char), count, stdout);
        skipPhase = false;
      }
    } else {
        fwrite(&bytes, sizeof(unsigned char), count, stdout);
    }
  }
  fprintf(stderr, "Internal error path - should not get here, tail\n");
  fclose(stdout);
  return 0;
}
/* ---------------------------------------------------------------------- */
/*
 *      fft_cc.cc -- DSP Pipe - FFT of a complex stream
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

int dspp::fft_cc(int numberOfComplexSamples) {
  DsppFFT transform(numberOfComplexSamples);
  transform.processSampleSet();
  return 0;
}
/* ---------------------------------------------------------------------- */
/*
 *      tee.cc -- DSP Pipe - tee stream to stream
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

int dspp::tee(char * otherStream) {
  const int BUFFER_SIZE = 4096;
  int count = 0;
  unsigned char bytes[BUFFER_SIZE];
  FILE * otherPath;
  otherPath = popen(otherStream, "w");
  for (;;) {
    count = fread(&bytes, sizeof(unsigned char), BUFFER_SIZE, stdin);
    if(count < BUFFER_SIZE) {
      if (count == 0) {
        fprintf(stderr, "Short data stream, tail\n");
        fwrite(&bytes, sizeof(unsigned char), count, stdout);
        fwrite(&bytes, sizeof(unsigned char), count, otherPath);
        fclose(stdout);
        pclose(otherPath);
        return 0;
      }
    }
    fwrite(&bytes, sizeof(unsigned char), count, stdout);
    fwrite(&bytes, sizeof(unsigned char), count, otherPath);
  }
  fprintf(stderr, "Internal error path - should not get here, tail\n");
  fclose(stdout);
  return 0;
}
/* ---------------------------------------------------------------------- */
/*
 *      direct_to_iq.cc -- DSP Pipe - direct sample to IQ sample
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

int dspp::direct_to_iq() {
  const int BUFFER_SIZE = 4096;
  int count = 0;
  signed char bytes[BUFFER_SIZE];
  signed char data[BUFFER_SIZE];
  for (;;) {
    count = fread(&bytes, sizeof(signed char), BUFFER_SIZE, stdin);
    if(count != BUFFER_SIZE) {
      fprintf(stderr, "Short data stream, tail\n");
      fclose(stdout);
      return 0;
    }
    for (int i = 0; i < BUFFER_SIZE; i += 8) {
      data[i + 0] = 0;
      data[i + 1] = bytes[i + 1];
      data[i + 2] = bytes[i + 2];
      data[i + 3] = 0;
      data[i + 4] = 0;
      data[i + 5] = - bytes[i + 5];
      data[i + 6] = - bytes[i + 6];
      data[i + 7] = 0;
    }
    fwrite(&data, sizeof(signed char), count, stdout);
  }
  fprintf(stderr, "Internal error path - should not get here, direct_to_iq\n");
  fclose(stdout);
  return 0;
}
/* ---------------------------------------------------------------------- */

int main(int argc, char *argv[]) {

  dspp dsppInstance;

  int c;

  const unsigned int COMMAND_LENGTH = 32;

  if (argc <= 1 || !argv[1] || (strlen(argv[1]) >= COMMAND_LENGTH)) {
    fprintf(stderr, USAGE_STR, argv[0]);
    return -2;
  }

  char command[COMMAND_LENGTH];

  memset(command, 0, sizeof(command));

  char * new_argv[argc];
  if (argc > 1) {
    if (argv[1][0] == '-') {
      snprintf(command, sizeof(command)-1, "%s", argv[1]);
    } else {
      snprintf(command, sizeof(command)-3, "--%s", argv[1]);
    }
  } else {
    fprintf(stderr, USAGE_STR, argv[0]);
    return -2;
  }

  int index;

  for (index = 0; index < argc; index++) {
    if (index == 1) {
      new_argv[1] = (char *) &command;
    } else {
      new_argv[index] = argv[index];
    }
  }

  int doneProcessing = 0;
  while ((c = getopt_long(argc, new_argv, "h", longOpts, NULL)) >= 0) {
    switch (c) {
      case 'h': {
        fprintf(stderr, USAGE_STR, argv[0]);
        return -2;
      }
      case 1: {
        doneProcessing = !dsppInstance.convert_byte_sInt16();
        break;
      }
      case 2: {
        doneProcessing = !dsppInstance.convert_byte_f();
        break;
      }
      case 3: {
        doneProcessing = !dsppInstance.convert_uByte_f();
        break;
      }
      case 4: {
          float amount;
	  if (argc == 3) {
            sscanf(argv[2], "%f", &amount);
            doneProcessing = !dsppInstance.shift_frequency_cc(amount);
	  } else {
	    fprintf(stderr, "shift_frequency_cc parameter error\n");
	    doneProcessing = true;
	  }
          break;
      }
      case 5: {
          float cutOffFrequency;
          int M;
          int amount;
	  int N;
	  if (argc == 7) {
            sscanf(argv[2], "%f", &cutOffFrequency);
            sscanf(argv[3], "%d", &M);
            sscanf(argv[4], "%d", &amount);
	    sscanf(argv[5], "%d", &N);
            doneProcessing = !dsppInstance.decimate_cc(cutOffFrequency, M, amount, N, argv[6]);
	  } else {
	    fprintf(stderr, "decimate_cc parameter error\n");
	    doneProcessing = true;
	  }
          break;
      }
      case 6: {
        doneProcessing = !dsppInstance.fmdemod_cf();
        break;
      }
      case 7: {
          float cutOffFrequency;
          int M;
          int amount;
	  int N;
	  if (argc == 7) {
            sscanf(argv[2], "%f", &cutOffFrequency);
            sscanf(argv[3], "%d", &M);
            sscanf(argv[4], "%d", &amount);
	    sscanf(argv[5], "%d", &N);
            doneProcessing = !dsppInstance.decimate_ff(cutOffFrequency, M, amount, N, argv[6]);
	  } else {
	    doneProcessing = true;
	  }
          break;
      }
      case 8: {
        doneProcessing = !dsppInstance.convert_f_uInt16();
        break;
      }
      case 9: {
        doneProcessing = !dsppInstance.convert_f_sInt16();
        break;
      }
      case 10: {
        int port;
        int frequency = 0;
        int sampleRate = 0;
        int mode = 0;
        int gain = 0;
	if (argc == 8 || argc == 4) {
          sscanf(argv[3], "%d", &port);
	  if (argc == 8) {
            sscanf(argv[4], "%d", &frequency);
	    sscanf(argv[5], "%d", &sampleRate);
            sscanf(argv[6], "%d", &mode);
            sscanf(argv[7], "%d", &gain);
	  }
          fprintf(stderr, "calling tcp client\n");
          doneProcessing = !dsppInstance.convert_tcp_byte(argv[2], port, frequency, sampleRate, mode, gain);
	} else {
	  fprintf(stderr, "convert_tcp_byte parameter error\n");
	  doneProcessing = true;
	}
        break;
      }
      case 11: {
        int port;
	if (argc == 4) {
          sscanf(argv[3], "%d", &port);
          fprintf(stderr, "starting TCP server\n");
          doneProcessing = !dsppInstance.convert_byte_tcp(argv[2], port);
	} else {
	  fprintf(stderr, "convert_byte_tcp parameter error\n");
	  doneProcessing = true;
	}
        break;
      }
      case 12: {
        int M;
        int N;
	if (argc == 6 && !strncmp("CUSTOM", argv[5], strlen("CUSTOM"))) {
          sscanf(argv[3], "%d", &M);
          sscanf(argv[4], "%d", &N);
          fprintf(stderr, "starting custom FIR filter\n");
          doneProcessing = !dsppInstance.custom_fir_ff(argv[2], M, N, FIRFilter::CUSTOM);
	} else {
	  fprintf(stderr, "custom_fir_ff parameter error\n");
          fprintf(stderr, "%d %s\n", argc, argv[5]);
	  doneProcessing = true;
	}
        break;
      }
      case 13: {
        int M;
        int N;
	if (argc == 6 && !strncmp("CUSTOM", argv[5], strlen("CUSTOM"))) {
          sscanf(argv[3], "%d", &M);
          sscanf(argv[4], "%d", &N);
          fprintf(stderr, "starting custom FIR filter\n");
          doneProcessing = !dsppInstance.custom_fir_cc(argv[2], M, N, FIRFilter::CUSTOM);
	} else {
	  fprintf(stderr, "custom_fir_cc parameter error\n");
          fprintf(stderr, "%d %s\n", argc, argv[5]);
	  doneProcessing = true;
	}
        break;
      }
      case 14: {
	if (argc == 2) {
          fprintf(stderr, "starting real to complex quadrature \n");
          doneProcessing = !dsppInstance.real_to_complex_fc();
	} else {
	  fprintf(stderr, "real_to_complex_fc parameter error\n");
          fprintf(stderr, "%d\n", argc);
	  doneProcessing = true;
	}
        break;
      }
      case 15: {
        float sampleRate;
        if (argc == 3) {
          sscanf(argv[2], "%f", &sampleRate);
          FMMod modulator(sampleRate);
          doneProcessing = !modulator.modulate();
        } else {
	  fprintf(stderr, "fmmod_fc parameter error\n");
          fprintf(stderr, "%d\n", argc);
	  doneProcessing = true;
        }
      }
      case 16: {
        int amount;
        if (argc == 3) {
          sscanf(argv[2], "%d", &amount);
          doneProcessing = !dsppInstance.head(amount);
        } else {
	  fprintf(stderr, "head parameter error\n");
          fprintf(stderr, "%d\n", argc);
	  doneProcessing = true;
        }
      }
      case 17: {
        int amount;
        if (argc == 3) {
          sscanf(argv[2], "%d", &amount);
          doneProcessing = !dsppInstance.tail(amount);
        } else {
	  fprintf(stderr, "tail parameter error\n");
          fprintf(stderr, "%d\n", argc);
	  doneProcessing = true;
        }
      }
      case 18: {
        doneProcessing = !dsppInstance.convert_sInt16_f();
        break;
      }
      case 19: {
        int numberOfComplexSamples = 0;
        if (argc == 3) {
          sscanf(argv[2], "%d", &numberOfComplexSamples);
          doneProcessing = !dsppInstance.fft_cc(numberOfComplexSamples);
        } else {
          fprintf(stderr, "fft_cc parameter error\n");
        }
        break;
      }
      case 20: {
        if (argc == 3) {
          fprintf(stderr, "Starting parallel stream: %s\n", argv[2]);
          doneProcessing = !dsppInstance.tee(argv[2]);
        } else {
          fprintf(stderr, "tee parameter error\n");
        }
        break;
      }
      case 21: {
        float cutoff = 0.3333333;
        int decimation = 1;
        if (argc == 3) {
          sscanf(argv[2], "%f", &cutoff);
          SFIRFilter sfilter(cutoff);
          fprintf(stderr, "Low pass smooth filter a complex stream with cutoff at %s of Nyquist:\n", argv[2]);
          sfilter.filterSignal();
          doneProcessing = true;
        } else if (argc == 4) {
          sscanf(argv[2], "%f", &cutoff);
          sscanf(argv[3], "%d", &decimation);
          SFIRFilter sfilter(cutoff, decimation);
          fprintf(stderr,
                  "Low pass smooth filter a complex stream with cutoff at %s of Nyquist - decimation of %s:\n",
                  argv[2], argv[3]);
          sfilter.filterSignal();
          doneProcessing = true;
        } else if (argc == 5) {
          sscanf(argv[2], "%f", &cutoff);
          sscanf(argv[3], "%d", &decimation);
          bool highPass = strcmp(argv[4], "true") == 0;
          SFIRFilter sfilter(cutoff, decimation, highPass);
          if (highPass) {
            fprintf(stderr,
                    "High pass smooth filter a complex stream with cutoff at %s of Nyquist - decimation of %s:\n",
                    argv[2], argv[3]);
          } else {
            fprintf(stderr,
                    "Low pass smooth filter a complex stream with cutoff at %s of Nyquist - decimation of %s:\n",
                    argv[2], argv[3]);
          }
          sfilter.filterSignal();
          doneProcessing = true;
        } else {
          fprintf(stderr, "sfir_cc parameter error\n");
        }
        break;
      }
      case 22: {
        float cutoff = 0.3333333;
        int decimation = 1;
        if (argc == 3) {
          sscanf(argv[2], "%f", &cutoff);
          SFIRFilter sfilter(cutoff, decimation, false, false);
          fprintf(stderr, "Low pass smooth filter a real stream with cutoff at %s of Nyquist:\n", argv[2]);
          sfilter.filterSignal();
          doneProcessing = true;
        } else if (argc == 4) {
          sscanf(argv[2], "%f", &cutoff);
          sscanf(argv[3], "%d", &decimation);
          SFIRFilter sfilter(cutoff, decimation, false, false);
          fprintf(stderr,
                  "Low pass smooth filter a real stream with cutoff at %s of Nyquist - decimation of %s:\n",
                  argv[2], argv[3]);
          sfilter.filterSignal();
          doneProcessing = true;
        } else if (argc == 5) {
          sscanf(argv[2], "%f", &cutoff);
          sscanf(argv[3], "%d", &decimation);
          bool highPass = strcmp(argv[4], "true") == 0;
          SFIRFilter sfilter(cutoff, decimation, highPass, false);
          if (highPass) {
            fprintf(stderr,
                    "High pass smooth filter a real stream with cutoff at %s of Nyquist - decimation of %s:\n",
                    argv[2], argv[3]);
          } else {
            fprintf(stderr,
                    "Low pass smooth filter a real stream with cutoff at %s of Nyquist - decimation of %s:\n",
                    argv[2], argv[3]);
          }
          sfilter.filterSignal();
          doneProcessing = true;
        } else {
          fprintf(stderr, "sfir_ff parameter error\n");
        }
        break;
      }
      case 23: {
	if (argc == 2) {
          fprintf(stderr, "starting real of complex\n");
          doneProcessing = !dsppInstance.real_of_complex_cf();
	} else {
	  fprintf(stderr, "real_of_complex_cf parameter error\n");
          fprintf(stderr, "%d\n", argc);
	  doneProcessing = true;
	}
        break;
      }
      case 24: {
	if (argc == 2) {
          fprintf(stderr, "direct to complex IQ\n");
          doneProcessing = !dsppInstance.direct_to_iq();
	} else {
	  fprintf(stderr, "direct to comple IQ parameter error\n");
          fprintf(stderr, "%d\n", argc);
	  doneProcessing = true;
	}
        break;
      }
      case 25: {
        int decimation = 1;
        if (argc == 3) {
          sscanf(argv[2], "%d", &decimation);
          CFilter cfilter(decimation);
          fprintf(stderr, "Comb filter a complex stream with decimation: %s\n", argv[2]);
          cfilter.filterSignal();
          doneProcessing = true;
	} else {
	  fprintf(stderr, "Comb filter parameter error\n");
	  doneProcessing = true;
	}
        break;
      }
      case 26: {
        if (argc == 2) {
	  fprintf(stderr, "starting mag\n");
          doneProcessing = !dsppInstance.mag_cf();
	} else {
	  fprintf(stderr, "mag should have no parameters - error\n");
	  doneProcessing = true;
	}
        break;
      }
      case 27: {
        if (argc == 3) {
	  fprintf(stderr, "starting gain\n");
          float gain = 0.0;
          sscanf(argv[2], "%f", &gain);
          doneProcessing = !dsppInstance.gain(gain);
	} else {
	  fprintf(stderr, "gain should have one floating point parameter - error\n");
	  doneProcessing = true;
	}
        break;
      }
      case 28: {
        if (argc == 2) {
	  fprintf(stderr, "starting limit_real_stream\n");
          doneProcessing = !dsppInstance.limit_real_stream();
	} else {
	  fprintf(stderr, "limit_real_stream should have no parameters - error\n");
	  doneProcessing = true;
	}
        break;
      }
      case 29: {
        if (argc == 2) {
          const int BUFFER_SIZE = 4096;
	  fprintf(stderr, "starting dc_removal\n");
          float buffer[BUFFER_SIZE];
          for (int i = 0; i < BUFFER_SIZE; i++) buffer[i] = 0.0;
          doneProcessing = !dsppInstance.dc_removal(buffer, BUFFER_SIZE);
	} else {
	  fprintf(stderr, "dc_removal should have no parameters - error\n");
	  doneProcessing = true;
	}
        break;
      }
      case 30: {
        float target = 0.0;
        if (argc == 3) {
	  fprintf(stderr, "starting agc\n");
          sscanf(argv[2], "%f", &target);
          doneProcessing = !dsppInstance.agc(target);
	} else {
	  fprintf(stderr, "agc should have only a target level parameter - error\n");
	  doneProcessing = true;
	}
        break;
      }
      default:
        return -2;
      }
    if (doneProcessing) {
      break;
    }
  }

  return 0;

}

/* ---------------------------------------------------------------------- */

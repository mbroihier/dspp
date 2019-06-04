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
        "  convert_byte_tcp         : convert a byte stream to a tcp byte stream\n";

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
  { "convert_tcp_byte"         ,no_argument, NULL, 10 },
  { "convert_byte_tcp"         , no_argument, NULL, 11 },
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
      fprintf(stderr, "Short data stream\n");
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
      fprintf(stderr, "Short data stream\n");
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
      fprintf(stderr, "Short data stream\n");
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
      fprintf(stderr, "Short data stream\n");
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
 *      convert_tcp_byte.cc -- DSP Pipe - convert a TCP stream connection to a 
 *                                      byte(generic) stream
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
int dspp::convert_tcp_byte(const char * IPAddress, int port, int frequency, int sampleRate) {
  fprintf(stderr, "Creating a client object\n");
  RTLTCPClient client(IPAddress, port, frequency, sampleRate);
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

int main(int argc, char *argv[]) {

  dspp dsppInstance;

  int c;

  const int COMMAND_LENGTH = 32;

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
	if (argc == 6 || argc == 4) {
          sscanf(argv[3], "%d", &port);
	  if (argc == 6) {
            sscanf(argv[4], "%d", &frequency);
	    sscanf(argv[5], "%d", &sampleRate);
	  }
          fprintf(stderr, "calling tcp client\n");
          doneProcessing = !dsppInstance.convert_tcp_byte(argv[2], port, frequency, sampleRate);
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

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


/* ---------------------------------------------------------------------- */

int convert_byteLE_int16();
int convert_aByte_f();
int convert_aUnsignedByte_f();
int shift_frequency_cc(float cyclesPerSample);
int decimate_cc(float cutOffFrequency, int M, int amount, int N, const char * window);
int fmdemod_cf();
int decimate_ff(float cutOffFrequency, int M, int amount, int N, const char * window);
int convert_f_unsignedShort();
int convert_f_signedShort();
int convert_tcp_aUnsignedByte(const char * IPAddress, int port, int frequency, int sampleRate);

static const char USAGE_STR[] = "\n"
        "Usage: %s <command> [ parameter 1 [ ... parameter n]]\n"
        "  -h                       : help\n"
        "  convert_byteLE_int16     : convert little endian byte stream to internal short ints\n"
        "  convert_aByte_f          : convert a signed byte stream to internal floating point\n"
        "  convert_aUnsignedByte_f  : convert a signed byte stream to internal floating point\n"
        "  shift_frequency_cc       : recenter a signal by x cycles per sample\n"
        "  decimate_cc              : replace every n samples with 1 (complex)\n"
        "  fmdemod_cf               : demodulate FM signal\n"
        "  decimate_ff              : replace every n samples with 1 (real)\n"
        "  convert_f_unsignedShort  : convert a float(real) stream into an unsigned short stream\n"
        "  convert_f_signedShort    : convert a float(real) stream into an signed short stream\n"
        "  convert_tcp_aUnsignedByte: convert a tcp stream into an unsigned byte stream\n";

static struct option longOpts[] = {
  { "convert_byteLE_int16"     , no_argument, NULL, 1 },
  { "convert_aByte_f"          , no_argument, NULL, 2 },
  { "convert_aUnsignedByte_f"  , no_argument, NULL, 3 },
  { "shift_frequency_cc"       , no_argument, NULL, 4 },
  { "decimate_cc"              , no_argument, NULL, 5 },
  { "fmdemod_cf"               , no_argument, NULL, 6 },
  { "decimate_ff"              , no_argument, NULL, 7 },
  { "convert_f_unsignedShort"  , no_argument, NULL, 8 },
  { "convert_f_signedShort"    , no_argument, NULL, 9 },
  { "convert_tcp_aUnsignedByte", no_argument, NULL, 10 },
  { NULL, 0, NULL, 0 }
};

int main(int argc, char *argv[]) {

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
        doneProcessing = !convert_byteLE_int16();
        break;
      }
      case 2: {
        doneProcessing = !convert_aByte_f();
        break;
      }
      case 3: {
        doneProcessing = !convert_aUnsignedByte_f();
        break;
      }
      case 4: {
          float amount;
	  if (argc == 3) {
            sscanf(argv[2], "%f", &amount);
            doneProcessing = !shift_frequency_cc(amount);
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
            doneProcessing = !decimate_cc(cutOffFrequency, M, amount, N, argv[6]);
	  } else {
	    fprintf(stderr, "decimate_cc parameter error\n");
	    doneProcessing = true;
	  }
          break;
      }
      case 6: {
        doneProcessing = !fmdemod_cf();
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
            doneProcessing = !decimate_ff(cutOffFrequency, M, amount, N, argv[6]);
	  } else {
	    doneProcessing = true;
	  }
          break;
      }
      case 8: {
        doneProcessing = !convert_f_unsignedShort();
        break;
      }
      case 9: {
        doneProcessing = !convert_f_signedShort();
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
          doneProcessing = !convert_tcp_aUnsignedByte(argv[2], port, frequency, sampleRate);
	} else {
	  fprintf(stderr, "convert_tcp_aUnsignedByte parameter error\n");
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

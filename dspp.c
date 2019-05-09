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
int decimate_cc(int amount);

static const char USAGE_STR[] = "\n"
        "Usage: %s <command> [ parameter 1 [ ... parameter n]]\n"
        "  -h                       : help\n"
        "  convert_byteLE_int16     : convert little endian byte stream to internal short ints\n"
        "  convert_aByte_f          : convert a signed byte stream to internal floating point\n"
        "  convert_aUnsignedByte_f  : convert a signed byte stream to internal floating point\n"
        "  shift_frequency_cc       : recenter a signal by x cycles per sample\n"
        "  decimate_cc              : replace every n samples with 1\n";

static struct option longOpts[] = {
  { "convert_byteLE_int16"   , no_argument, NULL, 1 },
  { "convert_aByte_f"        , no_argument, NULL, 2 },
  { "convert_aUnsignedByte_f", no_argument, NULL, 3 },
  { "shift_frequency_cc"     , no_argument, NULL, 4 },
  { "decimate_cc"            , no_argument, NULL, 5 },
  { NULL, 0, NULL, 0 }
};

int main(int argc, char *argv[]) {

  int c;

  const int COMMAND_LENGTH = 32;

  if (argc <= 1 || ! argv[1] || (strlen(argv[1]) >= COMMAND_LENGTH)) {
    fprintf(stderr, USAGE_STR, argv[0]);
    return -2;
  }

  char command[strlen(argv[1])+1];

  memset(command, 0, strlen(argv[1])+1);

  char * new_argv[argc];
  if (argc > 1) {
    if (argv[1][0] == '-') {
      sprintf(command, "%s", argv[1]);
    } else {
      sprintf(command, "--%s", argv[1]);
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

  while ((c = getopt_long(argc, new_argv, "h", longOpts, NULL)) >= 0 ) {
    switch (c) {
      case 'h': {
        fprintf(stderr, USAGE_STR, argv[0]);
        return -2;
      }
      case 1: {
        convert_byteLE_int16();
        break;
      }
      case 2: {
        convert_aByte_f();
        break;
      }
      case 3: {
        convert_aUnsignedByte_f();
        break;
      }
      case 4: {
          float amount; 
          sscanf(argv[2], "%f", &amount);
          shift_frequency_cc(amount);
          break;
      }
      case 5: {
          int amount; 
          sscanf(argv[2], "%d", &amount);
          decimate_cc(amount);
          break;
      }
      default:
	return -2;
      }
    }

  return 0;

}

/* ---------------------------------------------------------------------- */

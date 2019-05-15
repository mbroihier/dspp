/*
 *      shift_frequency_cc.c -- DSP Pipe - shift the frequency of a quadature
 *                              by amount cycles per sample
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "FIRFilter.h"
/* ---------------------------------------------------------------------- */

int decimate_cc(float cutOffFrequency, int M, int amount, const char * window) {
  const int BUFFER_SIZE = 4096;
  float f[BUFFER_SIZE];
  float of[BUFFER_SIZE];
  int count;
  float * fptr, * ofptr;
  FIRFilter filter(cutOffFrequency, M, amount, FIRFilter::HAMMING);

  if (amount > BUFFER_SIZE) {
    fprintf(stderr, "Error - decimation amount is too large\n");
    return -1;
  } else if (amount < 1) {
    fprintf(stderr, "Can not decimate by 0\n");
    return -1;
  }

  int modBufferSizeBytes = ((BUFFER_SIZE / (2*amount)) * amount * 2)*sizeof(float);
  int numberOfSamples = modBufferSizeBytes / (8 * amount); // original count decimated by amount
  int outputBufferSizeBytes = numberOfSamples * 8;

  fprintf(stderr, "There will be %d samples in the filtered and decimated output buffer\n", numberOfSamples);
  
  fcntl(STDIN_FILENO, F_SETPIPE_SZ, modBufferSizeBytes); 
  fcntl(STDOUT_FILENO, F_SETPIPE_SZ, outputBufferSizeBytes); 

  ofptr = of;
  int outputBufferCount = 0;

  for (;;) {
    count = fread(&f, sizeof(char), modBufferSizeBytes, stdin);
    if(count < modBufferSizeBytes) {
      fprintf(stderr, "Short data stream\n");
      fclose(stdout);
      return 0;
    }
    filter.filterSignal(f, of, numberOfSamples);
    fwrite(&of, sizeof(char), outputBufferSizeBytes, stdout);
  }
  return 0;

}

/* ---------------------------------------------------------------------- */

/*
 *      shift_frequency_cc.c -- DSP Pipe - shift the frequency of a quadature
 *                              by amount cycles per sample
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#define _GNU_SOURCE
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

  int modBufferSize = (BUFFER_SIZE / (2*amount)) * amount * 2;
  int numberOfSamples = modBufferSize / 4;
  fcntl(STDIN_FILENO, F_SETPIPE_SZ, modBufferSize); 
  fcntl(STDOUT_FILENO, F_SETPIPE_SZ, BUFFER_SIZE); 

  ofptr = of;
  int outputBufferCount = 0;

  for (;;) {
    count = fread(&f, sizeof(float), modBufferSize, stdin);
    if(count < modBufferSize) {
      fprintf(stderr, "Short data stream\n");
      fclose(stdout);
      return 0;
    }
    filter.filterSignal(f, of, numberOfSamples);
    fwrite(&of, sizeof(float), numberOfSamples/amount, stdin);
    /*
    fptr = f;
    for (int i=0; i < modBufferSize; i+=2*amount) {
      *ofptr++ = *fptr++;
      *ofptr++ = *fptr++;
      fptr += (amount-1)*2;
      outputBufferCount += 2;
      if (outputBufferCount >= BUFFER_SIZE) {
        fwrite(&of, sizeof(float), BUFFER_SIZE, stdout);
        outputBufferCount = 0;
        ofptr = of;
      }
    }
    */
  }
  return 0;

}

/* ---------------------------------------------------------------------- */

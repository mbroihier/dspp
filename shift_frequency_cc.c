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
/* ---------------------------------------------------------------------- */

int shift_frequency_cc(float amount) {
  const int BUFFER_SIZE = 4096;
  float f[BUFFER_SIZE];
  float of[BUFFER_SIZE];
  int count;
  float * fptr, * ofptr;
  fcntl(STDIN_FILENO, F_SETPIPE_SZ, BUFFER_SIZE); 
  fcntl(STDOUT_FILENO, F_SETPIPE_SZ, BUFFER_SIZE); 
  float sinDeltaAmount = sin(amount*2.0*PI);
  float cosDeltaAmount = cos(amount*2.0*PI);
  float cosAmount = 1.0;
  float sinAmount = 0.0;
  float newCosAmount;
  float newSinAmount;

  float I;
  float Q;
  fprintf(stderr, "cycles per sample correction: %f\n", amount);
  for (;;) {
    count = fread(&f, sizeof(char), BUFFER_SIZE, stdin);
    if(count < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream\n");
      fclose(stdout);
      return 0;
    }
    fptr = f;
    ofptr = of;

    for (int i=0; i < BUFFER_SIZE; i+=sizeof(float)*2) {
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

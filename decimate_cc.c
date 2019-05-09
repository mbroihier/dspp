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

int decimate_cc(int amount) {
  const int BUFFER_SIZE = 4096;
  float f[BUFFER_SIZE];
  float of[BUFFER_SIZE];
  int count;
  float * fptr, * ofptr;
  fcntl(STDIN_FILENO, F_SETPIPE_SZ, BUFFER_SIZE); 
  fcntl(STDOUT_FILENO, F_SETPIPE_SZ, BUFFER_SIZE); 

  if (amount > BUFFER_SIZE) {
    fprintf(stderr, "Error - decimation amount is too large\n");
    return -1;
  } else if (amount < 1) {
    fprintf(stderr, "Can not decimate by 0\n");
    return -1;
  }

  int remainder = BUFFER_SIZE - amount*2;
  int outputBufferCount = 0;
  int correction;
  fprintf(stderr, "remainder: %d\n", remainder); 
  
  const int bump = (amount -1)*2*sizeof(float);

  fprintf(stderr, "bump: %d\n", bump);

  for (;;) {
    count = fread(&f, sizeof(float), BUFFER_SIZE, stdin);
    if(count < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream\n");
      fclose(stdout);
      return 0;
    }
    correction = (amount - (BUFFER_SIZE - remainder)/2)*2*sizeof(float);
    fprintf(stderr, "remainder: %d, correction: %d\n", remainder, correction); 
    fptr = f + correction;
    ofptr = of;

    for (int i=0; i < BUFFER_SIZE; i+=2*amount) {
      remainder = i;
      *ofptr++ = *fptr++;
      *ofptr++ = *fptr++;
      fptr += bump;
      fprintf(stderr, "new ofptr: %p, new fptr: %p\n", ofptr, fptr);
      outputBufferCount += 2;
      if (outputBufferCount >= BUFFER_SIZE) {
        fprintf(stderr, "remainder: %d\n", remainder); 
        fwrite(&of, sizeof(float), BUFFER_SIZE, stdout);
        outputBufferCount = 0;
      }
      fprintf(stderr, "outputBufferCount: %d\n", outputBufferCount);
    }
    fprintf(stderr, "need to read another buffer\n");
  }
  return 0;

}

/* ---------------------------------------------------------------------- */

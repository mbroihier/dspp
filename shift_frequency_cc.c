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
      fprintf(stderr, "Short data stream\n");
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

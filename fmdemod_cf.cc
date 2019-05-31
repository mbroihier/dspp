/*
 *      fmdemod_cf.c -- DSP Pipe - demodulate a FM signal
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

/* ---------------------------------------------------------------------- */

int fmdemod_cf() {
  const int BUFFER_SIZE = 2048;
  float f[BUFFER_SIZE];
  const int HALF_BUFFER_SIZE = BUFFER_SIZE / 2;
  float of[HALF_BUFFER_SIZE];
  float * fptr, * ofptr;
  int fSize = sizeof(f);
  int ofSize = sizeof(of);
  const float EPSILON = 1.0 / 128.0;

  //fcntl(STDIN_FILENO, F_SETPIPE_SZ, fSize); 
  //fcntl(STDOUT_FILENO, F_SETPIPE_SZ, ofSize);
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
      fprintf(stderr, "Short data stream\n");
      fclose(stdout);
      return 0;
    }
    ofptr = of;
    fptr = f;
    for (int i = 0; i < HALF_BUFFER_SIZE; i++) {
      I = *fptr++;
      Q = *fptr++;
      if ((abs(I) > EPSILON) || (abs(Q) > EPSILON)) {
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

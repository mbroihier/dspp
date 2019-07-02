/*
 *      FMMod - FM modulation class
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "FMMod.h"


/* ---------------------------------------------------------------------- */
FMMod::FMMod(float sampleRate) {
  this->sampleRate = sampleRate;
  this->fd = 5000.0;
  TWO_PI = 2.0 * M_PI;
  deltaT = 1.0 / sampleRate;
  inputBuffer  = (float *) malloc(sizeof(float));
  outputBuffer = (float *) malloc(sizeof(float)*2);
};


/* ---------------------------------------------------------------------- */
int FMMod::readSignalPipe() {
  int count = fread(inputBuffer, sizeof(float), 1, stdin);
  return count;
}

/* ---------------------------------------------------------------------- */
int FMMod::writeSignalPipe() {
  int count = fwrite(outputBuffer, sizeof(float), 2, stdout);
  return count;
}

/* ---------------------------------------------------------------------- */
int FMMod::modulate(){
 /*
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
 *  I = Ac*cos(s(t))
 *  Q = Ac*sin(s(t))
 */

  float integralSum = 0.0;
  float K = 2.0 * M_PI * fd * deltaT;

  for (;;) {
    if (readSignalPipe() != 1) {
      return 0;
    }
    integralSum += K * *inputBuffer;
    //while(integralSum > TWO_PI) integralSum -= TWO_PI;
    //while(integralSum < -TWO_PI) integralSum += TWO_PI;
    *outputBuffer     = cos(integralSum);
    *(outputBuffer+1) = sin(integralSum);
    writeSignalPipe();
  }
};


FMMod::~FMMod(void) {
  free(inputBuffer);
  free(outputBuffer);
};


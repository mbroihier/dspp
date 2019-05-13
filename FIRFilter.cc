/*
 *      fir_filter.h - FIR filter class
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "FIRFilter.h"

/* ---------------------------------------------------------------------- */
FIRFilter::FIRFilter(float cutoffFrequency, int M, int decimation, WindowType windowType) {
  this->decimation = decimation;
  this->M = M;
  // M = (int) 4.0 / transitionBW; // at the moment, I'm not adjusting this by the decimation
  if ((M % 2) == 0) {
    fprintf(stderr, "There must be an odd number of coefficients\n");
    exit(-1);
  }
  if (decimation > M) {
    fprintf(stderr, "The decimation factor must be greater than the number of filter coefficients\n");
    exit(-1);
  }
  float sinc[M];
  midPoint = M/2;
  //
  // sinc (x) = sin(x)/x, x != 0
  //          = 1, x == 0
  //
  const float K = 2.0 * M_PI * cutoffFrequency;
  for (int i = 1 ; i < midPoint ; i++) {
    float x = K * i;
    sinc[midPoint + i] = sinc[midPoint - i] = sin(x) / x;
  }
  sinc[midPoint] = 1.0;
  float window[M];
  const float K2 = 2.0 * M_PI / (M - 1.0);
  switch (windowType) {
    case HAMMING: {
      for (int i = 1; i < midPoint; i++) {
        window[midPoint + i] = window[midPoint - i] = 0.54 - 0.46 * cos(K2 * i);
      }
      window[midPoint] = 1.0;
    }
  }
  coefficients = (float *) malloc(sizeof(float)*M);
  oldInput = (float *) malloc(sizeof(float)*(M-1));
  oldestQ = M - 2;
  float accumulator = 0.0;
  float * coefficientReference = coefficients;
  for (int i = 0; i < M; i++) {
    *coefficientReference = window[i] * sinc[i]; // apply the window to the sinc
    accumulator += *coefficientReference++;
  }
  coefficientReference = coefficients;
  for (int i = 0; i < M; i++) {
    *coefficientReference++ = *coefficientReference / accumulator; // normalize the coefficients so the total gain is 1.0
  }
  for (int i = 0; i <= oldestQ; i++) {
    oldInput[i] = 0.0;
  }


};

void FIRFilter::filterSignal(float * input, float * output, int samples){

  float * I = input;
  float * Q = input + 1;
  float * signal = I;
  float sum = 0.0;
  for (int j = midPoint; j < M; j++) {
    sum += coefficients[j] * *signal++;
    if (j > 0) {
      sum += coefficients[j] * oldInput[j*2];
    }
  }
  *output++ = sum;
  signal = Q;
  for (int j = midPoint; j < M; j++) {
    sum += coefficients[j] * *signal++;
    if (j > 0) {
      sum += coefficients[j] * oldInput[j*2+1];
    }
  }
  *output++ = sum;
  sum = 0.0;
  for (int i = 1; i < samples; i++) {
    I += decimation;
    Q = I + 1;
    signal = I;
    for (int j = 0; j < M; j++) {
      sum += coefficients[j] * signal[j - midPoint];
    }
    *output++ = sum;
    sum = 0.0;
    signal = Q;
    for (int j = 0; j < M; j++) {
      sum += coefficients[j] * signal[j - midPoint];
    }
    *output++ = sum;
    sum = 0.0;
  }
  I += decimation; // is first location after buffer
  I--; //point at last Q
  for (int i = oldestQ; i > 0; i += 2) {
    oldInput[i] = *I--; // old Q
    oldInput[i-1] = *I--; // old I
  }
};

FIRFilter::~FIRFilter(void) {
  free(coefficients);
  free(oldInput);
};


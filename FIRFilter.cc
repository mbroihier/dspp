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
  if ((M % 2) == 0) {
    fprintf(stderr, "There must be an odd number of coefficients\n");
    exit(-1);
  }
  float sinc[M];
  midPoint = M/2;
  fprintf(stderr, "decimation factor: %d, number of filter coefficients: %d, midpoint: %d\n", decimation, M, midPoint);
  //
  // sinc (x) = sin(x)/x, x != 0
  //          = 1, x == 0
  //
  const float K = 2.0 * M_PI * cutoffFrequency;
  for (int i = 1 ; i <= midPoint ; i++) {
    float x = K * i;
    sinc[midPoint + i] = sinc[midPoint - i] = sin(x) / x;
    //fprintf(stderr, "sinc index %d and %d, value: %f\n", midPoint + i, midPoint - i, sinc[midPoint + i]);
  }
  sinc[midPoint] = 1.0;
  float window[M];
  const float K2 = 2.0 * M_PI / (M - 1.0);
  switch (windowType) {
    case HAMMING: {
      for (int i = 1; i <= midPoint; i++) {
        window[midPoint + i] = window[midPoint - i] = 0.54 - 0.46 * cos(K2 * (midPoint - i));
        //fprintf(stderr, "window index %d and %d, value: %f\n", midPoint + i, midPoint - i, window[midPoint + i]);
      }
      window[midPoint] = 1.0;
    }
  }
  coefficients = (float *) malloc(sizeof(float)*M);
  oldInput = (float *) malloc(sizeof(float)*M*2);
  oldestQ = 2*M - 1;
  float accumulator = 0.0;
  float * coefficientReference = coefficients;
  for (int i = 0; i < M; i++) {
    *coefficientReference = window[i] * sinc[i]; // apply the window to the sinc
    accumulator += *coefficientReference++;
  }
  coefficientReference = coefficients;
  for (int i = 0; i < M; i++) {
    *coefficientReference = *coefficientReference / accumulator; // normalize the coefficients so the total gain is 1.0
    coefficientReference++;
  }
  for (int i = 0; i <= oldestQ; i++) {
    oldInput[i] = 0.0;
  }
  fprintf(stderr, "FIR filter initialized\n");

  for (int i = 0; i < M; i++) {
    fprintf(stderr, "%d: %f\n", i, coefficients[i]);
  }

};

void FIRFilter::filterSignal(float * input, float * output, int samples){

  float * I = input;
  float * Q = input + 1;
  float * signal = I;
  float sum = 0.0;
  //fprintf(stderr, "first couple inputs: %f, %f, %f, %f\n", *input, *(input+1), *(input+2), *(input+3));
  //fprintf(stderr, "first couple outputs: %f, %f, %f, %f\n", *output, *(output+1), *(output+2), *(output+3));
  //fprintf(stderr, "starting to filter a buffer\n");

  //
  // Filter using previously stored input from the last buffer
  //

  float * oldSignal = oldInput + 2 * M -2 ;  // point at least old I
  for (int j = midPoint; j < M; j++) {
    sum += coefficients[j] * *signal;
    //fprintf(stderr, "working with I: %f\n", *signal);
    signal += 2;
    if (j != midPoint) {
      sum += coefficients[j] * *oldSignal;
      //fprintf(stderr, "working with oldI: %f\n", *oldSignal);
      oldSignal -= 2;
    }
  }
  *output++ = sum;
  sum = 0.0;
  signal = Q;
  oldSignal = oldInput + 2 * M - 1;
  for (int j = midPoint; j < M; j++) {
    sum += coefficients[j] * *signal;
    //fprintf(stderr, "working with Q: %f\n", *signal);
    signal += 2;
    if (j != midPoint) {
      sum += coefficients[j] * *oldSignal;
      //fprintf(stderr, "working with oldQ: %f\n", *oldSignal);
      oldSignal -= 2;
    }
  }
  *output++ = sum;

  //
  // Filter with this current buffer of input
  //

  sum = 0.0;
  for (int i = 1; i < samples; i++) {
    //fprintf(stderr, "doing sample %d of %d, output pointer is at: %p\n", i, samples, output);
    I += decimation*2;
    Q = I + 1;
    //fprintf(stderr, "I is pointing at: %p, value is: %f, Q is pointing at: %p, value is: %f\n", I, *I, Q, *Q);
    signal = I;
    for (int j = 0; j < M; j++) {
      sum += coefficients[j] * signal[(j - midPoint)*2];
    }
    *output++ = sum;
    sum = 0.0;
    signal = Q;
    for (int j = 0; j < M; j++) {
      sum += coefficients[j] * signal[(j - midPoint)*2];
    }
    *output++ = sum;
    sum = 0.0;
  }

  //
  // Copy the end of the buffer that will be used in the filtering
  // of the next buffer that arrives
  //

  I += decimation*2; // is first location after buffer
  I--; //point at last Q
  //fprintf(stderr, "copying the remaining input buffer\n");
  for (int i = oldestQ; i > 0; i -= 2) {
    oldInput[i] = *I--; // old Q
    oldInput[i-1] = *I--; // old I
    //fprintf(stderr, "oldInput %d: Q=%f, I=%f\n", i, oldInput[i], oldInput[i-1]);
  }
  //fprintf(stderr, "filtered a buffer\n");
};

FIRFilter::~FIRFilter(void) {
  free(coefficients);
  free(oldInput);
};


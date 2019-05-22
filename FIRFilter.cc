/*
 *      fir_filter.h - FIR filter class
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

#include "FIRFilter.h"

/* ---------------------------------------------------------------------- */
FIRFilter::FIRFilter(float cutoffFrequency, int M, int decimation, int N, WindowType windowType) {
  this->decimation = decimation;
  this->M = M;
  this->N = N;
  if ((M % 2) == 0) {
    fprintf(stderr, "There must be an odd number of coefficients\n");
    exit(-1);
  }

  int additionalSamplesNeeded = M / decimation;

  if ( additionalSamplesNeeded > 0 ) {
    fprintf(stderr, "Note: requested number of samples per buffer was %d, but the number of taps requires additional samples: %d\n", N, additionalSamplesNeeded);
    this->N = N + additionalSamplesNeeded;
    N = this->N;
  }

  // Always make the number of samples read a multiple of decimation factor and this number of samples must always be greater than the
  // number of coefficients.
  floatSamplesPerBufferToRead = N * decimation;
  if (floatSamplesPerBufferToRead < M) {
    fprintf(stderr, "Must always read enough samples to fill the filter taps\n");
    exit(-1);
  }
  INPUT_BUFFER_SIZE = (floatSamplesPerBufferToRead) * sizeof(float) * 2;
  SIGNAL_BUFFER_SIZE = INPUT_BUFFER_SIZE + M * sizeof(float) * 2; // always delay the last M samples for the next buffer read
  OUTPUT_BUFFER_SIZE = N * 2 * sizeof(float);
  fcntl(STDIN_FILENO, F_SETPIPE_SZ, INPUT_BUFFER_SIZE); 
  fcntl(STDOUT_FILENO, F_SETPIPE_SZ, OUTPUT_BUFFER_SIZE); 
  signalBuffer = (float *) malloc(SIGNAL_BUFFER_SIZE);
  outputBuffer = (float *) malloc(OUTPUT_BUFFER_SIZE);
  inputBuffer = signalBuffer + M*2; // buffer start for read
  int diff = inputBuffer - signalBuffer;
  fprintf(stderr, "inputBuffer location: %p, signalBuffer location: %p, difference: %d\n", inputBuffer, signalBuffer, diff);
  inputToDelay = inputBuffer + (INPUT_BUFFER_SIZE - M * sizeof(float) * 2) / sizeof(float);
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
  for (int i = 0; i < 2 * M; i++) {
    signalBuffer[i] = 0.0;
  }
  fprintf(stderr, "FIR filter initialized\n");

  for (int i = 0; i < M; i++) {
    fprintf(stderr, "%d: %f\n", i, coefficients[i]);
  }

};

FIRFilter::FIRFilter(float cutoffFrequency, int M, int decimation, int N, WindowType windowType, bool real) {
  this->decimation = decimation;
  this->M = M;
  this->N = N;
  this->real = true;
  if ((M % 2) == 0) {
    fprintf(stderr, "There must be an odd number of coefficients\n");
    exit(-1);
  }

  int additionalSamplesNeeded = M / decimation;

  if ( additionalSamplesNeeded > 0 ) {
    fprintf(stderr, "Note: requested number of samples per buffer was %d, but the number of taps requires additional samples: %d\n", N, additionalSamplesNeeded);
    this->N = N + additionalSamplesNeeded;
    N = this->N;
  }

  // Always make the number of samples read a multiple of decimation factor and this number of samples must always be greater than the
  // number of coefficients.
  floatSamplesPerBufferToRead = N * decimation;
  if (floatSamplesPerBufferToRead < M) {
    fprintf(stderr, "Must always read enough samples to fill the filter taps\n");
    exit(-1);
  }
  INPUT_BUFFER_SIZE = (floatSamplesPerBufferToRead) * sizeof(float);
  SIGNAL_BUFFER_SIZE = INPUT_BUFFER_SIZE + M * sizeof(float); // always delay the last M samples for the next buffer read
  OUTPUT_BUFFER_SIZE = N * sizeof(float);
  fcntl(STDIN_FILENO, F_SETPIPE_SZ, INPUT_BUFFER_SIZE); 
  fcntl(STDOUT_FILENO, F_SETPIPE_SZ, OUTPUT_BUFFER_SIZE); 
  signalBuffer = (float *) malloc(SIGNAL_BUFFER_SIZE);
  outputBuffer = (float *) malloc(OUTPUT_BUFFER_SIZE);
  inputBuffer = signalBuffer + M; // buffer start for read
  int diff = inputBuffer - signalBuffer;
  fprintf(stderr, "inputBuffer location: %p, signalBuffer location: %p, difference: %d\n", inputBuffer, signalBuffer, diff);
  inputToDelay = inputBuffer + (INPUT_BUFFER_SIZE - M * sizeof(float)) / sizeof(float);
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
  for (int i = 0; i < 2 * M; i++) {
    signalBuffer[i] = 0.0;
  }
  fprintf(stderr, "FIR filter initialized\n");

  for (int i = 0; i < M; i++) {
    fprintf(stderr, "%d: %f\n", i, coefficients[i]);
  }

};

int FIRFilter::readSignalPipe() {
  int count = fread(inputBuffer, sizeof(char), INPUT_BUFFER_SIZE, stdin);
  return count;
}

int FIRFilter::writeSignalPipe() {
  int count = fwrite(outputBuffer, sizeof(char), OUTPUT_BUFFER_SIZE, stdout);
  return count;
}

void FIRFilter::filterSignal(){

  float * I;
  float * Q;
  float * coefficientPtr;
  float * output;
  float sum;
  float sumI;
  float sumQ;

  //
  // Filter with this current buffer of input
  //

  if (real) {
    fprintf(stderr, "Object is initialized for real filter processing.\n");
    exit(-1);
  }
  for (;;) {
    if (readSignalPipe() != INPUT_BUFFER_SIZE) {
      fprintf(stderr, "Short read....\n");
      exit(-1);
    }
    //fprintf(stderr, "last old I is pointing at: %p, value is: %f, last old Q is pointing at: %p, value is: %f\n", inputBuffer-2, *(inputBuffer-2), inputBuffer-1, *(inputBuffer-1));
    //fprintf(stderr, "first new I is pointing at: %p, value is: %f, first new Q is pointing at: %p, value is: %f\n", inputBuffer, *(inputBuffer), inputBuffer+1, *(inputBuffer+1));

    I = inputBuffer - M / 2 * 2;
                                                  // set I to the first input
                                                  // that will be processed by
                                                  // coefficient[0]
    Q = I + 1;
    output = outputBuffer;
    coefficientPtr = coefficients;
    //for (int i = 0; i < N + extra - 1; i++) {
    for (int i = 0; i < N; i++) {
      //fprintf(stderr, "doing sample %d of %d, output pointer is at: %p\n", i, N + extra, output);
      //fprintf(stderr, "I is pointing at: %p, value is: %f, Q is pointing at: %p, value is: %f\n", I, *I, Q, *Q);
      sumI = 0.0;
      sumQ = 0.0;
      coefficientPtr = coefficients;
      for (int j = 0; j < M; j++) {
        //fprintf(stderr, "j is: %d, I is pointing at: %p, value is: %f, Q is pointing at: %p, value is: %f\n", j, I, *I, Q, *Q);
        sumI += *coefficientPtr * *I;
        sumQ += *coefficientPtr * *Q;
        coefficientPtr++;
        I += 2;
        Q += 2;
      }
      I += (decimation - M) * 2;
      Q = I + 1;
      *output++ = sumI;
      *output++ = sumQ;
    }

    //
    // Copy the end of the buffer that will be used in the filtering
    // of the next buffer that arrives
    //
    writeSignalPipe();
    memcpy(signalBuffer, inputToDelay, M * 8);
    // after copy, the end of the copied data should match up with the beginning of the input buffer, so
    // look at it after the read
    //fprintf(stderr, "filtered a buffer\n");
  }
};

void FIRFilter::filterReal(){

  float * I;
  float * coefficientPtr;
  float * output;
  float sumI;

  //
  // Filter with this current buffer of input
  //

  if (!real) {
    fprintf(stderr, "Object is not initialized for real filter processing.\n");
    exit(-1);
  }
  for (;;) {
    if (readSignalPipe() != INPUT_BUFFER_SIZE) {
      fprintf(stderr, "Short read....\n");
      exit(-1);
    }
    I = inputBuffer - M / 2;
                                                  // set I to the first input
                                                  // that will be processed by
                                                  // coefficient[0]
    output = outputBuffer;
    coefficientPtr = coefficients;
    for (int i = 0; i < N; i++) {
      //fprintf(stderr, "doing sample %d of %d, output pointer is at: %p\n", i, N + extra, output);
      sumI = 0.0;
      coefficientPtr = coefficients;
      for (int j = 0; j < M; j++) {
        //fprintf(stderr, "j is: %d, I is pointing at: %p, value is: %f\n", j, I, *I);
        sumI += *coefficientPtr * *I;
        coefficientPtr++;
        I += 1;
      }
      I += (decimation - M);
      *output++ = sumI;
    }

    //
    // Copy the end of the buffer that will be used in the filtering
    // of the next buffer that arrives
    //
    writeSignalPipe();
    memcpy(signalBuffer, inputToDelay, M * 4);
    // after copy, the end of the copied data should match up with the beginning of the input buffer, so
    // look at it after the read
    //fprintf(stderr, "filtered a buffer\n");
  }
};

FIRFilter::~FIRFilter(void) {
  free(coefficients);
  free(inputBuffer);
  free(outputBuffer);
};


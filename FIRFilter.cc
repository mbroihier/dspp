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
  this->real = false;
  if ((M % 2) == 0) {
    fprintf(stderr, "There must be an odd number of coefficients\n");
    exit(-1);
  }
  if (windowType == CUSTOM) {
    fprintf(stderr, "Conflicting FIR filter configuration request\n");
    exit(-1);
  }

  INPUT_BUFFER_SIZE = (N * decimation) * sizeof(float) * 2;
  SIGNAL_BUFFER_SIZE = INPUT_BUFFER_SIZE + (M - 1) * sizeof(float) * 2; // always delay the last M samples for the next buffer read
  OUTPUT_BUFFER_SIZE = N * 2 * sizeof(float);
  signalBuffer = (float *) malloc(SIGNAL_BUFFER_SIZE);
  outputBuffer = (float *) malloc(OUTPUT_BUFFER_SIZE);
  inputBuffer = signalBuffer + (M - 1)*2; // buffer start for read
  int diff = inputBuffer - signalBuffer;
  fprintf(stderr, "inputBuffer location: %p, signalBuffer location: %p, difference: %d\n", inputBuffer, signalBuffer, diff);
  inputToDelay = inputBuffer + (INPUT_BUFFER_SIZE - (M - 1)* sizeof(float) * 2) / sizeof(float);
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
  const float K3 = 4.0 * M_PI / (M - 1.0);
  switch (windowType) {
    case HAMMING: {
      for (int i = 1; i <= midPoint; i++) {
        window[midPoint + i] = window[midPoint - i] = 0.54 - 0.46 * cos(K2 * (midPoint - i));
        //fprintf(stderr, "window index %d and %d, value: %f\n", midPoint + i, midPoint - i, window[midPoint + i]);
      }
      window[midPoint] = 1.0;
      break;
    }
    case BLACKMAN: {
      for (int i = 1; i <= midPoint; i++) {
        window[midPoint + i] = window[midPoint - i] = 0.42 - 0.5 * cos(K2 * (midPoint - i) + 0.08 * cos(K3 * (midPoint - 1)));
        //fprintf(stderr, "window index %d and %d, value: %f\n", midPoint + i, midPoint - i, window[midPoint + i]);
      }
      window[midPoint] = 1.0;
      break;
    }
    default: {
      fprintf(stderr, "Internal error, should not ever get here!\n");
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
  if (windowType == CUSTOM) {
    fprintf(stderr, "Conflicting FIR filter configuration request\n");
    exit(-1);
  }

  INPUT_BUFFER_SIZE = (N * decimation) * sizeof(float);
  SIGNAL_BUFFER_SIZE = INPUT_BUFFER_SIZE + (M - 1) * sizeof(float); // always delay the last M-1 samples for the next buffer read
  fprintf(stderr, "INPUT_BUFFER_SIZE = %d, SIGNAL_BUFFER_SIZE = %d\n", INPUT_BUFFER_SIZE, SIGNAL_BUFFER_SIZE);
  OUTPUT_BUFFER_SIZE = N * sizeof(float);
  signalBuffer = (float *) malloc(SIGNAL_BUFFER_SIZE);
  outputBuffer = (float *) malloc(OUTPUT_BUFFER_SIZE);
  inputBuffer = signalBuffer + M - 1; // buffer start for read
  int diff = inputBuffer - signalBuffer;
  fprintf(stderr, "inputBuffer location: %p, signalBuffer location: %p, difference: %d\n", inputBuffer, signalBuffer, diff);
  inputToDelay = inputBuffer + (INPUT_BUFFER_SIZE - (M - 1) * sizeof(float)) / sizeof(float);
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
  const float K3 = 4.0 * M_PI / (M - 1.0);
  switch (windowType) {
    case HAMMING: {
      for (int i = 1; i <= midPoint; i++) {
        window[midPoint + i] = window[midPoint - i] = 0.54 - 0.46 * cos(K2 * (midPoint - i));
        //fprintf(stderr, "window index %d and %d, value: %f\n", midPoint + i, midPoint - i, window[midPoint + i]);
      }
      window[midPoint] = 1.0;
      break;
    }
    case BLACKMAN: {
      for (int i = 1; i <= midPoint; i++) {
        window[midPoint + i] = window[midPoint - i] = 0.42 - 0.5 * cos(K2 * (midPoint - i) + 0.08 * cos(K3 * (midPoint - 1)));
        //fprintf(stderr, "window index %d and %d, value: %f\n", midPoint + i, midPoint - i, window[midPoint + i]);
      }
      window[midPoint] = 1.0;
      break;
    }
    default: {
      fprintf(stderr, "Internal error, should not ever get here!\n");
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
  for (int i = 0; i < M; i++) {
    signalBuffer[i] = 0.0;
  }
  fprintf(stderr, "FIR filter initialized\n");

  for (int i = 0; i < M; i++) {
    fprintf(stderr, "%d: %f\n", i, coefficients[i]);
  }

};

FIRFilter::FIRFilter(const char * filePath, int M, int N, WindowType windowType) {
  this->decimation = 1;
  this->M = M;
  this->N = N;
  this->real = false;
  if ((M % 2) == 0) {
    fprintf(stderr, "There must be an odd number of coefficients\n");
    exit(-1);
  }
  if (windowType != CUSTOM) {
    fprintf(stderr, "Conflicting FIR filter configuration request\n");
    exit(-1);
  }

  INPUT_BUFFER_SIZE = (N * decimation) * sizeof(float) * 2;
  SIGNAL_BUFFER_SIZE = INPUT_BUFFER_SIZE + (M - 1) * sizeof(float) * 2; // always delay the last M-1 samples for the next buffer read
  fprintf(stderr, "INPUT_BUFFER_SIZE = %d, SIGNAL_BUFFER_SIZE = %d\n", INPUT_BUFFER_SIZE, SIGNAL_BUFFER_SIZE);
  OUTPUT_BUFFER_SIZE = N * 2 *sizeof(float);
  signalBuffer = (float *) malloc(SIGNAL_BUFFER_SIZE);
  outputBuffer = (float *) malloc(OUTPUT_BUFFER_SIZE);
  inputBuffer = signalBuffer + (M - 1)*2; // buffer start for read
  int diff = inputBuffer - signalBuffer;
  fprintf(stderr, "inputBuffer location: %p, signalBuffer location: %p, difference: %d\n", inputBuffer, signalBuffer, diff);
  inputToDelay = inputBuffer + (INPUT_BUFFER_SIZE - (M - 1) * sizeof(float) * 2) / sizeof(float);
  midPoint = M/2;
  fprintf(stderr, "decimation factor: %d, number of filter coefficients: %d, midpoint: %d\n", decimation, M, midPoint);

  coefficients = (float *) malloc(sizeof(float)*M);

  FILE * filePtr = fopen(filePath, "r");
  if (!filePtr) {
    fprintf(stderr, "Custom coefficient file does not exist.\n");
    exit(-1);
  }
  for (int i = 0; i < M; i++) {
    if (! fscanf(filePtr, "%f", &coefficients[i])) {
      fprintf(stderr, "Not enough coefficients\n");
      exit(-1);
    }
  }
  fclose(filePtr); 
  
  for (int i = 0; i < 2 * M; i++) {
    signalBuffer[i] = 0.0;
  }
  fprintf(stderr, "FIR filter initialized\n");

  for (int i = 0; i < M; i++) {
    fprintf(stderr, "%d: %f\n", i, coefficients[i]);
  }

};

FIRFilter::FIRFilter(const char * filePath, int M, int N, WindowType windowType, bool real) {
  this->decimation = 1;
  this->M = M;
  this->N = N;
  this->real = true;
  if ((M % 2) == 0) {
    fprintf(stderr, "There must be an odd number of coefficients\n");
    exit(-1);
  }
  if (windowType != CUSTOM) {
    fprintf(stderr, "Conflicting FIR filter configuration request\n");
    exit(-1);
  }

  INPUT_BUFFER_SIZE = (N * decimation) * sizeof(float);
  SIGNAL_BUFFER_SIZE = INPUT_BUFFER_SIZE + (M - 1) * sizeof(float); // always delay the last M-1 samples for the next buffer read
  fprintf(stderr, "INPUT_BUFFER_SIZE = %d, SIGNAL_BUFFER_SIZE = %d\n", INPUT_BUFFER_SIZE, SIGNAL_BUFFER_SIZE);
  OUTPUT_BUFFER_SIZE = N * sizeof(float);
  signalBuffer = (float *) malloc(SIGNAL_BUFFER_SIZE);
  outputBuffer = (float *) malloc(OUTPUT_BUFFER_SIZE);
  inputBuffer = signalBuffer + M - 1; // buffer start for read
  int diff = inputBuffer - signalBuffer;
  fprintf(stderr, "inputBuffer location: %p, signalBuffer location: %p, difference: %d\n", inputBuffer, signalBuffer, diff);
  inputToDelay = inputBuffer + (INPUT_BUFFER_SIZE - (M - 1) * sizeof(float)) / sizeof(float);
  midPoint = M/2;
  fprintf(stderr, "decimation factor: %d, number of filter coefficients: %d, midpoint: %d\n", decimation, M, midPoint);

  coefficients = (float *) malloc(sizeof(float)*M);

  FILE * filePtr = fopen(filePath, "r");
  if (!filePtr) {
    fprintf(stderr, "Custom coefficient file does not exist.\n");
    exit(-1);
  }
  for (int i = 0; i < M; i++) {
    if (! fscanf(filePtr, "%f", &coefficients[i])) {
      fprintf(stderr, "Not enough coefficients\n");
      exit(-1);
    }
  }
  fclose(filePtr); 
  
  for (int i = 0; i < M; i++) {
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

    I = signalBuffer;
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
    memcpy(signalBuffer, inputToDelay, (M-1) * 8);
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
    I = signalBuffer;
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
    memcpy(signalBuffer, inputToDelay, (M-1) * 4);
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


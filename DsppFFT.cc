/*
 *      DsppFFT.cc - FFTW3 wrappers class for dspp
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include "DsppFFT.h"
/* ---------------------------------------------------------------------- */
DsppFFT::DsppFFT(int size){
  numberOfSamples = size;
  signal = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*numberOfSamples);
  rawSignal = (float *) fftw_malloc(sizeof(float)*numberOfSamples*2);
  signalInFreqDomain = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*numberOfSamples);
  rawSignalInFreqDomain = (float *) fftw_malloc(sizeof(float)*numberOfSamples*2);
  plan = fftw_plan_dft_1d(numberOfSamples, signal, signalInFreqDomain, FFTW_FORWARD, FFTW_ESTIMATE);
};

int DsppFFT::processSampleSet() {
  int count = 0;
  float * floatPtr;
  double * doublePtr;
  for (;;) {
    count = fread(rawSignal, sizeof(float), numberOfSamples*2, stdin);
    if (count == numberOfSamples*2) {
      floatPtr = rawSignal;
      doublePtr = (double *) signal;
      for (int index = 0; index < numberOfSamples; index++) {
        *doublePtr++ = *floatPtr++;
        *doublePtr++ = *floatPtr++;
      }
      fftw_execute(plan);
      floatPtr = rawSignalInFreqDomain;
      doublePtr = (double *) signalInFreqDomain;
      for (int index = 0; index < numberOfSamples; index++) {
        *floatPtr++ = *doublePtr++;
        *floatPtr++ = *doublePtr++;
      }
    } else {
      fprintf(stderr, "short pipe, fft_cc\n");
      break;
    }
    fwrite(rawSignalInFreqDomain, sizeof(float), numberOfSamples*2, stdout);
  }
  return 1; // pipe terminated - typically ok
}

DsppFFT::~DsppFFT(void){
  if (plan) fftw_destroy_plan(plan);
  if (signal) fftw_free(signal);
  if (signalInFreqDomain) fftw_free(signalInFreqDomain);
};



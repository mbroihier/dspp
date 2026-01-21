/*
 *      RealToQuadrature.cc - Convert a real signal into its complex form
 *
 *      Copyright (C) 2026
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <cstring>
#include "RealToQuadrature.h"
/* ---------------------------------------------------------------------- */
RealToQuadrature::RealToQuadrature(int size){
  numberOfSamples = size;
  rawSignal = (float *) malloc(sizeof(float)*numberOfSamples);
  outSignal = (float *) malloc(sizeof(float)*numberOfSamples*8);
  signal = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*numberOfSamples);
  signalInFreqDomain = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*numberOfSamples);
  hilbertOfRaw = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*numberOfSamples);
  signalInQuadrature = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*numberOfSamples);
  plan = fftw_plan_dft_1d(numberOfSamples, signal, signalInFreqDomain, FFTW_FORWARD, FFTW_ESTIMATE);
  iplan = fftw_plan_dft_1d(numberOfSamples, signalInFreqDomain, hilbertOfRaw, FFTW_BACKWARD,
                           FFTW_ESTIMATE);
};

int RealToQuadrature::processSampleSetHilbert() {
  int count = 0;
  float * rawPtr;
  float * floatPtr;
  double * doublePtr;
  fprintf(stderr, "signalInFreqDomain addr: %p, offset by size: %p, %p\n", signalInFreqDomain, signalInFreqDomain[numberOfSamples], signalInFreqDomain+numberOfSamples);
  for (;;) {
    count = fread(rawSignal, sizeof(float), numberOfSamples, stdin);
    if (count == numberOfSamples) {
      floatPtr = rawSignal;
      doublePtr = (double *) signal;
      for (int index = 0; index < numberOfSamples; index++) {
        *doublePtr++ = *floatPtr++;
        *doublePtr++ = 0.0; // set imaginary part to zero
      }
      // take FFT
      fftw_execute(plan);
      // clear negative frequencies
      doublePtr = (double *) signalInFreqDomain;  // set pointer to negative frequencies
      for (int index = 0; index < numberOfSamples; index++) {
        if (index == 0) {
          doublePtr++;  // skip
          doublePtr++;  // skip (ie leave DC alone
        } else {
          if (index < numberOfSamples/2) {
            *doublePtr++ *= 2.0;  // double positive frequencies
            *doublePtr++ *= 2.0;
          } else {
            *doublePtr++ = 0.0;  // zero real part
            *doublePtr++ = 0.0;  // zero imaginary part
          }
        }
      }
      // take the inverse - this will be the Hilbert Transform
      fftw_execute(iplan);
      floatPtr = outSignal;
      rawPtr = rawSignal;
      doublePtr = (double *) hilbertOfRaw;
      // combine the raw input with the signal modified by the Hilbert transform
      // IQ = raw + j*H(raw)
      for (int index = 0; index < numberOfSamples; index++) {
        *floatPtr++ = *rawPtr++ - doublePtr[1];
        *floatPtr++ = doublePtr[0];
        doublePtr++;
        doublePtr++;
      }
    } else {
      fprintf(stderr, "short pipe, processSampleSet\n");
      break;
    }
    fwrite(outSignal, sizeof(float), numberOfSamples*2, stdout);
  }
  return 1; // pipe terminated - typically ok
}

int RealToQuadrature::processSampleSetDownconversion() {
  int count = 0;
  float * inFloatPtr;
  float * outFloatPtr;
  for (;;) {
    count = fread(rawSignal, sizeof(float), numberOfSamples, stdin);
    if (count == numberOfSamples) {
      inFloatPtr = rawSignal;
      outFloatPtr = outSignal;
      for (int index = 0; index < numberOfSamples; index++) {
        //
        int mv = index % 4;
        switch (mv) {
        case 0 : {
          *outFloatPtr++ = *inFloatPtr++; // R 
          *outFloatPtr++ = 0.0; // I set imaginary part to zero
          break;
        }
        //
        case 1 : {
          *outFloatPtr++ = 0.0; // R set real part to zero
          *outFloatPtr++ = - *inFloatPtr++;  // I write out -j
          break;
        }
         //
        case 2 : {
          *outFloatPtr++ = - *inFloatPtr++;  // R write out - of input
          *outFloatPtr++ = 0.0; // I set imaginary to zero
          break;
        }
        //
        case 3 : {
          *outFloatPtr++ = 0.0; // R set real to zero
          *outFloatPtr++ = *inFloatPtr++; // I write out input
          break;
        }
        default:
          fprintf(stderr, "logic error in RealToQuadrature processSignalSetFreqDoubling\n");
        }
      }
    } else {
      fprintf(stderr, "short pipe, processSampleSet\n");
      break;
    }
    fwrite(outSignal, sizeof(float), numberOfSamples*2, stdout);
  }
  return 1; // pipe terminated - typically ok
}

RealToQuadrature::~RealToQuadrature(void){
  if (plan) fftw_destroy_plan(plan);
  if (iplan) fftw_destroy_plan(iplan);
  if (signal) fftw_free(signal);
  if (signalInFreqDomain) fftw_free(signalInFreqDomain);
  if (hilbertOfRaw) fftw_free(hilbertOfRaw);
  if (signalInQuadrature) fftw_free(signalInQuadrature);
  if (rawSignal) free(rawSignal);
  if (outSignal) free(outSignal);
};

#ifdef DEBUG
int main(int argc, char *argv[]) {
  RealToQuadrature rtqo(256);
  if (argc == 1) {
    rtqo.processSampleSetFreqDoubling();
  } else {
    if (argc == 2 && strncmp(argv[1],"-H", 2) == 0) {
      rtqo.processSampleSetFFT();
    } else {
      fprintf(stderr, "Usage: RealToQuadrature [-H] - real to quadrature conversion. If -H, "
              "the Hilbert method is used.\n");
    }
  }
  return 0;
}
#endif

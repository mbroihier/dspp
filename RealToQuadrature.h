#ifndef REAL_TO_QUADRATURE_H_
#define REAL_TO_QUADRATURE_H_
/*
 *      RealToQuadrature.cc - various methods to convert a real signal
 *                              to complex
 *
 *      Copyright (C) 2026
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <fftw3.h>
/* ---------------------------------------------------------------------- */
class RealToQuadrature {

  protected:
  float * rawSignal;
  float * outSignal;
  fftw_complex * signal;
  fftw_complex * signalInFreqDomain;
  fftw_complex * hilbertOfRaw;
  fftw_complex * signalInQuadrature;
  int numberOfSamples; // number of samples in delay signal buffer
  fftw_plan plan;
  fftw_plan iplan;

  public:

  RealToQuadrature(int fftSize);

  int processSampleSetHilbert(void);
  int processSampleSetDownconversion(void);

  ~RealToQuadrature(void);
    
};
#endif  // REAL_TO_QUADRATURE_H_


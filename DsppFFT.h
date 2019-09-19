/*
 *      DsppFFT.cc - FFTW3 wrappers class for dspp
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <fftw3.h>
/* ---------------------------------------------------------------------- */
class DsppFFT {

  protected:
  fftw_complex * signal;
  fftw_complex * signalInFreqDomain;
  float * rawSignal;
  float * rawSignalInFreqDomain;
  int numberOfSamples; // number of samples in delay signal buffer
  fftw_plan plan;

  public:

  DsppFFT(int size);

  int processSampleSet(void);

  ~DsppFFT(void);
    
};


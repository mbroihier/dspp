/*
 *      shift_frequency_cc.c -- DSP Pipe - shift the frequency of a quadature
 *                              by amount cycles per sample
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "FIRFilter.h"
/* ---------------------------------------------------------------------- */

int decimate_cc(float cutOffFrequency, int M, int amount, int N, const char * window) {

  FIRFilter filter(cutOffFrequency, M, amount, N, FIRFilter::HAMMING);
  filter.filterSignal();
  
  return 0;

}

/* ---------------------------------------------------------------------- */

/*
 *      decimate_ff.c -- DSP Pipe - decimate real signal
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

int decimate_ff(float cutOffFrequency, int M, int amount, int N, const char * window) {

  FIRFilter filter(cutOffFrequency, M, amount, N, FIRFilter::HAMMING, true);
  filter.filterReal();
  
  return 0;

}

/* ---------------------------------------------------------------------- */

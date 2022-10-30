/*
 *      FindNLargestF.cc - find the N largest magnitude frequencies in a FFT
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "FindNLargestF.h"

/* ---------------------------------------------------------------------- */
void FindNLargestF::init(int size, int number) {
  this->size = size;
  this->number = number;
  binArray = reinterpret_cast<int *>(malloc(number * sizeof(int)));
  samples = reinterpret_cast<float *>(malloc(size * sizeof(float) * 2));
  mag = reinterpret_cast<float *>(malloc(size * sizeof(float)));
  sampleBufferSize = size * 2;
}

FindNLargestF::FindNLargestF(int size, int number) {
  init(size, number);
}

void FindNLargestF::doWork() {
  int frame = 0;
  int count = 0;
  float * samplePtr;
  float * magPtr;
  fprintf(stderr, "Find %d largest magnitude frequencies in FFT\n", number);
  bool done = false;
  while (!done) {
    count = fread(samples, sizeof(float), sampleBufferSize, stdin);
    if (count < sampleBufferSize) {
      done = true;
      continue;
    }
    samplePtr = samples;
    magPtr = mag;
    float peak = 0.0;
    // generate magnitude
    int bin = 0;
    for (int j = 0; j < size; j++) {
      float r = *samplePtr++;
      float i = *samplePtr++;
      *magPtr = sqrt(r*r + i*i);
      if (*magPtr > peak) {
        peak = *magPtr;
        binArray[0] = bin;
      }
      magPtr++;
      bin++;
    }
    fprintf(stderr, "bin should be 512, it is %d\n", bin);
    for (int binIndex = 1; binIndex < number; binIndex++) {
      float threshold = mag[binArray[binIndex - 1]];
      peak = 0.0;
      for (int frequencyIndex = 0; frequencyIndex < size; frequencyIndex++) {
        if ((mag[frequencyIndex] <= threshold) && (mag[frequencyIndex] > peak)) {
          bool notInBinArray = true;
          for (int checkIndex = 0; checkIndex < binIndex; checkIndex++) {
            if (frequencyIndex == binArray[checkIndex]) {
              notInBinArray = false;
            }
          }
          if (notInBinArray) {  // update peak
            peak = mag[frequencyIndex];
            bin = frequencyIndex;
          }
        }
      }
      binArray[binIndex] = bin;
    }
    fwrite(binArray, sizeof(int), number, stdout);
    fprintf(stderr, "binArray on frame %d\n", frame++);
    for (int binIndex = 0; binIndex < number; binIndex++) {
      fprintf(stderr, "binArray[%3d]: %3d, mag[binArray[%3d]: %f \n", binIndex, binArray[binIndex],
              binIndex, mag[binArray[binIndex]]);
    }
  }
}

FindNLargestF::~FindNLargestF(void) {
  if (mag) free(mag);
  if (binArray) free(binArray);
  if (samples) free(samples);
}


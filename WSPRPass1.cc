/*
 *      WSPRPass1.cc - find potential WSPR signals
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <algorithm>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "WSPRPass1.h"
#include "SpotCandidate.h"
#include "Fano.h"
#define SELFTEST 1

/* ---------------------------------------------------------------------- */
void WSPRPass1::init(int size, int number, char * prefix) {
  this->size = size;
  this->number = number;
  this->prefix = prefix;
  freq = 375.0;
  deltaFreq = freq / size;
  fprintf(stderr, "allocating binArray memory\n");
  binArray = reinterpret_cast<int *>(malloc(number * sizeof(int)));
  fprintf(stderr, "allocating samples memory\n");
  fftOverTime = reinterpret_cast<float *> (malloc(size * sizeof(float) * 2 * PROCESSING_SIZE));
  fprintf(stderr, "allocating mag memory\n");
  mag = reinterpret_cast<float *>(malloc(size * sizeof(float)));
  magAcc = reinterpret_cast<float *>(malloc(size * sizeof(float)));
    sampleBufferSize = size * 2;
  tic = 0;
  memset(magAcc, 0, size * sizeof(float));
  }

WSPRPass1::WSPRPass1(int size, int number, char * prefix) {
  fprintf(stderr, "creating WSPRPass1 object\n");
  init(size, number, prefix);
  fprintf(stderr, "done creating WSPRPass1 object\n");
}

void WSPRPass1::doWork() {
  std::map<int, SpotCandidate *> candidatesPass1;
  char hashtab[32768*13] = {0};  //EXPERIMENT

  int count = 0;
  float * samplePtr;
  float * magPtr;
  float * magAccPtr;
  float * fftOverTimePtr;
  fprintf(stderr, "Find WSPR signals pass 1\n");
  bool done = false;
  float wallClock = 0.0;
  float deltaTime = 1.0 / freq * size;
  fftOverTimePtr = fftOverTime;
  while (!done) {
    // get an FFT's worth of bins
    count = fread(fftOverTimePtr, sizeof(float), sampleBufferSize, stdin);
    fprintf(stderr, "done with read for tic %d, wall clock %7.2f\n", tic, wallClock);
    if (count < sampleBufferSize  || tic >= PROCESSING_SIZE) {
      done = true;
      continue;
    }
#ifdef SELFTEST
    samplePtr = fftOverTimePtr;
    fprintf(stderr, "SELFTEST for checking sample access is enabled\n");
    for (int bin = 0; bin < size; bin++) {
      float r = fftOverTime[tic * size * 2 + bin * 2];
      float i = fftOverTime[tic * size * 2 + bin * 2 + 1];
      if (r != *samplePtr) {
        fprintf(stderr, "SELFTEST error - r != input, bin: %d, tic: %d, r: %10.5f, input: %10.5f\n",
                bin, tic, r, *samplePtr);
        return;
      }
      samplePtr++;
      if (i != *samplePtr) {
        fprintf(stderr, "SELFTEST error - i != input, bin: %d, tic: %d, i: %10.5f, input: %10.5f\n",
                bin, tic, i, *samplePtr);
        return;
      }
      samplePtr++;
    }
#endif
    samplePtr = fftOverTimePtr;
    fftOverTimePtr += sampleBufferSize;
    magPtr = mag;
    magAccPtr = magAcc;
    // generate magnitude
    for (int j = 0; j < size; j++) {
      float r = *samplePtr++;
      float i = *samplePtr++;
      *magPtr = sqrt(r*r + i*i);
      *magAccPtr++ += *magPtr;
      magPtr++;
    }
    tic++;
    wallClock += deltaTime;
  }
  // where are the frequencies with the most energy?
  float peak = 0.0;
  for (int i = 0; i < size; i++) {
    if (magAcc[i] > peak) {
      peak = magAcc[i];
      binArray[0] = i;
    }
  }
  float threshold = peak;
  for (int i = 1; i < number; i++) {
    peak = 0.0;
    for (int j = 0; j < size; j++) {
      if (magAcc[j] > peak && magAcc[j] < threshold) {
        peak = magAcc[j];
        binArray[i] = j;
      }
    }
    threshold = peak;
  }
  for (int i = 0; i < size; i++) {
    bool inBinArray = false;
    for (int j = 0; j < number; j++) {
      if (binArray[j] == i) {
        inBinArray = true;
      }
    }
    if (inBinArray) {
      fprintf(stderr, "%3d: %12.0f *\n", i, magAcc[i]);
    } else {
      fprintf(stderr, "%3d: %12.0f\n", i, magAcc[i]);
    }
  }
  // Now let's assume that there is a WSPR signal at the peak locations.
  // Slide a 7 bin frequency window across each peak location starting at the largest peak.
  // For each window, calculate the centroid of the window for each time tic and store that information in a Spot
  // Candidate.  Develop a symbol for each tic and then scan down all the tics looking for something that Fano
  // decodes.
  for (int currentPeakIndex = 0; currentPeakIndex < number; currentPeakIndex++) {
    int currentPeakBin = binArray[currentPeakIndex];
    int freqBinsToProcess[SpotCandidate::WINDOW];
    int offset = SpotCandidate::WINDOW / 2;
    for (int i = -offset; i <= offset; i++) {
      if (i < 0) {
        freqBinsToProcess[i + offset] = ((currentPeakBin + i) >= 0) ? currentPeakBin + i : size + (currentPeakBin + i);
      } else {
        freqBinsToProcess[i + offset] = (currentPeakBin + i) % size;
      }
    }
#ifdef SELFTEST
    fprintf(stderr, "SELFTEST for checking bins to process for this peak\n");
    int lastBin = -1;
    for (int i = 0; i < SpotCandidate::WINDOW; i++) {
      fprintf(stderr, "freqBinsToProcess[%d]: %d\n", i, freqBinsToProcess[i]);
      if (lastBin < 0) {
        lastBin = freqBinsToProcess[i];
      } else {
        if (freqBinsToProcess[i] == lastBin + 1) {
          lastBin = freqBinsToProcess[i];
        } else {
          if (lastBin == 255  && freqBinsToProcess[i] == 0) {
            lastBin = 0;
          } else {
            fprintf(stderr, "Error, out of sequence\n");
          }
        }
      }
    }
#endif
    std::vector<SpotCandidate::SampleRecord> candidateInfo;
    for (int t = 0; t < tic; t++) {
      SpotCandidate::SampleRecord sr;
      sr.centroid = 0.0;
      sr.magnitude = 0.0;
      sr.magSlice.clear();
      sr.timeStamp = t;
      sr.timeSeconds = t * deltaTime;
      float acc = 0.0;
      float accBinLoc = 0.0;
      for (int bin = 0; bin < SpotCandidate::WINDOW; bin++) {
        float r = fftOverTime[t * size * 2 + freqBinsToProcess[bin] * 2];
        float i = fftOverTime[t * size * 2 + freqBinsToProcess[bin] * 2 + 1];
        float m = sqrt(r * r + i * i);
        sr.magSlice.push_back(m);
        sr.r.push_back(r);
        sr.i.push_back(i);
        acc += m;
        accBinLoc += bin * m;
      }
      if (acc > 1.0) {
        sr.centroid = accBinLoc / acc;
        sr.magnitude = acc;
        candidateInfo.push_back(sr);
      } else {
        fprintf(stderr, "Error - should always be able to generate a centroid\n");
        return;
      }
    }
    SpotCandidate candidate(currentPeakBin, candidateInfo, deltaFreq);
    //candidate.printReport();
    Fano fanoObject;
    unsigned char symbols[162];
    unsigned int metric;
    unsigned int cycles;
    unsigned int maxnp;
    unsigned char data[12];
    unsigned int nbits = 81;
    int delta = 60;
    unsigned int maxcycles = 10000;
    int numberOfShifts = candidateInfo.size() - NOMINAL_NUMBER_OF_SYMBOLS;
    for (int shift = 0; shift < numberOfShifts; shift++) {
      std::vector<SpotCandidate::SampleRecord> subset;
      for (int index = 0; index < NOMINAL_NUMBER_OF_SYMBOLS; index++) {
        subset.push_back(candidateInfo[index + shift]);
      }
      std::vector<int>  tokens;
      float snr = 0.0;
      float slope = 0.0;
      candidate.tokenize(subset, tokens, snr, slope);
      for (int index = 0; index < NOMINAL_NUMBER_OF_SYMBOLS; index++) {
        symbols[index] = tokens[index];
      }
      fprintf(stderr, "Deinterleave symbols\n");
      fanoObject.deinterleave(symbols);
      fprintf(stderr, "Performing Fano\n");
      if (fanoObject.fano(&metric, &cycles, &maxnp, data, symbols, nbits, delta, maxcycles)) {
        fprintf(stderr, "Did not decode peak bin: %d @ shift: %d, metric: %8.8x, cycles: %d, maxnp: %d\n",
                currentPeakBin, shift, metric, cycles, maxnp);
      } else {
        bool pass = false;
        for (auto c : data) {
          if (c != 0) pass = true;
        }
        if (pass) {
          fprintf(stderr, "Fano successful, current peak bin: %d, shift: %d\n", currentPeakBin, shift);
          int8_t message[12];
          char call_loc_pow[23] = {0};
          char call[13] = {0};
          char callsign[13] = {0};
          char loc[7] = {0};
          char pwr[3] = {0};
          //char hashtab[32768*13] = {0};
          for (int i = 0; i < 12; i++) {
            if (data[i] > 127) {
              message[i] = data[i] - 256;
            } else {
              message[i] = data[i];
            }
          }
          fanoObject.unpk(message, call_loc_pow, call, loc, pwr, callsign);
          fprintf(stderr, "unpacked data: %s %s %s %s %s\n", call_loc_pow, call, loc, pwr, callsign);
        } else {
          fprintf(stderr, "Did not decode peak bin: %d @ shift: %d, metric: %8.8x, cycles: %d, maxnp: %d\n",
                  currentPeakBin, shift, metric, cycles, maxnp);
        }
      }
    }
  }
  fprintf(stderr, "leaving doWork within WSPRPass1\n");
}

WSPRPass1::~WSPRPass1(void) {
  fprintf(stderr, "destructing WSPRPass1\n");
  if (fftOverTime) free(fftOverTime);
  if (mag) free(mag);
  if (magAcc) free(magAcc);
  if (binArray) free(binArray);
  if (samples) free(samples);
}


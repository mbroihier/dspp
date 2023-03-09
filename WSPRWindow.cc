/*
 *      WSPRWindow.cc - Object that collects a window of WSPR data
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
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "SpotCandidate.h"
#include "WSPRWindow.h"
#define SELFTEST 1

/* ---------------------------------------------------------------------- */
void WSPRWindow::init(int size, int number, char * prefix, float dialFreq, bool skipSync) {
  this->size = size;
  this->number = number;
  this->prefix = prefix;
  this->dialFreq = dialFreq;
  this->skipSync = skipSync;
  freq = 375.0;
  fftObject = new DsppFFT(size);
  deltaFreq = freq / size;
  fprintf(stderr, "allocating binArray memory\n");
  binArray = reinterpret_cast<int *>(malloc(number * sizeof(int)));
  fprintf(stderr, "allocating FFT memory - %d bytes\n", size * sizeof(float) * 2 * FFTS_PER_SHIFT * SHIFTS);
  fftOverTime = reinterpret_cast<float *> (malloc(size * sizeof(float) * 2 * FFTS_PER_SHIFT * SHIFTS));
  fprintf(stderr, "allocating window IQ memory - %d bytes\n", (int)freq  * sizeof(float) * 2 * PROCESSING_SIZE);
  windowOfIQData = reinterpret_cast<float *> (malloc((int)freq * sizeof(float) * 2 * PROCESSING_SIZE));
  fprintf(stderr, "allocating mag memory\n");
  mag = reinterpret_cast<float *>(malloc(size * sizeof(float)));
  magAcc = reinterpret_cast<float *>(malloc(size * sizeof(float)));
  sampleBufferSize = (int) freq * PROCESSING_SIZE * 2;
  tic = 0;
  memset(magAcc, 0, size * sizeof(float));
  for (int index = 0; index < size * 2 * FFTS_PER_SHIFT * SHIFTS; index++) {
    fftOverTime[index] = 0.0;
  }
}

WSPRWindow::WSPRWindow(int size, int number, char * prefix, float dialFreq, bool skipSync) {
  fprintf(stderr, "creating WSPRWindow object\n");
  init(size, number, prefix, dialFreq, skipSync);
  fprintf(stderr, "done creating WSPRWindow object\n");
}

void WSPRWindow::doWork() {
  char hashtab[32768*13] = {0};  //EXPERIMENT - move to Fano
  pid_t background = 0;
  int status = 0;

  int count = 0;
  float * samplePtr;
  float * magPtr;
  float * magAccPtr;
  float * fftOverTimePtr;
  fprintf(stderr, "Process WSPR Windows\n", number);
  bool done = false;
  float wallClock = 0.0;
  float deltaTime = 1.0 / freq * size;
  int baseTime = time(0);
  while (!done) {
    fprintf(stderr, "Starting a window at %d\n", time(0) - baseTime);
    // get a Window worth of samples
    // wait for an odd to even minute transition
    if (background) {
      pid_t id;
      float skipSamples[2];
      while (background) {
        id = waitpid(background, &status, WNOHANG);
        if (id < 0) {
          background = 0;  // background child process has terminated
        }
        if (0 == fread(skipSamples, sizeof(float), 2, stdin)) {  // skip all samples
          sleep(1.0);
        }
      }
    }
    if (! skipSync) {
      bool currentState = time(0) / 60 & 0x01; // is it an even or odd min?
      if (currentState == 0) {  // in a even minute, so wait for an odd
        while (((time(0) / 60) & 0x01) == 0) {
          fprintf(stderr, "e");
          float skipSamples[2];
          fread(skipSamples, sizeof(float), 2, stdin);  // skip samples until even minute
        }
      }
      while (((time(0) / 60) & 0x01) == 1) {
        fprintf(stderr, "o");
        float skipSamples[2];
        fread(skipSamples, sizeof(float), 2, stdin);  // skip samples until even minute
      }
    }
    fprintf(stderr, "\nCollecting samples at %d\n", time(0) - baseTime);

    count = fread(windowOfIQData, sizeof(float), sampleBufferSize, stdin);
    fprintf(stderr, "Done collecting samples at %d\n", time(0) - baseTime);
    if (count < sampleBufferSize) {
      done = true;
      continue;
    }
    background = fork();
    if (background == 0) {  // this is the child process, so continue this processing in "backgroun"
      fftOverTimePtr = fftOverTime;
      for (int shift = 0; shift < SHIFTS; shift++) {
        fftOverTimePtr = fftOverTime;
        fftOverTimePtr += shift * size * 2 * FFTS_PER_SHIFT;
        float * samplePtr = windowOfIQData;
        samplePtr += shift * 2;
        int samplesLeft = sampleBufferSize - shift * 2;
        while (samplesLeft >= 2 * size) {
          fftObject->processSampleSet(samplePtr, fftOverTimePtr);
          samplesLeft -= size * 2;
          fftOverTimePtr += size * 2;
          samplePtr += size * 2;
        }
      }
      fprintf(stderr, "Done with FFTs at %d\n", time(0) - baseTime);

      // Now it is time to find the frequencies that have the most power on them
      samplePtr = fftOverTime;
      // generate magnitude
      for (int fftIndex = 0; fftIndex < FFTS_PER_SHIFT; fftIndex++) {
        magPtr = mag;
        magAccPtr = magAcc;
        for (int j = 0; j < size; j++) {
          float r = *samplePtr++;
          float i = *samplePtr++;
          *magPtr = sqrt(r*r + i*i);
          *magAccPtr++ += *magPtr;
          magPtr++;
        }
      }
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
  
      memset(magAcc, 0, size * sizeof(float));  // clear magnitude accumulation for next cycle

      // Scan sequences of FFTs looking for WSPR signal
      // For each peak
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
        for (int shift = 0; shift < SHIFTS; shift++) {
          fprintf(stderr, "Processing sample shift of %d\n", shift);
          candidateInfo.clear();  // clear information for this cycle
          for (int t = 0; t < FFTS_PER_SHIFT; t++) {
            SpotCandidate::SampleRecord sr;
            sr.centroid = 0.0;
            sr.magnitude = 0.0;
            sr.magSlice.clear();
            sr.timeStamp = t;
            sr.timeSeconds = t * deltaTime;
            float acc = 0.0;
            float accBinLoc = 0.0;
            for (int bin = 0; bin < SpotCandidate::WINDOW; bin++) {
              float r = fftOverTime[shift * FFTS_PER_SHIFT * size * 2 + t * size * 2 + freqBinsToProcess[bin] * 2];
              float i = fftOverTime[shift * FFTS_PER_SHIFT * size * 2 + t * size * 2 + freqBinsToProcess[bin] * 2 + 1];
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
              sr.centroid = 0.0;
              sr.magnitude = acc;
              candidateInfo.push_back(sr);
              fprintf(stderr, "Error - should always be able to generate a centroid\n");
              fprintf(stderr, "FFT sample %d, in shift %d\n", t, shift);
            }
          }
          SpotCandidate candidate(currentPeakBin, candidateInfo, deltaFreq);
          //candidate.printReport();
          unsigned char symbols[162];
          unsigned int metric;
          unsigned int cycles;
          unsigned int maxnp;
          unsigned char data[12];
          unsigned int nbits = 81;
          int delta = 60;
          unsigned int maxcycles = 10000;
          int numberOfSymbolSets = candidateInfo.size() - NOMINAL_NUMBER_OF_SYMBOLS;
          for (int symbolSet = 0; symbolSet < numberOfSymbolSets; symbolSet++) {
            std::vector<SpotCandidate::SampleRecord> subset;
            for (int index = 0; index < NOMINAL_NUMBER_OF_SYMBOLS; index++) {
              subset.push_back(candidateInfo[index + symbolSet]);
            }
            std::vector<int>  tokens;
            candidate.tokenize(subset, tokens);
            for (int index = 0; index < NOMINAL_NUMBER_OF_SYMBOLS; index++) {
              symbols[index] = tokens[index];
            }
            fprintf(stderr, "Deinterleave symbols\n");
            fanoObject.deinterleave(symbols);
            fprintf(stderr, "Performing Fano\n");
            if (fanoObject.fano(&metric, &cycles, &maxnp, data, symbols, nbits, delta, maxcycles)) {
              fprintf(stderr, "Did not decode peak bin: %d @ symbol set: %d, metric: %8.8x, cycles: %d, maxnp: %d\n",
                      currentPeakBin, symbolSet, metric, cycles, maxnp);
            } else {
              bool pass = false;
              for (auto c : data) {
                if (c != 0) pass = true;
              }
              if (pass) {
                fprintf(stderr, "Fano successful, current peak bin: %d, symbol set: %d\n", currentPeakBin, symbolSet);
                int8_t message[12];
                char call_loc_pow[23] = {0};
                char call[13] = {0};
                char callsign[13] = {0};
                char loc[7] = {0};
                char pwr[3] = {0};
                for (int i = 0; i < 12; i++) {
                  if (data[i] > 127) {
                    message[i] = data[i] - 256;
                  } else {
                    message[i] = data[i];
                  }
                }
                fanoObject.unpk(message, hashtab, call_loc_pow, call, loc, pwr, callsign);
                fprintf(stderr, "unpacked data: %s %s %s %s %s\n", call_loc_pow, call, loc, pwr, callsign);
                fprintf(stdout, "spot: %s at frequency %15.0f\n", call_loc_pow, dialFreq + candidate.getFrequency());
              } else {
                fprintf(stderr, "Did not decode peak bin: %d @ symbol set: %d, metric: %8.8x, cycles: %d, maxnp: %d\n",
                        currentPeakBin, symbolSet, metric, cycles, maxnp);
              }
            }
          }
        }
      }
      exit(0) ; // terminate child process
    }
  }
  fprintf(stderr, "leaving doWork within WSPRWindow\n");
}

WSPRWindow::~WSPRWindow(void) {
  fprintf(stderr, "destructing WSPRWindow\n");
  if (fftOverTime) free(fftOverTime);
  if (mag) free(mag);
  if (magAcc) free(magAcc);
  if (binArray) free(binArray);
}

#ifdef SELFTEST
int main() {
  char pre[10] = {0};
  float dialFreq = 14095600.0;
  snprintf(pre, sizeof(pre), "%s", "prefix");
  WSPRWindow testObj(256, 7, pre, dialFreq, false);
  testObj.doWork();
}
#endif

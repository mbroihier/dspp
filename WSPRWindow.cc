/*
 *      WSPRWindow.cc - Object that collects a window of WSPR data
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <algorithm>
#include <map>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <string.h>
#include "SpotCandidate.h"
#include "WSPRWindow.h"
#include "WSPRUtilities.h"
//#define SELFTEST 1

/* ---------------------------------------------------------------------- */
void WSPRWindow::init(int size, int number, char * prefix, float dialFreq, char * reporterID,
                      char * reporterLocation) {
  this->size = size;
  this->number = number;
  this->prefix = prefix;
  this->dialFreq = dialFreq;
  freq = BASE_BAND;
  fftObject = new DsppFFT(size);
  deltaFreq = freq / size;
  fprintf(stderr, "allocating binArray memory\n");
  binArray = reinterpret_cast<int *>(malloc(number * sizeof(int)));
  fprintf(stderr, "allocating FFT memory - %d bytes\n", size * sizeof(float) * 2 * FFTS_PER_SHIFT * SHIFTS);
  fftOverTime = reinterpret_cast<float *> (malloc(size * sizeof(float) * 2 * FFTS_PER_SHIFT * SHIFTS));
  windowOfIQData = NULL;
  fprintf(stderr, "allocating mag memory\n");
  mag = reinterpret_cast<float *>(malloc(size * sizeof(float)));
  magAcc = reinterpret_cast<float *>(malloc(size * sizeof(float)));
  sampleBufferSize = (int) freq * PROCESSING_SIZE * 2;
  tic = 0;
  memset(magAcc, 0, size * sizeof(float));
  for (int index = 0; index < size * 2 * FFTS_PER_SHIFT * SHIFTS; index++) {
    fftOverTime[index] = 0.0;
  }
  snprintf(this->reporterID, sizeof(this->reporterID) - 1, "%s", reporterID);
  snprintf(this->reporterLocation, sizeof(this->reporterLocation) - 1, "%s", reporterLocation);
  if (strlen(this->reporterID) != strlen(reporterID) || strlen(this->reporterLocation) != strlen(reporterLocation)) {
    fprintf(stderr, "Error in reporter parameters\n");
    exit(-1);
  }
}

WSPRWindow::WSPRWindow(int size, int number, char * prefix, float dialFreq, char * reporterID,
                       char * reporterLocation) {
  fprintf(stderr, "creating WSPRWindow object\n");
  init(size, number, prefix, dialFreq, reporterID, reporterLocation);
  fprintf(stderr, "done creating WSPRWindow object\n");
}

void WSPRWindow::remap(std::vector<int> tokens, std::vector<int> &symbols, int mapSelector) {
  // map tokens to the possible symbol sets
  const int tokenToSymbol[] = { 0, 1, 2, 3,
                                0, 1, 3, 1,
                                0, 2, 1, 3,
                                0, 2, 3, 1,
                                0, 3, 1, 2,
                                0, 3, 2, 1,
                                1, 0, 2, 3,
                                1, 0, 3, 2,
                                1, 2, 0, 3,
                                1, 2, 3, 0,
                                1, 3, 0, 2,
                                1, 3, 2, 1,
                                2, 0, 1, 3,
                                2, 0, 3, 1,
                                2, 1, 0, 3,
                                2, 1, 3, 0,
                                2, 3, 0, 1,
                                2, 3, 1, 0,
                                3, 0, 1, 2,
                                3, 0, 2, 1,
                                3, 1, 0, 2,
                                3, 1, 2, 0,
                                3, 2, 0, 1,
                                3, 2, 1, 0 };
  int offset = mapSelector * 4;
  symbols.clear();
  for (auto element : tokens) {
    symbols.push_back(tokenToSymbol[element + offset] << 6);
  }
}
void WSPRWindow::doWork() {
  pid_t background = 0;

  time_t now;
  time_t spotTime;
  int count = 0;
  float * samplePtr;
  float * magPtr;
  float * magAccPtr;
  float * fftOverTimePtr;
  fprintf(stderr, "Process WSPR Windows\n");
  bool done = false;
  float deltaTime = 1.0 / freq * size;
  int baseTime = time(0);
  int sampleLabel = 0;
  char sampleFile[50];
  bool terminate = false;

  auto reaper = [&background, &terminate]() {
                  pid_t id;
                  int status;
                  while (!terminate) {
                    if (background) {
                      id = waitpid(background, &status, WNOHANG);
                      if (id < 0 || id == background) {
                        background = 0;  // background child process has terminated
                      }
                    }
                    sleep(0.0);
                  }
                };
  std::thread monitor(reaper);
  bool firstTime = true;
  while (!done) {
    fprintf(stderr, "Starting a window at %ld\n", time(0) - baseTime);
    // get a Window worth of samples
    if (!windows.empty()) {
      free(windows.front().data);
      windows.pop();  // remove the first entry if it exists
    }
    while (background || firstTime || windows.empty()) {
      fprintf(stderr, "first time or there is a background process or the queue is empty\n");
      if (!firstTime) {
        float remainsOf2Min[(PERIOD - PROCESSING_SIZE) * BASE_BAND * 2];
        // Before entering loop, disard the remaining samples associated with this 2 minute block
        fprintf(stderr, "Discarding %d unused samples of this 2 minute window\n", (PERIOD - PROCESSING_SIZE) * BASE_BAND * 2);
        fread(remainsOf2Min, sizeof(float), (PERIOD - PROCESSING_SIZE) * BASE_BAND * 2, stdin);
      }
      fprintf(stderr, "allocating window IQ memory - %d bytes\n", (int)freq  * sizeof(float) * 2 * PROCESSING_SIZE);
      now = time(0);
      windows.push({now, reinterpret_cast<float *> (malloc((int)freq * sizeof(float) * 2 * PROCESSING_SIZE))});
      fprintf(stderr, "\nCollecting %d samples at %ld - %s", sampleBufferSize, now - baseTime, ctime(&now));
      if ((count = fread(windows.back().data, sizeof(float), PROCESSING_SIZE * BASE_BAND * 2, stdin)) == 0) {
        fprintf(stderr, "Input read was empty, sleeping for a while at %s", ctime(&now));
        sleep(60.0);
      }
      firstTime = false;
    }

    windowOfIQData = windows.front().data;
    spotTime = windows.front().windowStartTime;

    sampleLabel = spotTime - baseTime;
    if (count < sampleBufferSize) {
      done = true;
      continue;
    }
    if (strlen(prefix) > 0) {
      snprintf(sampleFile, 50, "%s%d.bin", prefix, sampleLabel);
      WSPRUtilities::writeFile(sampleFile, windowOfIQData, sampleBufferSize);
    }
    fflush(stdout);  // flush standard out to make file output sane
    background = fork();
    if (background == 0) {  // this is the child process, so continue this processing in "background"
      float stdOfNoise = 0.0;
      fprintf(stdout, "Starting child\n");
      fanoObject.childAttach();  // attach shared memory
      fftOverTimePtr = fftOverTime;
      struct info { char * date; char * time; char * callSign; char * power; char * loc; int occurrence; double freq; int shift; float snr; float drift; };
      std::map<int, info> candidates;
      int numberOfCandidates = 0;
      for (int shift = 0; shift < SHIFTS; shift++) {
        fftOverTimePtr = fftOverTime;
        fftOverTimePtr += shift * size * 2 * FFTS_PER_SHIFT;
        float * samplePtr = windowOfIQData;
        samplePtr += shift * 2;
        int samplesLeft = sampleBufferSize - shift * 2;
        float * ptrLimit = &fftOverTime[SHIFTS * size * 2 * FFTS_PER_SHIFT];
        while (samplesLeft >= 2 * size && fftOverTimePtr < ptrLimit) {
          //  while enough samples to FFT and while enough FFT buffers
          fftObject->processSampleSet(samplePtr, fftOverTimePtr);
          samplesLeft -= size * 2;
          fftOverTimePtr += size * 2;
          samplePtr += size * 2;
        }
      }
      fprintf(stderr, "Done with FFTs at %ld\n", time(0) - baseTime);

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
      // calculate bacground average
      int outOfRange = size*0.46/2;  // bins outside of +/- 100 Hz
      float workingValue = 0.0;
      float sumOfNoise = 0.0;
      float sumOfNoiseSq = 0.0;
      int numberOfNoiseSamples = 0;
      for (int i = size/2 - outOfRange; i < size/2 + outOfRange; i++) {
        workingValue = magAcc[i]/FFTS_PER_SHIFT;
        sumOfNoise += workingValue;
        sumOfNoiseSq += workingValue * workingValue;
        numberOfNoiseSamples++;
      }
      stdOfNoise = sqrt((numberOfNoiseSamples * sumOfNoiseSq - sumOfNoise * sumOfNoise) /
                              (numberOfNoiseSamples * (numberOfNoiseSamples - 1)));
      fprintf(stderr, "background sumOfNoise: %f, sumOfNoiseSq: %f, number of samples: %d\n",
              sumOfNoise, sumOfNoiseSq, numberOfNoiseSamples);
      fprintf(stderr, "background std: %10.5f, dB: %5.2f, power dB: %5.2f\n", stdOfNoise, 10 * log10(stdOfNoise),
              20 * log10(stdOfNoise));
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
        for (int shift = 0; shift < SHIFTS; shift += 10) {
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
              break;
            }
          }
          SpotCandidate candidate(currentPeakBin, candidateInfo, deltaFreq);
          if (!candidate.isValid()) continue;
          //candidate.printReport();
          unsigned char symbols[162];
          unsigned int metric;
          unsigned int cycles;
          unsigned int maxnp;
          unsigned char data[12];
          unsigned int nbits = 81;
          int delta = 60;
          unsigned int maxcycles = 10000;
          int numberOfSymbolSets = candidateInfo.size() - NOMINAL_NUMBER_OF_SYMBOLS + 1;
          for (int symbolSet = 0; symbolSet < numberOfSymbolSets; symbolSet++) {
            std::vector<SpotCandidate::SampleRecord> subset;
            for (int index = 0; index < NOMINAL_NUMBER_OF_SYMBOLS; index++) {
              subset.push_back(candidateInfo[index + symbolSet]);
            }
            std::vector<int>  tokens;
            std::vector<int> symbolVector;
            float snr = 0.0;
            float slope = 0.0;
            candidate.tokenize(subset, tokens, snr, slope, stdOfNoise);
            for (int remapIndex = 0; remapIndex < 4; remapIndex += 3) {  // can be up to 24, now only 0 and 3
              remap(tokens, symbolVector, remapIndex);
              for (int index = 0; index < NOMINAL_NUMBER_OF_SYMBOLS; index++) {
                symbols[index] = symbolVector[index];
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
                  fprintf(stderr, "Fano successful, current peak bin: %d, symbol set: %d, remapIndex: %d\n",
                          currentPeakBin, symbolSet, remapIndex);
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
                  if (strlen(prefix) > 0) {
                    char sampleFile[100];
                    snprintf(sampleFile, sizeof(sampleFile), "%s_Signal_%d.bin", prefix, sampleLabel);
                    WSPRUtilities::writeFile(sampleFile, windowOfIQData, sampleBufferSize);
                  }
                  int unpkStatus = fanoObject.unpk(message, call_loc_pow, call, loc, pwr, callsign);
                  fprintf(stderr, "unpacked data: %s %s %s %s %s, status: %d\n",
                          call_loc_pow, call, loc, pwr, callsign, unpkStatus);
                  //fprintf(stdout, "spot: %s at frequency %15.0f, currentPeakIndex: %d, bin: %d shift: %d, "
                  //        "remapIndex: %d\n",
                  //        call_loc_pow, dialFreq + 1500.0 +
                  //        candidate.getFrequency(), currentPeakIndex, currentPeakBin, shift, remapIndex);
                  fprintf(stderr, "spot: %s at frequency %1.0f, currentPeakIndex: %d, bin: %d shift: %d, "
                          "remapIndex: %d, symbol set: %d, delta time: %2.1f\n",
                          call_loc_pow, dialFreq + 1500.0 + ((remapIndex == 0) ? 1: -1) * candidate.getFrequency() +
                          3.0 * deltaFreq,
                          currentPeakIndex, currentPeakBin, shift, remapIndex, symbolSet,
                          (symbolSet * 256 + shift) * SECONDS_PER_SHIFT - 1.0);
                  bool newCand = true;
                  for (auto iter = candidates.begin(); iter != candidates.end(); iter++) {
                    if ((strcmp((*iter).second.callSign, callsign) == 0) &&
                        (fabs((*iter).second.freq - (dialFreq + 1500.0 +
                                                     ((remapIndex == 0) ? 1: -1) *
                                                     candidate.getFrequency() + 3.0 * deltaFreq) < 3.0))) {
                      newCand = false;
                      (*iter).second.occurrence++;
                      int normalizedShift = symbolSet * 256 + shift;
                      (*iter).second.shift += normalizedShift;
                      if (snr > (*iter).second.snr) {
                        (*iter).second.snr = snr;
                      }
                    }
                  }
                  if (newCand) {
                    // note, this memory will be released when the child terminates
                    char * d = reinterpret_cast<char *>(malloc(7)); // date
                    char * t = reinterpret_cast<char *>(malloc(5)); // time
                    struct tm * gtm;
                    gtm = gmtime(&spotTime);
                    snprintf(d, 7, "%02d%02d%02d", gtm->tm_year - 100, gtm->tm_mon + 1, gtm->tm_mday);
                    snprintf(t, 5, "%02d%02d", gtm->tm_hour, gtm->tm_min);
                    char * cs = strdup(callsign);
                    char * p = strdup(pwr);
                    char * l = strdup(loc);
                    int normalizedShift = symbolSet * 256 + shift;
                    candidates[numberOfCandidates] = { d, t, cs, p, l, 1,
                                                       dialFreq + 1500.0 + ((remapIndex == 0) ? 1: -1) *
                                                       candidate.getFrequency() + 3.0 * deltaFreq,
                                                       normalizedShift,
                                                       snr, slope * SLOPE_TO_DRIFT_UNITS };
                    numberOfCandidates++;
                  }
                  break;
                } else {
                  fprintf(stderr,
                          "Did not decode peak bin: %d @ symbol set: %d, metric: %8.8x, cycles: %d, maxnp: %d\n",
                          currentPeakBin, symbolSet, metric, cycles, maxnp);
                }
              }
            }
          }
        }
      }
      fanoObject.childDetach();
      for (auto iter = candidates.begin(); iter != candidates.end(); iter++) {
        if ((*iter).second.occurrence > 1) {
          int iP = 0;
          sscanf((*iter).second.power, "%d", &iP);
          float p = exp10f((float) iP / 10.0) / 1000.0;
          char charSNR[4];
          snprintf(charSNR, sizeof(charSNR), "%.0f", (*iter).second.snr);
          fprintf(stdout, "%s %s: Candidate %d (%s) was seen %d times at %1.0f Hz with best SNR of %4.3f dB,\n"
                  " with transmitter power of %4.3f W, location of %s, drift of %3.2f, and delta time of %2.1f\n",
                  (*iter).second.date, (*iter).second.time,
                  (*iter).first, (*iter).second.callSign, (*iter).second.occurrence, (*iter).second.freq,
                  (*iter).second.snr, p, (*iter).second.loc,
                  (*iter).second.drift,
                  (*iter).second.shift * SECONDS_PER_SHIFT / (*iter).second.occurrence - 2.0);
          WSPRUtilities::reportSpot(reporterID, reporterLocation, (*iter).second.freq, (*iter).second.shift *
                                    SECONDS_PER_SHIFT / (*iter).second.occurrence - 2.0,
                                    (*iter).second.drift, (*iter).second.callSign,
                                    (*iter).second.loc, (*iter).second.power, charSNR, (*iter).second.date,
                                    (*iter).second.time);
        }
      }
      fprintf(stdout, "Child process complete\n");
      exit(0) ; // terminate child process
    }
  }
  terminate = true;
  monitor.join();  // wait for reaper thread to finish
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
  char id[13] = {0};
  char loc[7] = {0};
  float dialFreq = 14095600.0;
  snprintf(pre, sizeof(pre), "%s", "prefix");
  snprintf(id, sizeof(id), "%s", "KG5YJE/P");
  snprintf(loc, sizeof(loc), "%s", "EM13");
  WSPRWindow testObj(256, 9, pre, dialFreq, id, loc );
  testObj.doWork();
}
#endif

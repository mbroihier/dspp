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
  deltaFreq = freq / size;
  fprintf(stderr, "allocating binArray memory\n");
  binArray = reinterpret_cast<int *>(malloc(number * sizeof(int)));
  SNRData = reinterpret_cast<SNRInfo *>(malloc(number * sizeof(SNRInfo)));
  fprintf(stderr, "allocating FFT memory - %ld bytes\n", size * sizeof(float) * 2 * FFTS_PER_SHIFT * SHIFTS);
  fftOverTime = reinterpret_cast<float *> (malloc(size * sizeof(float) * 2 * FFTS_PER_SHIFT * SHIFTS));
  windowOfIQData = NULL;
  fprintf(stderr, "allocating mag memory\n");
  mag = reinterpret_cast<float *>(malloc(size * sizeof(float)));
  sortedMag = reinterpret_cast<float *>(malloc(size * sizeof(float)));
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

int WSPRWindow::remap(std::vector<int> tokens, std::vector<int> &symbols, int mapSelector) {
  // map tokens to the possible symbol sets
  const int tokenToSymbol[] = { 0, 1, 2, 3,
                                0, 1, 3, 2,
                                0, 2, 1, 3,
                                0, 2, 3, 1,
                                0, 3, 1, 2,
                                0, 3, 2, 1,
                                1, 0, 2, 3,
                                1, 0, 3, 2,
                                1, 2, 0, 3,
                                1, 2, 3, 0,
                                1, 3, 0, 2,
                                1, 3, 2, 0,
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

  const int interleavedSync [] = { 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0,
                                   0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1,
                                   0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1,
                                   1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1,
                                   0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0,
                                   0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1,
                                   0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1,
                                   0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0,
                                   0, 0 };

  int offset = mapSelector * 4;
  symbols.clear();
  int metric = 0;
  int index = 0;
  for (auto element : tokens) {
    if ((tokenToSymbol[element + offset] & 0x01) == interleavedSync[index++]) metric++;
    symbols.push_back(tokenToSymbol[element + offset] << 6);
  }
  return metric;
}

void WSPRWindow::doWork() {
  pid_t background = 0;

  time_t now;
  time_t spotTime;
  int count = 0;
  fprintf(stderr, "Process WSPR Windows\n");
  bool done = false;
  int baseTime = time(0);
  int sampleLabel = 0;
  char sampleFile[50];
  bool terminate = false;

  auto search = [&background, &terminate, &spotTime, &baseTime, &sampleLabel,
                 this]() {
                  float deltaTime = 1.0 / freq * size;
                  float * samplePtr;
                  fftObject = new DsppFFT(size);
                  while (!terminate) {
                    if (background) {
                      fprintf(stdout, "Starting search thread\n");
                      float * fftOverTimePtr = fftOverTime;
                      struct info { char * date; char * time; char * callSign; char * power; char * loc;
                        int occurrence; double freq; int shift; float snr; float drift; };
                      std::map<int, info> candidates;
                      int numberOfCandidates = 0;
                      for (int shift = 0; shift < SHIFTS; shift++) {
                        fftOverTimePtr = fftOverTime;
                        fftOverTimePtr += shift * size * 2 * FFTS_PER_SHIFT;
                        samplePtr = windowOfIQData;
                        samplePtr += shift * 2;
                        int samplesLeft = sampleBufferSize - shift * 2;
                        float * ptrLimit = &fftOverTime[SHIFTS * size * 2 * FFTS_PER_SHIFT];
                        while (samplesLeft >= 2 * size && fftOverTimePtr < ptrLimit) {
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
                        float * magPtr = mag;
                        float* magAccPtr = magAcc;
                        for (int j = 0; j < size; j++) {
                          float r = *samplePtr++;
                          float i = *samplePtr++;
                          *magPtr = sqrt(r*r + i*i);
                          *magAccPtr++ += *magPtr;
                          magPtr++;
                        }
                      }

                      calculateSNR(magAcc); // also sets binArray

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
                            freqBinsToProcess[i + offset] = ((currentPeakBin + i) >= 0) ?
                              currentPeakBin + i : size + (currentPeakBin + i);
                          } else {
                            freqBinsToProcess[i + offset] = (currentPeakBin + i) % size;
                          }
                        }
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
                              float r = fftOverTime[shift * FFTS_PER_SHIFT * size * 2 + t * size * 2 +
                                                    freqBinsToProcess[bin] * 2];
                              float i = fftOverTime[shift * FFTS_PER_SHIFT * size * 2 + t * size * 2 +
                                                    freqBinsToProcess[bin] * 2 + 1];
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
                              fprintf(stderr, "currentPeakIndex: %d, currentPeakBin: %d\n",
                                      currentPeakIndex, currentPeakBin);
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
                            //candidate.tokenize(subset, tokens, snr, slope, stdOfNoise);
                            candidate.tokenize(subset, tokens, slope);
                            snr = SNRData[currentPeakIndex].SNR;
                            for (int remapIndex = 0; remapIndex < 24; remapIndex += 1) {
                              int symbolMetric = remap(tokens, symbolVector, remapIndex);
                              fprintf(stderr, "symbol metric after remap(%d): %d, peak bin: %d\n",
                                      remapIndex, symbolMetric, currentPeakBin);
                              if (symbolMetric < 100) continue;  // if match is not good enough, go to next remapping
                              for (int index = 0; index < NOMINAL_NUMBER_OF_SYMBOLS; index++) {
                                symbols[index] = symbolVector[index];
                              }
                              fprintf(stderr, "Deinterleave symbols\n");
                              fanoObject.deinterleave(symbols);
                              fprintf(stderr, "Performing Fano\n");
                              if (fanoObject.fano(&metric, &cycles, &maxnp, data, symbols, nbits, delta, maxcycles)) {
                                fprintf(stderr, "Did not decode peak bin: %d @ symbol set: %d, "
                                        "metric: %8.8x, cycles: %d, maxnp: %d\n", currentPeakBin,
                                        symbolSet, metric, cycles, maxnp);
                              } else {
                                bool pass = false;
                                for (auto c : data) {
                                  if (c != 0) pass = true;
                                }
                                if (pass) {
                                  fprintf(stderr, "Fano successful, current peak bin: %d, symbol set: %d, "
                                          "remapIndex: %d\n", currentPeakBin, symbolSet, remapIndex);
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
                                  fprintf(stderr, "spot: %s at frequency %1.0f, currentPeakIndex: %d, bin: "
                                          "%d shift: %d, "
                                          "remapIndex: %d, symbol set: %d, delta time: %2.1f, symbolMetric: %d\n",
                                          call_loc_pow, dialFreq + 1500.0 +  candidate.getFrequency(),
                                          currentPeakIndex, currentPeakBin, shift, remapIndex, symbolSet,
                                          (symbolSet * 256 + shift) * SECONDS_PER_SHIFT - 2.0, symbolMetric);
                                  bool newCand = true;
                                  for (auto iter = candidates.begin(); iter != candidates.end(); iter++) {
                                    if ((strcmp((*iter).second.callSign, callsign) == 0) &&
                                        (fabs((*iter).second.freq - (dialFreq + 1500.0 +
                                                                     candidate.getFrequency())) < 3.0)) {
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
                                    // note, this memory will be released when this search cycle  terminates
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
                                                                       dialFreq + 1500.0 + candidate.getFrequency(),
                                                                       normalizedShift,
                                                                       snr, slope * SLOPE_TO_DRIFT_UNITS };
                                    numberOfCandidates++;
                                  }
                                  break;
                                } else {
                                  fprintf(stderr,
                                          "Did not decode peak bin: %d @ symbol set: %d, "
                                          "metric: %8.8x, cycles: %d, maxnp: %d\n",
                                          currentPeakBin, symbolSet, metric, cycles, maxnp);
                                }
                              }
                            }
                          }
                        }
                      }
                      for (auto iter = candidates.begin(); iter != candidates.end(); iter++) {
                        if ((*iter).second.occurrence > 1) {
                          int iP = 0;
                          sscanf((*iter).second.power, "%d", &iP);
                          float p = exp10f((float) iP / 10.0) / 1000.0;
                          char charSNR[4];
                          snprintf(charSNR, sizeof(charSNR), "%.0f", (*iter).second.snr);
                          fprintf(stdout, "%s %s: Candidate %d (%s) was seen %d times at %1.0f Hz "
                                  "with best SNR of %4.3f dB,\n"
                                  " with transmitter power of %4.3f W, location of %s, drift of %3.2f, "
                                  "and delta time of %2.1f\n",
                                  (*iter).second.date, (*iter).second.time,
                                  (*iter).first, (*iter).second.callSign, (*iter).second.occurrence,
                                  (*iter).second.freq,
                                  (*iter).second.snr, p, (*iter).second.loc,
                                  (*iter).second.drift,
                                  (*iter).second.shift * SECONDS_PER_SHIFT / (*iter).second.occurrence - 2.0);
                          if (strlen(reporterID)) {
                            WSPRUtilities::reportSpot(reporterID, reporterLocation, (*iter).second.freq,
                                                      (*iter).second.shift *
                                                      SECONDS_PER_SHIFT / (*iter).second.occurrence - 2.0,
                                                      (*iter).second.drift, (*iter).second.callSign,
                                                      (*iter).second.loc, (*iter).second.power, charSNR,
                                                      (*iter).second.date, (*iter).second.time);
                          }
                        }
                      }
                      for (auto iter = candidates.begin(); iter != candidates.end(); iter++) {
                        // free memory allocated
                        free((*iter).second.date);
                        free((*iter).second.time);
                        free((*iter).second.callSign);
                        free((*iter).second.power);
                        free((*iter).second.loc);
                      }
                      candidates.clear();
                      fprintf(stdout, "search process complete\n");
                      windowsMutex.lock();
                      if (!windows.empty()) {
                        free(windows.front().data);
                        windows.pop();  // remove the first entry (this should be the one being processed)
                      }
                      background = 0;
                      windowsMutex.unlock();
                    } else {
                      //fprintf(stderr, "waiting to start\n");
                      sleep(0.3);
                    }
                  }
                  delete fftObject;
                };
  std::thread process(search);
  bool firstTime = true;
  WindowOfIQDataT entry;
  int queueLen = 0;
  while (!done) {
    fprintf(stderr, "Starting a window at %ld\n", time(0) - baseTime);
    // get a Window worth of samples
    while (background || firstTime || windows.empty()) {
      fprintf(stderr, "first time or there is a background process or the queue is empty\n");
      if (!firstTime) {
        float remainsOf2Min[(PERIOD - PROCESSING_SIZE) * BASE_BAND * 2];
        // Before reading another sample set, disard the remaining samples associated with the previous 2 minute block
        fprintf(stderr, "Discarding %d unused samples of last 2 minute window\n",
                (PERIOD - PROCESSING_SIZE) * BASE_BAND * 2);
        fread(remainsOf2Min, sizeof(float), (PERIOD - PROCESSING_SIZE) * BASE_BAND * 2, stdin);
      }
      fprintf(stderr, "allocating window IQ memory - %ld bytes\n", (int)freq  * sizeof(float) * 2 * PROCESSING_SIZE);
      now = time(0);
      entry = {now, reinterpret_cast<float *> (malloc((int)freq * sizeof(float) * 2 * PROCESSING_SIZE))};
      fprintf(stderr, "\nCollecting %d samples at %ld - %s", sampleBufferSize, now - baseTime, ctime(&now));
      if ((count = fread(entry.data, sizeof(float), PROCESSING_SIZE * BASE_BAND * 2, stdin)) == 0) {
        fprintf(stderr, "Input read was empty, sleeping for a while at %s", ctime(&now));
        sleep(1.0); 
        if (background == 0) { // kick off a queued buffer
          windowsMutex.lock();
          queueLen = windows.size();
          windowsMutex.unlock();
          if (queueLen > 0) {
            fprintf(stderr,"No new data, but releasing a window that has been queued.  Size now %d\n", queueLen);
            count = sampleBufferSize;
          }
          break;
        }
      } else {
        windowsMutex.lock();
        windows.push(entry);
        fprintf(stderr, "Queue now has %ld entries\n", windows.size());
        windowsMutex.unlock();
      }
      firstTime = false;
    }

    if (count < sampleBufferSize) {
      done = true;
      continue;
    }

    windowsMutex.lock();
    windowOfIQData = windows.front().data;
    spotTime = windows.front().windowStartTime;
    windowsMutex.unlock();

    sampleLabel = spotTime - baseTime;
    if (strlen(prefix) > 0) {
      snprintf(sampleFile, 50, "%s%d.bin", prefix, sampleLabel);
      WSPRUtilities::writeFile(sampleFile, windowOfIQData, sampleBufferSize);
    }
    fflush(stdout);  // flush standard out to make file output sane
    assert(background == 0); // this should always be the case
    background = 1;  // signal search to start
  }
  terminate = true;
  process.join();  // wait for search thread to finish
  fprintf(stderr, "leaving doWork within WSPRWindow\n");
}

// compare for qsort
int WSPRWindow::SNRCompare(const void * a, const void * b) {
  if ((*(const SNRInfo *)a).magnitude < (*(const SNRInfo *)b).magnitude) {
    return -1;
  } else {
    return (*(const SNRInfo *)a).magnitude > (*(const SNRInfo *)b).magnitude;
  }
}
void WSPRWindow::calculateSNR(float * accumulatedMagnitude) {
  int regionOfInterestCount = 0;
  int regionSize = 75.0 * size / BASE_BAND;
  int bound0 = (size - regionSize) / 2;
  int bound1 = (size + regionSize) / 2;
  SNRInfo * working = reinterpret_cast<SNRInfo *>(malloc(sizeof(SNRInfo) * size));
  for (int magIndex = 0; magIndex < size; magIndex++) {
    if (magIndex < bound0 || magIndex > bound1) {
      working[regionOfInterestCount].magnitude = accumulatedMagnitude[magIndex];
      working[regionOfInterestCount].bin = magIndex;
      working[regionOfInterestCount++].SNR = -100.0;
    }
  }
  fprintf(stderr, "sorting %d magnitudes\n", regionOfInterestCount);
  qsort(working, regionOfInterestCount, sizeof(SNRInfo), SNRCompare);
  float noisePower = working[(int)(0.30 * regionOfInterestCount)].magnitude;
  float noisePowerdB = 20 * log10(noisePower);
  fprintf(stderr, "noisePower: %f, dB: %5.2f, power dB: %5.2f\n", noisePower, 10 * log10(noisePower),
          20 * log10(noisePower));
  for (int i = 0; i < regionOfInterestCount; i++) {
    fprintf(stderr, "sortedMag[%3d]: %10.0f\n", i, working[i].magnitude);
  }
  for (int i = 0; i < number; i++) {
    SNRData[i].magnitude = working[regionOfInterestCount - i - 1].magnitude;
    SNRData[i].bin = working[regionOfInterestCount - i - 1].bin;
    binArray[i] = SNRData[i].bin;
    SNRData[i].SNR = 20 * log10(working[regionOfInterestCount - i - 1].magnitude) - noisePowerdB - 26.3;
    fprintf(stderr, "SNRData[%2d]: %10.0f, bin: %d, SNR: %f dB\n", i, SNRData[i].magnitude, SNRData[i].bin,
            SNRData[i].SNR);
    fprintf(stderr, "SNRAlt[%2d]: %10.0f, bin: %d, SNR: %f dB\n", i, SNRData[i].magnitude, SNRData[i].bin,
            10 * log10(working[regionOfInterestCount - i - 1].magnitude) - 10 * log10(noisePower) - 26.3);
    
  }
  free(working);
}

float WSPRWindow::getSNR(int bin) {
  float ret = -100.0;
  SNRInfo rec;
  for (int i = 0; i < number; i++) {
    rec = SNRData[i];
    if (rec.bin == bin) {
      return rec.SNR;
    }
  }
  return ret;
}

WSPRWindow::~WSPRWindow(void) {
  fprintf(stderr, "destructing WSPRWindow\n");
  if (fftOverTime) free(fftOverTime);
  if (mag) free(mag);
  if (sortedMag) free(sortedMag);
  if (magAcc) free(magAcc);
  if (binArray) free(binArray);
  if (SNRData) free(SNRData);
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

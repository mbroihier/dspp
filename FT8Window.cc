/*
 *      FT8Window.cc - Object that collects a window of WSPR data
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
#include "FT4FT8Fields.h"
#include "FT8SpotCandidate.h"
#include "FT8Window.h"
#include "FT8Utilities.h"
#include "unpack.h"
//#define SELFTEST 1

/* ---------------------------------------------------------------------- */
void FT8Window::init(int size, int number, char * prefix, float dialFreq, char * reporterID,
                      char * reporterLocation) {
  this->size = size;
  this->number = number;
  this->prefix = prefix;
  this->dialFreq = dialFreq;
  freq = BASE_BAND;
  deltaFreq = freq / size;
  fprintf(stderr, "delta frequency: %1.2f\n", deltaFreq);
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

FT8Window::FT8Window(int size, int number, char * prefix, float dialFreq, char * reporterID,
                       char * reporterLocation) {
  fprintf(stderr, "creating FT8Window object\n");
  init(size, number, prefix, dialFreq, reporterID, reporterLocation);
  fprintf(stderr, "done creating FT8Window object\n");
}

int FT8Window::remap(std::vector<int> tokens, std::vector<int> &symbols, int mapSelector, double * ll174) {
  // map tokens to the possible symbol sets
  const int tokenToSymbol[] = { 0, 1, 3, 2, 6, 4, 5, 7 };
  const int costas[] = { 3, 1, 4, 0, 6, 5, 2,
                         8, 8, 8, 8, 8, 8, 8,
                         8, 8, 8, 8, 8, 8, 8,
                         8, 8, 8, 8, 8, 8, 8,
                         8, 8, 8, 8, 8, 8, 8, 8,
                         3, 1, 4, 0, 6, 5, 2,
                         8, 8, 8, 8, 8, 8, 8,
                         8, 8, 8, 8, 8, 8, 8,
                         8, 8, 8, 8, 8, 8, 8,
                         8, 8, 8, 8, 8, 8, 8, 8,
                         3, 1, 4, 0, 6, 5, 2 };  // note that 8 is not a possible symbol value

  symbols.clear();
  int index = 0;
  int metric = 0;
  for (auto symbol : tokens) {
    if (symbol == costas[index++]) {
      metric++;
    }
  }
  index = 0;
  int llindex = 0;
  for (auto element : tokens) {
    if (costas[index] == 8) {
      int sym = tokenToSymbol[element];
      symbols.push_back(sym);  // enter message element
      ll174[llindex++] = (sym & 4) == 0 ? 4.99 : -4.99;
      ll174[llindex++] = (sym & 2) == 0 ? 4.99 : -4.99;
      ll174[llindex++] = (sym & 1) == 0 ? 4.99 : -4.99;
    }
    index++;
  }
  fprintf(stderr, "size of tokens array %ld, ll174 index %d\n", tokens.size(), llindex);
  assert(llindex == 174);
  return metric;
}

void FT8Window::doWork() {
  //void ldpc_decode(double llcodeword[], int iters, int plain[], int *ok);
  //void ft8_crc(int msg1[], int msglen, int out[14]);
  pid_t background = 0;

  time_t now;
  time_t spotTime;
  int count = 0;
  fprintf(stderr, "Process FT8 Windows\n");
  bool done = false;
  int baseTime = time(0);
  int sampleLabel = 0;
  bool terminate = false;

  auto search = [&background, &terminate, &spotTime, &baseTime, &sampleLabel,
                 this]() {
                  FT8Utilities reporter;
                  float deltaTime = 1.0 / freq * size;
                  float * samplePtr;
                  std::map<uint32_t, char *> hash22;
                  std::map<uint32_t, char *> hash12;
                  std::map<uint32_t, char *> hash10;
                  fftObject = new DsppFFT(size);
                  while (!terminate) {
                    if (background) {
                      fprintf(stdout, "Starting search thread\n");
                      float * fftOverTimePtr = fftOverTime;
                      struct info { char * date; char * time; char * message; int occurrence; double freq;
                        int shift; float snr; };
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

                      // Scan sequences of FFTs looking for FT8 signal
                      // For each peak
                      std::vector<int> symbolVector;
                      for (int currentPeakIndex = 0; currentPeakIndex < number; currentPeakIndex++) {
                        int currentPeakBin = binArray[currentPeakIndex];
                        int freqBinsToProcess[FT8SpotCandidate::WINDOW];
                        int offset = FT8SpotCandidate::WINDOW / 2;
                        for (int i = -offset; i <= offset; i++) {
                          if (i < 0) {
                            freqBinsToProcess[i + offset] = ((currentPeakBin + i) >= 0) ?
                              currentPeakBin + i : size + (currentPeakBin + i);
                          } else {
                            freqBinsToProcess[i + offset] = (currentPeakBin + i) % size;
                          }
                        }
                        std::vector<FT8SpotCandidate::SampleRecord> candidateInfo;
                        for (int shift = 0; shift < SHIFTS; shift += 10) {
                          fprintf(stderr, "Bin %d, processing sample shift of %d\n", currentPeakBin, shift);
                          candidateInfo.clear();  // clear information for this cycle
                          for (int t = 0; t < FFTS_PER_SHIFT; t++) {
                            FT8SpotCandidate::SampleRecord sr;
                            sr.centroid = 0.0;
                            sr.magnitude = 0.0;
                            sr.magSlice.clear();
                            sr.timeStamp = t;
                            sr.timeSeconds = t * deltaTime;
                            float acc = 0.0;
                            float accBinLoc = 0.0;
                            for (int bin = 0; bin < FT8SpotCandidate::WINDOW; bin++) {
                              float r = fftOverTime[shift * FFTS_PER_SHIFT * size * 2 + t * size * 2 +
                                                    freqBinsToProcess[bin] * 2];
                              float i = fftOverTime[shift * FFTS_PER_SHIFT * size * 2 + t * size * 2 +
                                                    freqBinsToProcess[bin] * 2 + 1];
                              float m = sqrt(r * r + i * i);
                              sr.magSlice.push_back(m);
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
                          FT8SpotCandidate candidate(currentPeakBin, candidateInfo, deltaFreq, size);
                          if (!candidate.isValid()) continue;
                          double ll174[174];
                          int p174[174];
                          int status = 0;
                          int numberOfSymbolSets = candidateInfo.size() - NOMINAL_NUMBER_OF_SYMBOLS + 1;
                          fprintf(stderr, "number of symbol sets: %d (%ld - %d + 1)\n", numberOfSymbolSets,
                                  candidateInfo.size(), NOMINAL_NUMBER_OF_SYMBOLS);
                          for (int symbolSet = 0; symbolSet < numberOfSymbolSets; symbolSet++) {
                            std::vector<FT8SpotCandidate::SampleRecord> subset;
                            for (int index = 0; index < NOMINAL_NUMBER_OF_SYMBOLS; index++) {
                              subset.push_back(candidateInfo[index + symbolSet]);
                            }
                            std::vector<int>  tokens;
                            float snr = 0.0;
                            float slope = 0.0;
                            candidate.tokenize(size, subset, tokens, slope);
                            fprintf(stderr, "tokenization returned %ld tokens\n", tokens.size());
                            if (tokens.size() == 0) continue;
                            snr = SNRData[currentPeakIndex].SNR;
                            for (int remapIndex = 0; remapIndex < 24; remapIndex += 24) {
                              int symbolMetric = remap(tokens, symbolVector, remapIndex, ll174);
                              fprintf(stderr, "symbol metric after remap(%d): %d, peak bin: %d\n",
                                      remapIndex, symbolMetric, currentPeakBin);
                              for (auto entry : symbolVector) {
                                fprintf(stderr, "%2d", entry);
                              }
                              fprintf(stderr, " end of symbols\n");
                              if (symbolMetric < 6) continue;  // if match is not good enough, go to next remapping
                              std::vector<bool> bits;
                              for (auto value : symbolVector) {
                                bits.push_back(value & 0x4);
                                bits.push_back(value & 0x2);
                                bits.push_back(value & 0x1);
                              }
                              std::vector<bool> correctedBits;
                              status = FT4FT8Utilities::ldpcDecode(bits, 15, &correctedBits);
                              if (correctedBits.size() != 174) continue;
                              payload174 payload = payload174(correctedBits);
                              int * p174ptr = &p174[0];
                              int nonZero = 0;
                              for (auto b : correctedBits) {
                                *p174ptr++ = b ? 1:0;
                                nonZero += b ? 1:0;
                              }
                              fprintf(stderr, " ldpc decode status: %d\n", status);
                              if (status >= 83) { // it is good enough
                                candidate.printReport();
                                if (nonZero) {
                                  fprintf(stderr, "checking CRC\n");
                                  bool dontMatch = false;
                                  if (FT4FT8Utilities::crc(payload("generic77", 0, true)) ==
                                      payload("cs14", 0, true)) {
                                    dontMatch = false;
                                  } else {
                                    dontMatch = true;
                                  }
                                  if (!dontMatch) {
                                    char msg[50];
                                    fprintf(stderr, "CRCs match!, bin: %d, shift: %d, symbol set %d\n",
                                            currentPeakBin, shift, symbolSet);
                                    //std::string msg = unpack(p174);
                                    std::vector<bool> i0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type1, payload,
                                                                                 "i3", 0);
                                    i3 mI3 = i3(i0);
                                    if (strcmp(mI3.decode(), "1") == 0) {
                                      fprintf(stdout, "processing message type 1\n");
                                      std::vector<bool> b0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type1, payload,
                                                                                   "c28", 0);
                                      c28 receivedCS = c28(b0);
                                      std::vector<bool> s0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type1, payload,
                                                                                   "r1", 0);
                                      r1 receivedCSSuf = r1(s0);
                                      std::vector<bool> b1 = FT4FT8Fields::overlay(MESSAGE_TYPES::type1, payload,
                                                                                   "c28", 1);
                                      c28 senderCS = c28(b1);
                                      std::vector<bool> s1 = FT4FT8Fields::overlay(MESSAGE_TYPES::type1, payload,
                                                                                   "r1", 1);
                                      r1 senderCSSuf = r1(s1);
                                      std::vector<bool> R0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type1, payload,
                                                                                   "R1", 0);
                                      R1 R = R1(R0);
                                      std::vector<bool> l0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type1, payload,
                                                                                   "g15", 0);
                                      g15 location = g15(l0);
                                      snprintf(msg, sizeof(msg), "%s%s %s%s %s%s",
                                               receivedCS.decode(&hash22, &hash12, &hash10), receivedCSSuf.decode(),
                                               senderCS.decode(&hash22, &hash12, &hash10), senderCSSuf.decode(),
                                               R.decode(), location.decode());
                                    } else if (strcmp(mI3.decode(), "2") == 0) {
                                      fprintf(stdout, "processing message type 2\n");
                                      std::vector<bool> b0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type2, payload,
                                                                                   "c28", 0);
                                      c28 receivedCS = c28(b0);
                                      std::vector<bool> s0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type2, payload,
                                                                                   "p1", 0);
                                      p1 receivedCSSuf = p1(s0);
                                      std::vector<bool> b1 = FT4FT8Fields::overlay(MESSAGE_TYPES::type2, payload,
                                                                                   "c28", 1);
                                      c28 senderCS = c28(b1);
                                      std::vector<bool> s1 = FT4FT8Fields::overlay(MESSAGE_TYPES::type2, payload,
                                                                                   "p1", 1);
                                      p1 senderCSSuf = p1(s1);
                                      std::vector<bool> R0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type2, payload,
                                                                                   "R1", 0);
                                      R1 R = R1(R0);
                                      std::vector<bool> l0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type2, payload,
                                                                                   "g15", 0);
                                      g15 location = g15(l0);
                                      snprintf(msg, sizeof(msg), "%s%s %s%s %s%s",
                                               receivedCS.decode(&hash22, &hash12, &hash10), receivedCSSuf.decode(),
                                               senderCS.decode(&hash22, &hash12, &hash10), senderCSSuf.decode(),
                                               R.decode(), location.decode());
                                    } else if (strcmp(mI3.decode(), "4") == 0) {
                                      fprintf(stdout, "processing message type 4\n");
                                      std::vector<bool> b0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type4, payload,
                                                                                   "h12", 0);
                                      h12 hashedCS = h12(b0);
                                      std::vector<bool> b1 = FT4FT8Fields::overlay(MESSAGE_TYPES::type4, payload,
                                                                                   "c58", 0);
                                      c58 extendedCS = c58(b1);
                                      std::vector<bool> b2 = FT4FT8Fields::overlay(MESSAGE_TYPES::type4, payload,
                                                                                   "h1", 0);
                                      h1 hashIsSecond = h1(b2);
                                      std::vector<bool> b3 = FT4FT8Fields::overlay(MESSAGE_TYPES::type4, payload,
                                                                                   "r2", 0);
                                      r2 extra = r2(b3);
                                      std::vector<bool> b4 = FT4FT8Fields::overlay(MESSAGE_TYPES::type4, payload,
                                                                                   "c1", 0);
                                      c1 firstIsCQ = c1(b4);
                                      if (firstIsCQ.decode()) { // if first is CQ ignore hash field and extra
                                        snprintf(msg, sizeof(msg), "CQ %s" , extendedCS.decode());
                                      } else {
                                        if (hashIsSecond.decode()) {  // flip the order of the call signs
                                          snprintf(msg, sizeof(msg), "%s %s %s", extendedCS.decode(),
                                                   hashedCS.decode(&hash12), extra.decode());
                                        } else {
                                          snprintf(msg, sizeof(msg), "%s %s %s",
                                                   hashedCS.decode(&hash12), extendedCS.decode(), extra.decode());
                                        }
                                      }
                                    } else if (strcmp(mI3.decode(), "0") == 0) {
                                      fprintf(stdout, "processing message type 0\n");
                                      std::vector<bool> b0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type0, payload,
                                                                                   "n3", 0);
                                      n3 type0Type = n3(b0);
                                      fprintf(stdout, "type 0 subtype: %s\n", type0Type.decode());
                                      msg[0] = 0;
                                    } else {
                                      fprintf(stdout, "Msg decode of message type %s is not supported yet.\n",
                                              mI3.decode());
                                      msg[0] = 0;
                                    }
                                    bool newCand = true;
                                    for (auto iter = candidates.begin(); iter != candidates.end(); iter++) {
                                      if ((strcmp((*iter).second.message, msg) == 0) &&
                                          (fabs((*iter).second.freq - (dialFreq + 1500.0 +
                                                                       candidate.getFrequency())) < 25.0)) {
                                        newCand = false;
                                        (*iter).second.occurrence++;
                                        int normalizedShift = symbolSet * 512 + shift;
                                        (*iter).second.shift += normalizedShift;
                                        if (snr > (*iter).second.snr) {
                                          (*iter).second.snr = snr;
                                        }
                                      }
                                    }
                                    if (newCand  && (strlen(msg) > 6)) {
                                      char * d = reinterpret_cast<char *>(malloc(7)); // date
                                      char * t = reinterpret_cast<char *>(malloc(7)); // time
                                      struct tm * gtm;
                                      gtm = gmtime(&spotTime);
                                      snprintf(d, 7, "%02d%02d%02d", gtm->tm_year - 100, gtm->tm_mon + 1,
                                               gtm->tm_mday);
                                      snprintf(t, 7, "%02d%02d%02d", gtm->tm_hour, gtm->tm_min, gtm->tm_sec / 15 * 15);
                                      char * message = strdup(msg);
                                      int normalizedShift = symbolSet * 256 + shift;
                                      candidates[numberOfCandidates] = {d, t, message, 1,
                                                                        dialFreq + 1500.0 + candidate.getFrequency(),
                                                                        normalizedShift,
                                                                        snr };
                                      numberOfCandidates++;
                                    }
                                  }
                                } else {
                                  fprintf(stderr, "p174 is all zeros\n");
                                }
                              } else {
                                fprintf(stderr, "ldpc status is not good enough\n");
                              }
                            }
                          }
                        }
                      }
                      
                      if (strlen(prefix) > 0) {
                        if (candidates.size()) {
                          char sampleFile[100];
                          snprintf(sampleFile, sizeof(sampleFile), "%s_Signal_%d.bin", prefix, sampleLabel);
                          FT8Utilities::writeFile(sampleFile, windowOfIQData, sampleBufferSize);
                        }
                      }

                      for (auto iter = candidates.begin(); iter != candidates.end(); iter++) {
                        if ((*iter).second.occurrence > 1) {
                          fprintf(stdout, "%s %s: Msg %d: %s, was seen %d times at %1.0f Hz "
                                  "with best SNR of %4.3f dB, "
                                  "and delta time of %2.1f\n",
                                  (*iter).second.date, (*iter).second.time,
                                  (*iter).first, (*iter).second.message, (*iter).second.occurrence,
                                  (*iter).second.freq, (*iter).second.snr,
                                  (*iter).second.shift * SECONDS_PER_SHIFT / (*iter).second.occurrence - 0.5);

                          if (strlen(reporterID)) {
                            reporter.reportSpot(reporterID, reporterLocation, (*iter).second.freq,
                                                spotTime/15*15 + (int) ((*iter).second.shift * SECONDS_PER_SHIFT /
                                                                        (*iter).second.occurrence - 0.5),
                                                (*iter).second.snr, (*iter).second.message);
                          }
                        }
                      }
                      for (auto iter = candidates.begin(); iter != candidates.end(); iter++) {
                        // free memory allocated
                        free((*iter).second.date);
                        free((*iter).second.time);
                        free((*iter).second.message);
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
                      if (windows.size()) {  // there is a window of data to work with
                        windowsMutex.lock();
                        windowOfIQData = windows.front().data;
                        spotTime = windows.front().windowStartTime;
                        background = 1;
                        windowsMutex.unlock();

                        sampleLabel = spotTime - baseTime;
                        if (strlen(prefix) > 0) {
                          char sampleFile[50];
                          snprintf(sampleFile, sizeof(sampleFile), "%s%d.bin", prefix, sampleLabel);
                          FT8Utilities::writeFile(sampleFile, windowOfIQData, sampleBufferSize);
                        }
                        fflush(stdout);  // flush standard out to make file output sane
                      } else {  // nothing to do yet
                        //fprintf(stderr, "waiting to start\n");
                        sleep(0.3);
                      }
                    }
                  }
                  for (auto i : hash22) {
                    free(i.second);  // release memory in hash
                  }
                  delete fftObject;
                };
  std::thread process(search);
  bool firstTime = true;
  WindowOfIQDataT entry;
  while (!done) {
    // get a Window worth of samples
    while ((background || firstTime || windows.empty()) && !done) {
      fprintf(stderr, "first time or there is a background process or the queue is empty\n");
      fprintf(stdout, "Starting a window at %ld, background is %d\n", time(0) - baseTime, background);
      if (!firstTime) {
        float remainsOf2Win[(PERIOD - PROCESSING_SIZE) * BASE_BAND * 2];
        // Before reading another sample set, disard the remaining samples associated with the previous 2 minute block
        fprintf(stderr, "Discarding %d unused samples of last window\n",
                (PERIOD - PROCESSING_SIZE) * BASE_BAND * 2);
        fread(remainsOf2Win, sizeof(float), (PERIOD - PROCESSING_SIZE) * BASE_BAND * 2, stdin);
      }
      fprintf(stderr, "allocating window IQ memory - %ld bytes\n", (int)freq  * sizeof(float) * 2 * PROCESSING_SIZE);
      now = time(0);
      entry = {now, reinterpret_cast<float *> (malloc((int)freq * sizeof(float) * 2 * PROCESSING_SIZE))};
      fprintf(stderr, "\nCollecting %d samples at %ld - %s", sampleBufferSize, now - baseTime, ctime(&now));
      if ((count = fread(entry.data, sizeof(float), PROCESSING_SIZE * BASE_BAND * 2, stdin)) == 0) {
        fprintf(stderr, "Input read was empty, sleeping for a while at %s", ctime(&now));
        sleep(1.0);
        done = true;
        break;
      } else {
        if (windows.size() < 10) {
          windowsMutex.lock();
          windows.push(entry);
          fprintf(stderr, "Queue now has %ld entries\n", windows.size());
          fprintf(stdout, "Queue now has %ld entries\n", windows.size());
          windowsMutex.unlock();
        } else {  // fell too far behind, don't queue new window
          fprintf(stderr, "Not queuing the window -- fallen too far behind\n");
          fprintf(stdout, "Not queuing the window -- fallen too far behind\n");
          free (entry.data);
        }
      }
      firstTime = false;
    }
    sleep(0.3);

  }
  terminate = true;
  process.join();  // wait for search thread to finish
  fprintf(stderr, "leaving doWork within FT8Window\n");
}

// compare for qsort
int FT8Window::SNRCompare(const void * a, const void * b) {
  if ((*(const SNRInfo *)a).magnitude < (*(const SNRInfo *)b).magnitude) {
    return -1;
  } else {
    return (*(const SNRInfo *)a).magnitude > (*(const SNRInfo *)b).magnitude;
  }
}
void FT8Window::calculateSNR(float * accumulatedMagnitude) {
  int regionOfInterestCount = 0;
  //int regionSize = 75.0 * size / BASE_BAND;
  int regionSize = size - 2800.0 * size / BASE_BAND;  // care about 2800 Hz of the 3200 Hz bandwidth
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
    // note: 17dB constant was calculated the same way WSPR constant of 26.2 (ie 10*log(2500 Hz / 50 Hz))
    //       2500 Hz bandwith of USB, 50 Hz bandwith of FT8 signal
    SNRData[i].SNR = 20 * log10(working[regionOfInterestCount - i - 1].magnitude) - noisePowerdB - 17.0;
    fprintf(stderr, "SNRData[%2d]: %10.0f, bin: %d, SNR: %f dB\n", i, SNRData[i].magnitude, SNRData[i].bin,
            SNRData[i].SNR);
    fprintf(stderr, "SNRAlt[%2d]: %10.0f, bin: %d, SNR: %f dB\n", i, SNRData[i].magnitude, SNRData[i].bin,
            10 * log10(working[regionOfInterestCount - i - 1].magnitude) - 10 * log10(noisePower) - 17.0);
    
  }
  free(working);
}

float FT8Window::getSNR(int bin) {
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

FT8Window::~FT8Window(void) {
  fprintf(stderr, "destructing FT8Window\n");
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
  float dialFreq = 14074000.0;
  snprintf(pre, sizeof(pre), "%s", "");
  snprintf(id, sizeof(id), "%s", "KG5YJE/P");
  snprintf(loc, sizeof(loc), "%s", "EM13");
  FT8Window testObj(512, 9, pre, dialFreq, id, loc );
  testObj.doWork();
}
#endif

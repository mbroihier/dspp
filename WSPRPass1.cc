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

/* ---------------------------------------------------------------------- */
void WSPRPass1::init(int size, int number, char * prefix) {
  this->size = size;
  this->number = number;
  this->prefix = prefix;
  freq = 375.0;
  fprintf(stderr, "allocating binArray memory\n");
  binArray = reinterpret_cast<int *>(malloc(number * sizeof(int)));
  used = reinterpret_cast<bool *>(malloc(number * sizeof(bool)));
  fprintf(stderr, "allocating samples memory\n");
  samples = reinterpret_cast<float *>(malloc(size * sizeof(float) * 2));
  fprintf(stderr, "allocating mag memory\n");
  mag = reinterpret_cast<float *>(malloc(size * sizeof(float)));
  sampleBufferSize = size * 2;
  histogram = reinterpret_cast<int *>(malloc(size * sizeof(int)));
  tic = 0;
  for (int i = 0; i < size; i++) {
    histogram[i] = 0;
  }
  allCandidates = reinterpret_cast<CandidateRecord *>(malloc((size/4) * sizeof(CandidateRecord)));
  for (int i = 0; i < size/4; i++) {
    allCandidates[i].timeStamp = -1;
    allCandidates[i].range.lowerBound = i * 4 - 0.5;
    allCandidates[i].range.upperBound = i * 4 + 3.5;
    allCandidates[i].centroid = (allCandidates[i].range.upperBound + allCandidates[i].range.lowerBound) / 2.0;
    allCandidates[i].floatingCandidateID = -1;
    allCandidates[i].history = new std::list<SampleRecord>;
  }
  alreadyUpdated = reinterpret_cast<bool *>(malloc((size/4) * sizeof(bool)));
}

WSPRPass1::WSPRPass1(int size, int number, char * prefix) {
  fprintf(stderr, "creating WSPRPass1 object\n");
  init(size, number, prefix);
  fprintf(stderr, "done creating WSPRPass1 object\n");
}

void WSPRPass1::regressionFit(std::list<float> centroidList) {
  float sumX = 0.0;
  float sumY = 0.0;
  float sumXY = 0.0;
  float sumX2 = 0.0;
  int count = 0;
  // produce regression terms
  for (std::list<float>::iterator iter = centroidList.begin(); iter != centroidList.end(); iter++) {
    sumY += *iter;
    sumX += count;
    sumXY += *iter * count;
    sumX2 += count * count;
    count++;
  }
  slope = (count * sumXY - sumX * sumY) / (count * sumX2 - sumX * sumX);
  yIntercept = sumY / count - slope * sumX / count;
  fprintf(stderr, "linear fit of centroid data - slope: %7.2f, y-intercept: %7.2f\n", slope, yIntercept);
}

void WSPRPass1::convertToSymbols(std::list<float> centroidList) {
  const unsigned char sync[162] = {
     1,1,0,0,0,0,0,0,1,0,0,0,1,1,1,0,0,0,1,0,0,1,0,1,1,1,1,0,0,0,0,0,
     0,0,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,1,0,0,1,1,0,1,0,0,0,1,1,0,1,0,
     0,0,0,1,1,0,1,0,1,0,1,0,1,0,0,1,0,0,1,0,1,1,0,0,0,1,1,0,1,0,1,0,
     0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,1,1,0,1,1,0,0,1,1,0,1,0,0,0,1,1,1,
     0,0,0,0,0,1,0,1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,0,1,1,0,0,0,1,1,0,
     0,0 };
  const unsigned char token[4][24] = {
     0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,3,3,
     1,2,3,1,2,3,0,0,2,2,3,3,0,0,1,1,3,3,0,0,1,1,2,2,
     2,3,1,3,1,2,2,3,0,3,0,2,1,3,0,3,0,1,1,2,0,2,0,1,
     3,1,2,2,3,1,3,2,3,0,2,0,3,1,3,0,1,0,2,1,2,0,1,0 };
  for (int j = 0; j < 24; j++) {
    for (int i = 0; i < 4; i++) {
      fprintf(stderr, " %d ", token[i][j]);
    }
    fprintf(stderr, "\n");
  }
  bool first = true;
  float minValue = 0.0;
  float maxValue = 0.0;
  float drift = 0.0;
  float average = 0.0;
  float accumulator = 0.0;
  float sumSquares = 0.0;
  int count = 0;
  for (std::list<float>::iterator iter = centroidList.begin(); iter != centroidList.end(); iter++) {
    accumulator += *iter;
    sumSquares += *iter * *iter;
    count++;
    if (first) {
      minValue = *iter;
      maxValue = *iter;
      first = false;
    } else {    
      if (*iter < minValue) {
        minValue = *iter;
      } else if (*iter > maxValue) {
        maxValue = *iter;
      }
    }
  }
  average = accumulator / count;
  float std = sqrt((sumSquares - (accumulator * accumulator) / count)/ (count - 1));
  drift = maxValue - minValue - 3.0;
  fprintf(stderr, "maximum centroid observed was: %7.2f, minimum centroid observed was: %7.2f, drift is: %7.2f, "
          "average is: %7.2f, std is: %7.2f\n", maxValue, minValue, drift, average, std);
  std::list<int> bestGuessSymbols;
  int bestGuessTokenColumn = 0;
  int bestMatchCount = 0;
  int bestShiftValue = 0;
  if (drift >= 3.0) {
    fprintf(stderr, "attempting to tokenize centroid list\n");
    regressionFit(centroidList);
    float x = 0.0;
    float minExpected = 0.0;
    float maxExpected = 0.0;
    if (slope < 0.0) {  // The maximum value of the signal should occur near zero, the minimum value near count
      minExpected = yIntercept + slope * count - 1.5;
      maxExpected = yIntercept + 1.5;
    } else {
      minExpected = yIntercept - 1.5;
      maxExpected = yIntercept + slope * count + 1.5;
    }
    std::list<int> tokenList;
    fprintf(stderr, "expected bounds are: %7.2f and %7.2f\n", minExpected, maxExpected);
    for (std::list<float>::iterator iter = centroidList.begin(); iter != centroidList.end(); iter++) {
      float expectedY = yIntercept + slope * x;
      float base = expectedY - 1.5;
      int token = std::min(std::max((int)(*iter - base + 0.5),0),3);
      tokenList.push_back(token);
      fprintf(stderr, "sample %3d - expected: %7.2f, actual: %7.2f, error: %7.2f, token: %d\n",
              (int) x, expectedY, *iter, expectedY - *iter, token);
      x += 1.0;
    }
    /*
    const unsigned char testTokens [172] = {0,0,0,0,0,
                                            1,1,0,0,0,0,0,0,1,0,0,0,1,1,1,0,0,0,1,0,0,1,0,1,1,1,1,0,0,0,0,0,
                                            0,0,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,1,0,0,1,1,0,1,0,0,0,1,1,0,1,0,
                                            0,0,0,1,1,0,1,0,1,0,1,0,1,0,0,1,0,0,1,0,1,1,0,0,0,1,1,0,1,0,1,0,
                                            0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,1,1,0,1,1,0,0,1,1,0,1,0,0,0,1,1,1,
                                            0,0,0,0,0,1,0,1,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,0,1,1,0,0,0,1,1,0,
                                            0,0,
                                            0,0,0,0,0};
    tokenList.clear();
    for (unsigned int i = 0; i < sizeof(testTokens); i++) {
      tokenList.push_back(testTokens[i]);
    }
    */
    for (int col = 0; col < 24; col++) {  //translate the tokens to symbols and evaluate
      int matchCount = 0;
      std::list<int> testSymbols;
      int listCount = 0;
      for (std::list<int>::iterator iter = tokenList.begin(); iter != tokenList.end(); iter++) {
        testSymbols.push_back(token[*iter][col]);
        listCount++;
      }
      int shiftRange = listCount - 162;  // this is the number of shifts that can be made in the token list
      for (int i = 0; i < shiftRange; i++) {
        int currentShift = 0;
        matchCount = 0;
        int syncIndex = 0;
        for (std::list<int>::iterator iter = testSymbols.begin();
             (iter != testSymbols.end()) && (syncIndex < 162); iter++) {
          if (currentShift++ < i) continue;  // go to next list value
          matchCount += ((*iter & 0x01) == sync[syncIndex++]) ? 1:0;
        }
        fprintf(stderr, "Match count for token column %d shift %d was %d\n", col, i, matchCount);
        if (matchCount >= bestMatchCount) {  // this is the best match we've seen so far, record it
          bestShiftValue = i;
          bestGuessTokenColumn = col;
          bestGuessSymbols.clear();
          currentShift = 0;
          fprintf(stderr, "-------------\n");
          for (std::list<int>::iterator iter = testSymbols.begin(); iter != testSymbols.end(); iter++) {
            if (currentShift++ < i) continue;  // go to next list value
            bestGuessSymbols.push_back(*iter);
            fprintf(stderr, "%d\n", *iter);
          }
          bestMatchCount = matchCount;
        }
      }
    }
    fprintf(stderr, "Best match with sycn was with token column %d, shifted by %d, count of %d\n",
            bestGuessTokenColumn, bestShiftValue, bestMatchCount);
    fprintf(stderr, "Final symbol list:\n");
    for (std::list<int>::iterator iter = bestGuessSymbols.begin(); iter != bestGuessSymbols.end(); iter++) {
      fprintf(stderr, "%d\n", *iter);
    }
          
  } else {
    fprintf(stderr, "there is not enough frequency range to tokenize this list\n");
  }
}
void WSPRPass1::doWork() {
  std::map<int, float> groupCentroids;  // mapped by group ID
  std::map<int, SpotCandidate *> candidatesPass1;
  int frame = 0;
  int count = 0;
  float * samplePtr;
  float * magPtr;
  fprintf(stderr, "Find WSPR signals pass 1\n", number);
  bool done = false;
  float wallClock = 0.0;
  float deltaTime = 1.0 / freq * size;
  while (!done) {
    // get an FFT's worth of bins
    fprintf(stderr, "done with set of data\n");
    count = fread(samples, sizeof(float), sampleBufferSize, stdin);
    fprintf(stderr, "done with read for tic %d, wall clock %7.2f\n", tic, wallClock += deltaTime);
    if (count < sampleBufferSize) {
      done = true;
      continue;
    }
    for (int i = 0; i < number; i++) {
      used[i] = false;
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
    fprintf(stderr, "bin should be %d, it is %d\n", size, bin);
    // make a sorted list (up to number) of bin numbers that contain the highest frequency magnitudes
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
    // output this array of integer bin numbers that are the indices of the highest amplitude frequencies
    // fwrite(binArray, sizeof(int), number, stdout);
    fprintf(stderr, "binArray on frame %d\n", frame++);
    for (int binIndex = 0; binIndex < number; binIndex++) {
      fprintf(stderr, "binArray[%3d]: %3d, mag[binArray[%3d]: %f \n", binIndex, binArray[binIndex],
              binIndex, mag[binArray[binIndex]]);
    }
    
    // now group the bins that are adjacent and find the center bin of these groups
    std::map <int, int> binToGroup;
    std::map <int, std::list<int> *> groupToBins;
    int groupNumber = 0;
    int numberOfGroups = 0;
    int rememberedBinIndex = 0;
    for (int bin = 0; bin < size; bin++) {  // walk through all the bins
      bool inBinArray = false;
      for (int binIndex = 0; binIndex < number; binIndex++) {
        if (binArray[binIndex] == bin) {
          inBinArray = true;
          rememberedBinIndex = binIndex;
        }
      }
      bool escape = false;
      if (inBinArray) {
        for (int groupIndex = 0; groupIndex < numberOfGroups; groupIndex++) {
          for (std::list<int>::iterator iter = groupToBins[groupIndex]->begin(); iter != groupToBins[groupIndex]->end(); iter++) {
            if (abs(bin - *iter) <= 2 && !used[rememberedBinIndex]) { // this bin belongs to this group
              groupToBins[groupIndex]->push_back(bin);
              binToGroup[bin] = groupIndex;
              used[rememberedBinIndex] = true;
              escape = true;
              break;  // done with is bin
            }
          }
          if (escape) break;
        }
        if (!escape && !used[rememberedBinIndex]) {  // this bin is in the bin array, and not part of any of the
          // groups we last looked at, so make a new group
          used[rememberedBinIndex] = true;
          binToGroup[bin] = groupNumber;
          groupToBins[groupNumber] = new std::list<int>;
          groupToBins[groupNumber]->push_back(bin);
          groupNumber++;
          numberOfGroups++;
        }
      }
    }
    // find the centroids of each group
    for (int groupIndex = 0; groupIndex < numberOfGroups; groupIndex++) {
      fprintf(stderr, "bins in group %d:", groupIndex);
      groupToBins[groupIndex]->sort();
      //float minMag = mag[binArray[number-1]];
      float minMag = mag[size/2];
      float maxMag = minMag;
      for (std::list<int>::iterator iter = groupToBins[groupIndex]->begin(); iter != groupToBins[groupIndex]->end(); iter++) {
        if (mag[*iter] < minMag) {
          minMag = mag[*iter];
        }
        if (mag[*iter] > maxMag) {
          maxMag = mag[*iter];
        }
      }
      float clip = (maxMag - minMag) / 3.0 + minMag;
      float weightedCentroid = 0.0;
      float accumulator = 0.0;
      for (std::list<int>::iterator iter = groupToBins[groupIndex]->begin(); iter != groupToBins[groupIndex]->end(); iter++) {
        fprintf(stderr, " %d", *iter);
        if (mag[*iter] >= clip) {
          weightedCentroid += *iter * mag[*iter];
          accumulator += mag[*iter];
        }
      }
      if (accumulator > 1.0) {
        weightedCentroid = weightedCentroid / accumulator;
        float observedFreq = weightedCentroid * (freq/size);
        if (observedFreq > freq / 2.0) {
          observedFreq = -freq + observedFreq;
        }
        fprintf(stderr," --- weighted centroid: %7.2f, freq: %7.2f\n\n",weightedCentroid, observedFreq);
        int id = (int) (weightedCentroid + 0.5);
        if (candidatesPass1.end() == candidatesPass1.find(id)) {
          candidatesPass1[id] = new SpotCandidate(id);
        }
        candidatesPass1[id]->
          logSample(observedFreq, mag[id], tic, wallClock);
      } else {
        weightedCentroid = 0.0;
        fprintf(stderr, " -- weighted centroid forced to zero\n\n");
      }
      groupCentroids[groupIndex] = weightedCentroid;  // indexed by groupIndex
      histogram[(int) weightedCentroid]++;
      // map this group centroid to a fixed Candidate
      int fixedIndex = ((int) weightedCentroid + 0.5) / 4;
      SampleRecord sr;
      sr.centroid = groupCentroids[groupIndex];
      allCandidates[fixedIndex].groupIndexUsed = false;
      sr.magnitude = mag[(int) sr.centroid];
      sr.timeStamp = tic;
      allCandidates[fixedIndex].timeStamp = tic;
      allCandidates[fixedIndex].history->push_back(sr);
      allCandidates[fixedIndex].groupIndexUsed = false;
    }
    for (int canID = 0; canID < size/4; canID++) {
      alreadyUpdated[canID] = false;
    }
    tic++;
  }
  for (std::map<int, SpotCandidate *>::iterator iter = candidatesPass1.begin(); iter != candidatesPass1.end(); iter++) {
    (*iter).second->printReport();
  }
  int mostLikelyCandidate = -1;
  int freqBin = -1;
  for (std::map<int, SpotCandidate *>::iterator iter = candidatesPass1.begin(); iter != candidatesPass1.end(); iter++) {
    if ((*iter).second->getCount() > mostLikelyCandidate) {
      mostLikelyCandidate = (*iter).second->getCount();
      freqBin = (*iter).first;
    }
  }
  fprintf(stderr, "The most likely candidate with count %d was found at frequency bin %d\n", mostLikelyCandidate,
          freqBin);
  int mergeLowerBin = freqBin - 1;
  int mergeUpperBin = freqBin + 1;
  if (mergeLowerBin < 0) mergeLowerBin = size - 1;
  if (mergeUpperBin > size - 1) mergeUpperBin = 0;
  fprintf(stderr, "Will attempt to merge bins around %d, which are %d and %d\n", freqBin, mergeLowerBin, mergeUpperBin);
  int firstMergeBin = -1;
  int secondMergeBin = -1;
  if ((candidatesPass1.end() != candidatesPass1.find(mergeLowerBin)) && (candidatesPass1.end() != candidatesPass1.find(mergeUpperBin))) {
    if (candidatesPass1[mergeLowerBin]->getCount() > candidatesPass1[mergeUpperBin]->getCount()) {
      firstMergeBin = mergeLowerBin;
      secondMergeBin = mergeUpperBin;
    } else {
      firstMergeBin = mergeUpperBin;
      secondMergeBin = mergeLowerBin;
    }
  } else {
    if (candidatesPass1.end() != candidatesPass1.find(mergeLowerBin)) {  // lower bin exists, upper doesn't
      firstMergeBin = mergeLowerBin;
    } else {
      firstMergeBin = mergeUpperBin;
    }
  }
  if (firstMergeBin >= 0) {
    fprintf(stderr, "Merging list %d with list %d\n", freqBin, firstMergeBin);
    candidatesPass1[freqBin]->mergeList(candidatesPass1[firstMergeBin]->getList());
  }
  if (secondMergeBin >= 0) {
    fprintf(stderr, "Merging list %d with list %d\n", freqBin, secondMergeBin);
    candidatesPass1[freqBin]->mergeList(candidatesPass1[secondMergeBin]->getList());
  }
  candidatesPass1[freqBin]->printReport();
  int nextBin = 0;
  if ((firstMergeBin != -1) && firstMergeBin < freqBin) {
    nextBin = firstMergeBin - 1;
    if (nextBin < 0) nextBin = size - 1;
  } else {
    nextBin = firstMergeBin + 1;
    if (nextBin >= size) nextBin = 0;
  }
  fprintf(stderr, "Merging list %d with list %d\n", freqBin, nextBin);
  candidatesPass1[freqBin]-> mergeList(candidatesPass1[nextBin]->getList());
  candidatesPass1[freqBin]->printReport();
  const std::list<SpotCandidate::SampleRecord> bestList1 = candidatesPass1[freqBin]->getValidSublist(1);
  SpotCandidate * best1 = new SpotCandidate(999, bestList1);
  best1->printReport();
  const std::list<SpotCandidate::SampleRecord> bestList2 = candidatesPass1[freqBin]->getValidSublist(2);
  SpotCandidate * best2 = new SpotCandidate(998, bestList2);
  best2->printReport();
  const std::list<SpotCandidate::SampleRecord> bestList3 = candidatesPass1[freqBin]->getValidSublist(3);
  SpotCandidate * best3 = new SpotCandidate(997, bestList3);
  best3->printReport();
  fprintf(stderr, "leaving doWork within WSPRPass1\n");
  delete(best1);
  delete(best2);
  delete(best3);
}

WSPRPass1::~WSPRPass1(void) {
  fprintf(stderr, "destructing WSPRPass1\n");
  if (histogram) free(histogram);
  if (used) free(used);
  if (mag) free(mag);
  if (binArray) free(binArray);
  if (samples) free(samples);
  if (allCandidates) free(allCandidates);
  if (alreadyUpdated) free(alreadyUpdated);
}


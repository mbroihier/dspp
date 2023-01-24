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

void WSPRPass1::adjustTargets(float centroid, int candidate) {
  bool adjust = false;
  int limit = 0;
  if (targets.end() == targets.find(candidate)) {
    targets[candidate] = new std::map<int, float>;
    (*targets[candidate])[TARGET0] = centroid;
    (*targets[candidate])[TARGET1] = centroid;
    (*targets[candidate])[TARGET2] = centroid;
    (*targets[candidate])[TARGET3] = centroid;
    (*targets[candidate])[REF1] = centroid;
    (*targets[candidate])[REF2] = centroid;
  } else {
    if (centroid < (*targets[candidate])[REF1]) {
      (*targets[candidate])[REF1] = centroid;
      adjust = true;
      limit = REF2;
    } else if (centroid > (*targets[candidate])[REF2]) {
      (*targets[candidate])[REF2] = centroid;
      adjust = true;
      limit = REF1;
    }
  }
  if (adjust) {
    if ((*targets[candidate])[REF2] - (*targets[candidate])[REF1] > BW) {
      if (limit == REF1) {
        (*targets[candidate])[REF1] = (*targets[candidate])[REF2] - BW;
      } else {
        (*targets[candidate])[REF2] = (*targets[candidate])[REF1] + BW;
      }
      (*targets[candidate])[TARGET0] = (*targets[candidate])[REF1];
      (*targets[candidate])[TARGET3] = (*targets[candidate])[REF2];
      (*targets[candidate])[TARGET1] = (*targets[candidate])[TARGET0] + BW_DELTA;
      (*targets[candidate])[TARGET2] = (*targets[candidate])[TARGET3] - BW_DELTA;
      fprintf(stderr, "adjusting targets for candidate %d: %f, %f, %f, %f - %f\n", candidate,
              (*targets[candidate])[TARGET0], (*targets[candidate])[TARGET1], (*targets[candidate])[TARGET2],
              (*targets[candidate])[TARGET3], centroid);
    } else {
      fprintf(stderr, "not adjusting targets for candidate %d: %f, %f, %f, %f - %f\n", candidate,
              (*targets[candidate])[TARGET0], (*targets[candidate])[TARGET1], (*targets[candidate])[TARGET2],
              (*targets[candidate])[TARGET3], centroid);
    }
  } else {
      fprintf(stderr, "not adjusting targets for candidate %d: %f, %f, %f, %f - %f\n", candidate,
              (*targets[candidate])[TARGET0], (*targets[candidate])[TARGET1], (*targets[candidate])[TARGET2],
              (*targets[candidate])[TARGET3], centroid);
  }
  if ((*targets[candidate])[TARGET0] != (*targets[candidate])[TARGET3]) {
    logBase((*targets[candidate])[TARGET0], candidate);
  }
}
int WSPRPass1::findClosestTarget(float centroid, int candidate) {
  float shortestDistance = BW;
  int closest = TARGET0;
  for (int i = 0; i < 4; i++) {
    float delta = fabs(centroid - (*targets[candidate])[i]);
    if (delta < shortestDistance) {
      shortestDistance = delta;
      closest = i;
    }
  }
  return closest;
}

void WSPRPass1::logCentroid(float centroid, int candidate) {
  if (centroidHistory.end() == centroidHistory.find(candidate)) {  // this is the first time logging this candidate
    centroidHistory[candidate] = new std::list<SampleRecord>;
  }
  SampleRecord sr;
  sr.centroid = centroid;
  sr.timeStamp = tic;
  centroidHistory[candidate]->push_back(sr);
  fprintf(stderr, "recording history for candidate %d\n", candidate);
}

void WSPRPass1::logBase(float baseValue, int candidate) {
  if (centroidHistory.end() == centroidHistory.find(candidate)) {
    fprintf(stderr, "Internal error - attempting to log a base value prior to having a history of centroids\n");
    return;
  }
  if (baseHistory.end() == baseHistory.find(candidate)) {  // this is the first time logging this candidate
    baseHistory[candidate] = new std::list<BaseRecord>;
    BaseRecord br;
    br.base = baseValue;
    br.timeStamp = tic;
    for (unsigned int i = 0; i < centroidHistory[candidate]->size(); i++) {
      baseHistory[candidate]->push_back(br);
    }
  } else {
    BaseRecord br;
    br.base = baseValue;
    br.timeStamp = tic;
    baseHistory[candidate]->push_back(br);
  }
  fprintf(stderr, "recording base for candidate %d size is now: %d\n", candidate, baseHistory[candidate]->size());
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
void WSPRPass1::reportHistory(int numberOfCandidates) {
  fprintf(stderr, "Number of candidates: %3d\n", numberOfCandidates);
  for (int i = 0; i < numberOfCandidates; i++) {
    if (baseHistory.end() == baseHistory.find(i) || centroidHistory.end() == centroidHistory.find(i)) {
      fprintf(stderr, "Candidate %d is not valid - it was a constant frequency: %f\n", i, candidates[i]);
    } else {
      fprintf(stderr, "History Report for Candidate: %d\n", i);
      int j = 0;
      if (centroidHistory[i]->size() < 162) {
        fprintf(stderr, "Candidate %d can not be valid - it does not have enough samples (%d), %f\n", i,
                centroidHistory[i]->size(), candidates[i]);
      } else {
        int lastTimeStamp = 0;
        int sequentialSamples = 1;
        bool enoughSequentialSamples = false;
        std::list<float> demodedCandidateCentroid;
        std::list<BaseRecord>::iterator iter2 = baseHistory[i]->begin();
        for (std::list<SampleRecord>::iterator iter1 = centroidHistory[i]->begin();
             iter1 != centroidHistory[i]->end(); iter1++, iter2++, j++) {
          if ((*iter1).timeStamp == lastTimeStamp + 1) {
            sequentialSamples++;
            fprintf(stderr, "Sample %3d: %5.2f, %5.2f, %5d, %5d, %5d, *, %5d\n", j, (*iter1).centroid, (*iter2).base,
                    (int) floor((*iter1).centroid - (*iter2).base +0.5), (*iter1).timeStamp,
                    (*iter2).timeStamp, sequentialSamples);
            demodedCandidateCentroid.push_back((*iter1).centroid);
            if (sequentialSamples > 161) enoughSequentialSamples = true;
          } else {
            if (enoughSequentialSamples) {
              fprintf(stderr, "A demodulated list of centroids to process\n");
              int ctr = 0;
              for (std::list<float>::iterator diter = demodedCandidateCentroid.begin(); diter != demodedCandidateCentroid.end(); diter++) {
                fprintf(stderr, " %3d: %7.2f\n", ctr++, *diter);
              }
              convertToSymbols(demodedCandidateCentroid);
              enoughSequentialSamples = false;
            }
            sequentialSamples = 1;
            demodedCandidateCentroid.clear();
            demodedCandidateCentroid.push_back((*iter1).centroid);
            fprintf(stderr, "Sample %3d: %5.2f, %5.2f, %5d, %5d, %5d\n", j, (*iter1).centroid, (*iter2).base,
                    (int) floor((*iter1).centroid - (*iter2).base +0.5), (*iter1).timeStamp, (*iter2).timeStamp);
          }
          lastTimeStamp = (*iter1).timeStamp;
        }
        if (enoughSequentialSamples) {
          fprintf(stderr, "This candidate has enough sequential samples to be submitted to FANO\n");
        }
      }
    }
  }
  fprintf(stderr, "Histogram\n");
  int sum = 0;
  int bins[6];
  for (int i = 0; i < 6; i++) {
    bins[i] = 0;
  }
  for (int i = 0; i < size; i++) {
    sum -= bins[i % 6];
    sum += histogram[i];
    bins[i % 6] = histogram[i];
    fprintf(stderr, "histogram[%3d]: %4d, %5d\n", i, histogram[i], sum);
  }
  fprintf(stderr, "Fixed Candidate History Records\n");
  for (int i = 0; i < size / 4; i++) {
    fprintf(stderr, "Fixed Candidate %d, associated floating candidate ID: %d\n", i,
            allCandidates[i].floatingCandidateID);
    if (allCandidates[i].history->size() > 0) {
      if (allCandidates[i].floatingCandidateID == -1) {
        fprintf(stderr, "used - but not assigned to a floating candidate, history size = %d\n",
                allCandidates[i].history->size());
        int j = 0;
        for (std::list<SampleRecord>::iterator iter = allCandidates[i].history->begin();
             iter != allCandidates[i].history->end(); iter++, j++) {
          fprintf(stderr, "history[%3d]: %5.2f, %15.2f, ???, %4d\n", j, (*iter).centroid, (*iter).magnitude,
                  (*iter).timeStamp);
        }
      } else {
        int j = 0;
        for (std::list<SampleRecord>::iterator iter = allCandidates[i].history->begin();
             iter != allCandidates[i].history->end(); iter++, j++) {
          fprintf(stderr, "history[%3d]: %5.2f, %15.2f, %2d, %4d\n", j, (*iter).centroid, (*iter).magnitude,
                  findClosestTarget((*iter).centroid, allCandidates[i].floatingCandidateID),
                  (*iter).timeStamp);
        }
      }
    } else {
      fprintf(stderr, "not used\n");
    }
  }
}


void WSPRPass1::doWork() {
  int numberOfCandidates = 0;
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
    /*
    // Now track floating candidates through the fixed candidates
    for (std::map<int, float>::iterator iter = candidates.begin(); iter != candidates.end(); iter++) {
      // using the last centroid,  see if it has been used in assigning a candidate and see if this candidate
      // previously was assigned to this fixed candidate
      floatingCandidateUpdated[(*iter).first] = false;
      int lastFixedIndex = ((int) (*iter).second + 0.5) / 4;
      fprintf(stderr, "Looking at old floating candidate: %d, that has a last fixed candidate of: %d\n",
              (*iter).first, lastFixedIndex);
      if ((! alreadyUpdated[lastFixedIndex]) && (tic == allCandidates[lastFixedIndex].timeStamp)) {
        // has this candidate been updated this cycle and used?
        if ((allCandidates[lastFixedIndex].floatingCandidateID == (*iter).first) ||
            allCandidates[lastFixedIndex].floatingCandidateID == -1) {
          // this has been used before by this candidate or never used
          assert(allCandidates[lastFixedIndex].floatingCandidateID != -1);  // ? shouldn't happen
          candidates[(*iter).first] = allCandidates[lastFixedIndex].history->back().centroid;
          floatingCandidateUpdated[(*iter).first] = true;
          alreadyUpdated[lastFixedIndex] = true;
          allCandidates[lastFixedIndex].groupIndexUsed = true;  // suppress creation of floating
          allCandidates[lastFixedIndex].floatingCandidateID = (*iter).first;  // should be necessary
          fprintf(stderr, "updated fixed candidate %d info with floating candidate %d, centroid %f\n", lastFixedIndex,
                  allCandidates[lastFixedIndex].floatingCandidateID, candidates[(*iter).first]);
        } else {
          // The previous test is the simple case, but it may be that this was previously assigned a
          // different floating candidate ID.  At the moment, we will just discard this sample
          alreadyUpdated[lastFixedIndex] = true;
          fprintf(stderr, "We are dropping this group centroid %f this cycle\n",
                  allCandidates[lastFixedIndex].history->back().centroid);
        }
      } else {  // this centroid didn't fall into a fixed candiate that was updated this round, check a neighbor
        int otherNeighbor = lastFixedIndex;
        // look at lower neighbor
        fprintf(stderr, "looking at lower fixed neighbor\n");
        otherNeighbor--;
        if (otherNeighbor < 0) otherNeighbor = size/4 - 1;
        if ((! alreadyUpdated[otherNeighbor]) && (tic == allCandidates[otherNeighbor].timeStamp)) {
          // has this candidate been updated this cycle and used?
          if ((allCandidates[otherNeighbor].floatingCandidateID == (*iter).first) ||
              allCandidates[otherNeighbor].floatingCandidateID == -1) {
            // this has been used before by this candidate or never used
            candidates[(*iter).first] = allCandidates[otherNeighbor].history->back().centroid;
            floatingCandidateUpdated[(*iter).first] = true;
            alreadyUpdated[otherNeighbor] = true;
            allCandidates[otherNeighbor].groupIndexUsed = true;  // suppress creation of floating
            allCandidates[otherNeighbor].floatingCandidateID = (*iter).first;
            fprintf(stderr, "updated fixed candidate %d info with floating candidate %d (other)\n", otherNeighbor,
                    allCandidates[otherNeighbor].floatingCandidateID);
          } else {
            // The previous test is the simple case, but it may be that this was previously assigned a
            // different floating candidate ID.  At the moment, we will just discard this sample
            // alreadyUpdated[otherNeighbor] = true;
            fprintf(stderr, "We are deferring this group centroid %f this cycle (other neighbor path), value: %d\n",
                    allCandidates[otherNeighbor].history->back().centroid, otherNeighbor);
            fprintf(stderr, "Other information: original Fixed Candidate: %d, floating candidate ID for it: %d\n"
                    " other Fixed Candidate floating candidate ID: %d\n"
                    " this centriod: %f\n", lastFixedIndex,
                    allCandidates[lastFixedIndex].floatingCandidateID,
                    allCandidates[otherNeighbor].floatingCandidateID,
                    allCandidates[otherNeighbor].history->back().centroid);
          }
        } else {  // this centroid didn't fall into a fixed candiate neighbor that was updated this round
          fprintf(stderr, "floating cand %d (fixed %d) is not being updated - alreadyUpdated is %d or tic %d"
                  " is not %d\n", (*iter).first, lastFixedIndex, alreadyUpdated[lastFixedIndex],
                  tic, allCandidates[lastFixedIndex].timeStamp);
          fprintf(stderr, "floating cand %d (fixed %d) is not being updated - alreadyUpdated is %d or tic %d"
                  " is not %d\n", (*iter).first, otherNeighbor, alreadyUpdated[otherNeighbor],
                  tic, allCandidates[otherNeighbor].timeStamp);
        }
        // look at upper neighbor
        fprintf(stderr, "looking at the upper fixed neighbor\n");
        otherNeighbor = lastFixedIndex + 1;
        if (otherNeighbor >= size/4) otherNeighbor = 0;
        if ((! alreadyUpdated[otherNeighbor]) && (tic == allCandidates[otherNeighbor].timeStamp)) {
          // has this candidate been updated this cycle and used?
          if ((allCandidates[otherNeighbor].floatingCandidateID == (*iter).first) ||
              allCandidates[otherNeighbor].floatingCandidateID == -1) {
            // this has been used before by this candidate or never used
            candidates[(*iter).first] = allCandidates[otherNeighbor].history->back().centroid;
            floatingCandidateUpdated[(*iter).first] = true;
            alreadyUpdated[otherNeighbor] = true;
            allCandidates[otherNeighbor].groupIndexUsed = true;  // suppress creation of floating
            allCandidates[otherNeighbor].floatingCandidateID = (*iter).first;
            fprintf(stderr, "updated fixed candidate %d info with floating candidate %d (other)\n", otherNeighbor,
                    allCandidates[otherNeighbor].floatingCandidateID);
          } else {
            // The previous test is the simple case, but it may be that this was previously assigned a
            // different floating candidate ID.  At the moment, we will just discard this sample
            // Discarding the group centroid did not work, will try keeping it 
            //alreadyUpdated[otherNeighbor] = true;
            fprintf(stderr, "We are deferring this group centroid %f this cycle (other neighbor path), value: %d\n",
                    allCandidates[otherNeighbor].history->back().centroid, otherNeighbor);
            fprintf(stderr, "Other information: original Fixed Candidate: %d, floating candidate ID for it: %d\n"
                    " other Fixed Candidate floating candidate ID: %d\n"
                    " this centriod: %f\n", lastFixedIndex,
                    allCandidates[lastFixedIndex].floatingCandidateID,
                    allCandidates[otherNeighbor].floatingCandidateID,
                    allCandidates[otherNeighbor].history->back().centroid);
          }
        } else {  // this centroid didn't fall into a fixed candiate neighbor that was updated this round
          fprintf(stderr, "floating cand %d (fixed %d) is not being updated - alreadyUpdated is %d or tic %d"
                  " is not %d\n", (*iter).first, lastFixedIndex, alreadyUpdated[lastFixedIndex],
                  tic, allCandidates[lastFixedIndex].timeStamp);
          fprintf(stderr, "floating cand %d (fixed %d) is not being updated - alreadyUpdated is %d or tic %d"
                  " is not %d\n", (*iter).first, otherNeighbor, alreadyUpdated[otherNeighbor],
                  tic, allCandidates[otherNeighbor].timeStamp);
        }
      }
    }
    // create new floating candidates if there are any left over group centroids, but stop if number of candidates
    // is equal to the number of bin levels being looked at
    if (numberOfCandidates < number) {
      for (int candID = 0; candID < size/4; candID++) {
        if (! alreadyUpdated[candID]) { // fixed candidate wasn't used yet, so lets see if was updated this pass
          if (allCandidates[candID].history->size() > 0) {  // there is a history of use
            if (allCandidates[candID].history->back().timeStamp == tic) { // was it updated this time
              if (allCandidates[candID].groupIndexUsed == false) {  // make a new floating candidate
                candidates[numberOfCandidates] = allCandidates[candID].history->back().centroid;
                floatingCandidateUpdated[numberOfCandidates] = true;
                alreadyUpdated[numberOfCandidates] = true;
                allCandidates[candID].floatingCandidateID = numberOfCandidates;
                fprintf(stderr, "made a new floating cand, %d, for fixed candidate %d\n",
                        numberOfCandidates, candID);
                FILE * fh;
                char fileName[50];
                sprintf(fileName, "%s%d.txt", prefix, numberOfCandidates); 
                fh = fopen(fileName, "w");  // empty file if it exists
                fclose(fh);
                numberOfCandidates++;
                fprintf(stderr, "New candidate list\n");
                for (std::map<int, float>::iterator iter = candidates.begin(); iter != candidates.end(); iter++) {
                  fprintf(stderr, "candidates[%d]: %f\n", (*iter).first, (*iter).second);
                }
                if (numberOfCandidates == number) break;
              } else {
                fprintf(stderr, "not making a floating cand for fixed candidate %d, because group index was used: %d\n",
                        candID, allCandidates[candID].groupIndexUsed);
              }
            } else {
              fprintf(stderr, "not making a floating cand for fixed candidate %d, because time stamp was old: %d\n",
                      candID, allCandidates[candID].history->back().timeStamp);
            }
          } else {
            fprintf(stderr, "not making a floating cand for fixed candidate %d, because no history\n", candID);
          }
        } else {
          fprintf(stderr, "not making a floating cand for fixed candidate %d, because fixed was already used/updated\n",
                  candID);
        }
      }
    }
              
    // within this FFT, output the centroids of candidates  
    fprintf(stderr, "Snapshot of floating candiates\n");
    for (int candidateIndex = 0; candidateIndex < numberOfCandidates; candidateIndex++) {
      FILE * fh;
      char fileName[50];
      sprintf(fileName, "%s%d.txt", prefix, candidateIndex);
      fh = fopen(fileName, "a");
      if (floatingCandidateUpdated[candidateIndex]) {
        logCentroid(candidates[candidateIndex], candidateIndex);
        fprintf(stderr, "on tic %d, ", tic);
        adjustTargets(candidates[candidateIndex], candidateIndex);
        fprintf(fh, "%5.2f, %d, %d\n", candidates[candidateIndex],
                findClosestTarget(candidates[candidateIndex], candidateIndex), tic);
        fprintf(stderr, "%3d: %5.2f, %d, %d\n", candidateIndex, candidates[candidateIndex],
                findClosestTarget(candidates[candidateIndex], candidateIndex), tic);
      } else {
        fprintf(fh, "%5.2f, %d, %d, not updated on this pass\n", candidates[candidateIndex],
                findClosestTarget(candidates[candidateIndex], candidateIndex), tic);
        fprintf(stderr, "%3d: %5.2f, %d, %d, not updated on this pass\n", candidateIndex, candidates[candidateIndex],
                findClosestTarget(candidates[candidateIndex], candidateIndex), tic);
      }
      fclose(fh);
    }
    */
    tic++;
  }
  reportHistory(numberOfCandidates);
  for (std::map<int, SpotCandidate *>::iterator iter = candidatesPass1.begin(); iter != candidatesPass1.end(); iter++) {
    (*iter).second->printReport();
  }
  fprintf(stderr, "leaving doWork within WSPRPass1\n");
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


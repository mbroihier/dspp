/*
 *      WSPRSymbols.cc - find the WSPR Symbols by candidate
 *
 *      Copyright (C) 2022
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
#include "WSPRSymbols.h"

/* ---------------------------------------------------------------------- */
void WSPRSymbols::init(int size, int number) {
  this->size = size;
  this->number = number;
  fprintf(stderr, "allocating bitArray memory\n");
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
}

WSPRSymbols::WSPRSymbols(int size, int number) {
  fprintf(stderr, "creating WSPRSymbols object\n");
  init(size, number);
  fprintf(stderr, "done creating WSPRSymbols object\n");
}

void WSPRSymbols::adjustThresholds(float centroid, int candidate) {
  bool adjust = false;
  int limit = 0;
  if (thresholds.end() == thresholds.find(candidate)) {
    thresholds[candidate] = new std::map<int, float>;
    (*thresholds[candidate])[FIRST] = centroid;
    (*thresholds[candidate])[SECOND] = centroid;
    (*thresholds[candidate])[THIRD] = centroid;
    (*thresholds[candidate])[REF1] = centroid;
    (*thresholds[candidate])[REF2] = centroid;
  } else {
    if (centroid < (*thresholds[candidate])[REF1]) {
      (*thresholds[candidate])[REF1] = centroid;
      adjust = true;
      limit = REF2;
    } else if (centroid > (*thresholds[candidate])[REF2]) {
      (*thresholds[candidate])[REF2] = centroid;
      adjust = true;
      limit = REF1;
    }
  }
  if (adjust) {
    if ((*thresholds[candidate])[REF2] - (*thresholds[candidate])[REF1] > BW) {
      if (limit == REF1) {
        (*thresholds[candidate])[REF1] = (*thresholds[candidate])[REF2] - BW;
      } else {
        (*thresholds[candidate])[REF2] = (*thresholds[candidate])[REF1] + BW;
      }
      //(*thresholds[candidate])[FIRST] = (*thresholds[candidate])[REF1] + BW1;
      //(*thresholds[candidate])[SECOND] = (*thresholds[candidate])[REF1] + BW2;
      //(*thresholds[candidate])[THIRD] = (*thresholds[candidate])[REF1] + BW3;
      (*thresholds[candidate])[FIRST] = (*thresholds[candidate])[REF1] + LOWER;
      (*thresholds[candidate])[SECOND] = (*thresholds[candidate])[REF1] + CF;
      (*thresholds[candidate])[THIRD] = (*thresholds[candidate])[REF1] + HIGHER;
      fprintf(stderr, "adjusting thresholds for candidate %d: %f, %f, %f - %f\n", candidate,
              (*thresholds[candidate])[FIRST], (*thresholds[candidate])[SECOND], (*thresholds[candidate])[THIRD],
              centroid);
    } else {
      fprintf(stderr, "not adjusting thresholds for candidate %d: %f, %f, %f - %f\n", candidate,
              (*thresholds[candidate])[FIRST], (*thresholds[candidate])[SECOND], (*thresholds[candidate])[THIRD],
              centroid);
    }
  } else {
      fprintf(stderr, "not adjusting thresholds for candidate %d: %f, %f, %f - %f\n", candidate,
              (*thresholds[candidate])[FIRST], (*thresholds[candidate])[SECOND], (*thresholds[candidate])[THIRD],
              centroid);
  }
}
void WSPRSymbols::adjustTargets(float centroid, int candidate) {
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
int WSPRSymbols::findClosestTarget(float centroid, int candidate) {
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

void WSPRSymbols::logCentroid(float centroid, int candidate) {
  if (centroidHistory.end() == centroidHistory.find(candidate)) {  // this is the first time logging this candidate
    centroidHistory[candidate] = new std::list<SampleRecord>;
  }
  SampleRecord sr;
  sr.centroid = centroid;
  sr.timeStamp = tic;
  centroidHistory[candidate]->push_back(sr);
  fprintf(stderr, "recording history for candidate %d\n", candidate);
}

void WSPRSymbols::logBase(float baseValue, int candidate) {
  if (centroidHistory.end() == centroidHistory.find(candidate)) {
    fprintf(stderr, "Internal error - attempting to log a base value prior to having a history of centroids\n");
    return;
  }
  if (baseHistory.end() == baseHistory.find(candidate)) {  // this is the first time logging this candidate
    baseHistory[candidate] = new std::list<BaseRecord>;
    BaseRecord br;
    br.base = baseValue;
    br.timeStamp = tic;
    for (int i = 0; i < centroidHistory[candidate]->size(); i++) {
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

void WSPRSymbols::reportHistory(int numberOfCandidates) {
  fprintf(stderr, "Number of candidates: %3d\n", numberOfCandidates);
  for (int i = 0; i < numberOfCandidates; i++) {
    if (baseHistory.end() == baseHistory.find(i) || centroidHistory.end() == centroidHistory.find(i)) {
      fprintf(stderr, "Candidate %d is not valid - it was a constant frequency: %f\n", i, candidates[i]);
    } else {
      fprintf(stderr, "History Report for Candidate: %d\n", i);
      int j = 0;
      if (centroidHistory[i]->size() < 162) {
        fprintf(stderr, "Candidate %d can not be valid - it does not have enough samples (%d), %f\n", i, centroidHistory[i]->size(), candidates[i]);
      } else {
        int lastTimeStamp = 0;
        int sequentialSamples = 1;
        bool enoughSequentialSamples = false;
        std::list<BaseRecord>::iterator iter2 = baseHistory[i]->begin();
        for (std::list<SampleRecord>::iterator iter1 = centroidHistory[i]->begin();
             iter1 != centroidHistory[i]->end(); iter1++, iter2++, j++) {
          if ((*iter1).timeStamp == lastTimeStamp + 1) {
            sequentialSamples++;
            fprintf(stderr, "Sample %3d: %f, %f, %d, %d, %d * %d\n", j, (*iter1).centroid, (*iter2).base,
                    (int) floor((*iter1).centroid - (*iter2).base +0.5), (*iter1).timeStamp,
                    (*iter2).timeStamp, sequentialSamples);
            if (sequentialSamples > 161) enoughSequentialSamples = true;
          } else {
            sequentialSamples = 1;
            fprintf(stderr, "Sample %3d: %f, %f, %d, %d, %d\n", j, (*iter1).centroid, (*iter2).base,
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
}


void WSPRSymbols::doWork() {
  std::map<int, int> candidateToGroup;  // mapping of candidate to group for this cycle
  int numberOfCandidates = 0;
  std::map<int, float> groupCentroids;  // mapped by group ID
  int frame = 0;
  int count = 0;
  float * samplePtr;
  float * magPtr;
  fprintf(stderr, "Find %d largest magnitude frequencies in FFT\n", number);
  bool done = false;
  int windowS = 0;
  int windowE = 0;
  while (!done) {
    // get an FFT's worth of bins
    fprintf(stderr, "done with set of data\n");
    count = fread(samples, sizeof(float), sampleBufferSize, stdin);
    fprintf(stderr, "done with read for tic %d\n", tic);
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
    fwrite(binArray, sizeof(int), number, stdout);
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
      float weightedCentroid = 0.0;
      float accumulator = 0.0;
      for (std::list<int>::iterator iter = groupToBins[groupIndex]->begin(); iter != groupToBins[groupIndex]->end(); iter++) {
        fprintf(stderr, " %d", *iter);
        weightedCentroid += *iter * mag[*iter];
        accumulator += mag[*iter];
      }
      weightedCentroid = weightedCentroid / accumulator;
      fprintf(stderr," --- weighted centroid: %f\n\n",weightedCentroid);
      groupCentroids[groupIndex] = weightedCentroid;  // indexed by groupIndex
      histogram[(int) weightedCentroid]++;
    }
    std::map<int, bool> alreadyUpdated;
    for (int canID = 0; canID < numberOfCandidates; canID++) {
      alreadyUpdated[canID] = false;
    }
    // map groups to candidates
    for (int groupIndex = 0; groupIndex < numberOfGroups; groupIndex++) {
      bool newCandidate = true;
      int currentCandidateID = 0;
      float canRangeLow = 0.0;
      float canRangeHigh = 0.0;
      for (int canID = 0; canID < numberOfCandidates; canID++) {
        //if (targets.end() != targets.find(canID)) {
        //  if ((*targets[canID])[TARGET0] == (*targets[canID])[TARGET3]) {
        //    canRangeLow = (*targets[canID])[TARGET0] - 4.0;
        //    canRangeHigh = (*targets[canID])[TARGET0] + 4.0;
        //  } else {
        //    canRangeLow = (*targets[canID])[TARGET0] - 0.6;
        //    canRangeHigh = (*targets[canID])[TARGET3] + 0.6;
        //  }
        //} else {
        //    canRangeLow = candidates[canID] - 6.0;
        //    canRangeHigh = candidates[canID] + 6.0;
        //}
        
        // look at existing candidates and, based on the histogram, determine a range that would be reasonable
        // for a group centroid to be this candidate
        canRangeLow = candidates[canID] - 6.0;
        canRangeHigh = candidates[canID] + 6.0;
        if (tic > 20) {
          if (canRangeLow < 6.0) canRangeLow = 6.0;
          if (canRangeHigh > 249.0) canRangeHigh = 249.0;
          int start = (int) canRangeLow;
          int stop = (int) canRangeHigh;
          int half = (start + stop) / 2;
          int atLeast = tic / 4;
          for (int lookat = start; lookat <= stop;  lookat++) {
            if (lookat < half) {
              if (histogram[lookat] < atLeast) {
                canRangeLow += 1.0;
              }
            } else {
              if (histogram[lookat] < atLeast) {
                canRangeHigh -= 1.0;
              }
            }
          }
        } else {
          if (canRangeLow < 0.0) canRangeLow = 0.0;
          if (canRangeHigh > 255.0) canRangeHigh = 255.0;
        }          
          
        fprintf(stderr, "Candidate %d has a range of %f to %f\n", canID, canRangeLow, canRangeHigh);
        if ((canRangeLow <=  groupCentroids[groupIndex]) && (canRangeHigh >= groupCentroids[groupIndex])) {
          newCandidate = false;  // this is too close to another candidate to add another candidate
          if (alreadyUpdated[canID]) {
            fprintf(stderr, "suppressing update of candidate %d with group %d info\n",
                    canID, groupIndex);
            //break;
          } else {
            candidateToGroup[canID] = groupIndex;
            currentCandidateID = canID;
            candidates[currentCandidateID] = groupCentroids[groupIndex];
            fprintf(stderr, "updating candidate centroid for candidate %d with %f from group %d\n",
                    currentCandidateID, groupCentroids[groupIndex], groupIndex);
            alreadyUpdated[canID] = true;
            break;
          }
        }
      }
      if (newCandidate) {
        fprintf(stderr, "making a new candidate, %d with centroid %f\n", numberOfCandidates, groupCentroids[groupIndex]);
        candidates[numberOfCandidates] = groupCentroids[groupIndex];
        candidateToGroup[numberOfCandidates] = groupIndex;
        alreadyUpdated[numberOfCandidates] = true;
        FILE * fh;
        char fileName[50];
        sprintf(fileName, "cand%d.txt", numberOfCandidates); 
        fh = fopen(fileName, "w");  // empty file if it exists
        fclose(fh);
        numberOfCandidates++;
        fprintf(stderr, "New candidate list\n");
        for (std::map<int, float>::iterator iter = candidates.begin(); iter != candidates.end(); iter++) {
          fprintf(stderr, "candidates[%d]: %f\n", (*iter).first, (*iter).second);
        }
      }
    }
    // within this FFT, output the centroids of candidates  
    for (int candidateIndex = 0; candidateIndex < numberOfCandidates; candidateIndex++) {
      FILE * fh;
      char fileName[50];
      sprintf(fileName, "cand%d.txt", candidateIndex);
      fh = fopen(fileName, "a");
      //if (candidateIndex == 0 && alreadyUpdated[candidateIndex]) {
      if (false) {
        char peaks[22];
        peaks[0] = 0;
        bool windowOK = false;
        for (int i = windowS; i < windowE; i++) {
          if (groupToBins[candidateToGroup[candidateIndex]]->end() !=
              std::find(groupToBins[candidateToGroup[candidateIndex]]->begin(),
                        groupToBins[candidateToGroup[candidateIndex]]->end(), i)) {
            windowOK = true;
            break;
          }
        }
        if (! windowOK) {
          windowS = (groupToBins[candidateToGroup[candidateIndex]]->front() +
                     groupToBins[candidateToGroup[candidateIndex]]->back())/2 - 10;
          windowE = windowS + 21;
          fprintf(fh, "------- window alignment change - candidate 0 found in group %d - windowS %d, windowE %d\n",
                  candidateToGroup[candidateIndex],
                  windowS, windowE);
          assert(windowS >=0 && windowE < size);
        }
        for (int i = windowS; i < windowE; i++) {
          if (i == windowS) {
            fprintf(fh, "%12.0f", mag[i]);
            strcat(peaks,"0");
          } else {
            fprintf(fh, ",%12.0f", mag[i]);
            if (i == windowE - 1) {
              strcat(peaks,"0");
            } else {
              if (mag[i - 1] < mag[i] && mag[i + 1] < mag[i] &&
                  groupToBins[candidateToGroup[candidateIndex]]->end() !=
                  std::find(groupToBins[candidateToGroup[candidateIndex]]->begin(),
                            groupToBins[candidateToGroup[candidateIndex]]->end(), i)) {
                strcat(peaks,"1");
              } else {
                strcat(peaks,"0");
              }
            }
          }
        }
        if (alreadyUpdated[candidateIndex]) {
          logCentroid(candidates[candidateIndex], candidateIndex);
          //adjustThresholds(candidates[candidateIndex], candidateIndex);
          adjustTargets(candidates[candidateIndex], candidateIndex);
          fprintf(fh, ", %s, %5.2f, %d\n", peaks, candidates[candidateIndex],
                  findClosestTarget(candidates[candidateIndex], candidateIndex));
        } else {
          fprintf(fh, ", %s, %5.2f, %d, not updated on this pass\n", peaks, candidates[candidateIndex],
                  findClosestTarget(candidates[candidateIndex], candidateIndex));
        }
      } else {
        if (alreadyUpdated[candidateIndex]) {
          logCentroid(candidates[candidateIndex], candidateIndex);
          //adjustThresholds(candidates[candidateIndex], candidateIndex);
          adjustTargets(candidates[candidateIndex], candidateIndex);
          fprintf(fh, "%5.2f, %d, %d\n", candidates[candidateIndex],
                  findClosestTarget(candidates[candidateIndex], candidateIndex), tic);
        } else {
          fprintf(fh, "%5.2f, %d, %d, not updated on this pass\n", candidates[candidateIndex],
                  findClosestTarget(candidates[candidateIndex], candidateIndex), tic);
        }
      }
      fclose(fh);
    }
    tic++;
  }
  reportHistory(numberOfCandidates);
  fprintf(stderr, "leaving doWork within WSPRSymbols\n");
}

WSPRSymbols::~WSPRSymbols(void) {
  fprintf(stderr, "destructing WSPRSymbols\n");
  for (std::map<int, std::map<int, float>*>::iterator iter = thresholds.begin(); iter != thresholds.end(); iter++) {
    delete ((*iter).second);
  }
  thresholds.clear();
  if (histogram) free(histogram);
  if (used) free(used);
  if (mag) free(mag);
  if (binArray) free(binArray);
  if (samples) free(samples);
}


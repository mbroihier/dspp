/*
 *      FindNLargestF.cc - find the N largest magnitude frequencies in a FFT
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
#include "FindNLargestF.h"

/* ---------------------------------------------------------------------- */
void FindNLargestF::init(int size, int number) {
  this->size = size;
  this->number = number;
  fprintf(stderr, "allocating bitArray memory\n");
  binArray = reinterpret_cast<int *>(malloc(number * sizeof(int)));
  fprintf(stderr, "allocating samples memory\n");
  samples = reinterpret_cast<float *>(malloc(size * sizeof(float) * 2));
  fprintf(stderr, "allocating mag memory\n");
  mag = reinterpret_cast<float *>(malloc(size * sizeof(float)));
  sampleBufferSize = size * 2;
  tic = 0;
}

FindNLargestF::FindNLargestF(int size, int number) {
  fprintf(stderr, "creating FindNLargestF object\n");
  init(size, number);
  fprintf(stderr, "done creating FindNLargestF object\n");
}

void FindNLargestF::adjustThresholds(float centroid, int candidate) {
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
void FindNLargestF::adjustTargets(float centroid, int candidate) {
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
int FindNLargestF::findClosestTarget(float centroid, int candidate) {
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

void FindNLargestF::logCentroid(float centroid, int candidate) {
  if (centroidHistory.end() == centroidHistory.find(candidate)) {  // this is the first time logging this candidate
    centroidHistory[candidate] = new std::list<SampleRecord>;
  }
  SampleRecord sr;
  sr.centroid = centroid;
  sr.timeStamp = tic;
  centroidHistory[candidate]->push_back(sr);
}

void FindNLargestF::logBase(float baseValue, int candidate) {
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
}

void FindNLargestF::reportHistory(int numberOfCandidates) {
  fprintf(stderr, "Number of candidates: %3d\n", numberOfCandidates);
  for (int i = 0; i < numberOfCandidates; i++) {
    fprintf(stderr, "History Report for Candidate: %d\n", i);
    int j = 0;
    std::list<BaseRecord>::iterator iter2 = baseHistory[i]->begin();
    for (std::list<SampleRecord>::iterator iter1 = centroidHistory[i]->begin();
         iter1 != centroidHistory[i]->end(); iter1++, iter2++, j++) {
      fprintf(stderr, "Sample %3d: %f, %f, %d, %d, %d\n", j, (*iter1).centroid, (*iter2).base,
              (int) floor((*iter1).centroid - (*iter2).base +0.5), (*iter1).timeStamp, (*iter2).timeStamp);
    }
  }
}


void FindNLargestF::doWork() {
  std::map<int, float> candidates;    // list of cadidates mapped to their centroid location
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
    fprintf(stderr, "done with read\n");
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
    int numberOfGroups = 1;
    binToGroup[binArray[0]] = groupNumber;
    groupToBins[groupNumber] = new std::list<int>;
    groupToBins[groupNumber]->push_back(binArray[0]);
    bool escape = false;
    for (int binIndex = 1; binIndex < number; binIndex++) {
      // does this bin belong in an existing group
      for (int groupIndex = 0; groupIndex < numberOfGroups; groupIndex++) {
        escape = false;
        for (std::list<int>::iterator iter = groupToBins[groupIndex]->begin(); iter != groupToBins[groupIndex]->end(); iter++) {
          if (abs(binArray[binIndex] - *iter) <= 2) { // this bin belongs to this group
            groupToBins[groupIndex]->push_back(binArray[binIndex]);
            binToGroup[binArray[binIndex]] = groupIndex;
            escape = true;
            break;  // done with this group, done with is bin
          }
        }
        if (escape) break;
      }
      if (! escape) {
        groupNumber++;
        numberOfGroups++;
        binToGroup[binArray[binIndex]] = groupNumber;
        groupToBins[groupNumber] = new std::list<int>;
        groupToBins[groupNumber]->push_back(binArray[binIndex]);
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
    }
    std::map<int, bool> alreadyUpdated;
    for (int canID = 0; canID < numberOfCandidates; canID++) {
      alreadyUpdated[canID] = false;
    }
    // map groups to candidates
    for (int groupIndex = 0; groupIndex < numberOfGroups; groupIndex++) {
      bool newCandidate = true;
      int currentCandidateID = 0;
      for (int canID = 0; canID < numberOfCandidates; canID++) {
        if (fabs(groupCentroids[groupIndex] - candidates[canID]) < 5.0) {
          if (alreadyUpdated[canID]) {
            fprintf(stderr, "suppressing update of candidate %d with group %d info\n",
                    canID, groupIndex);
            break;
          }
          newCandidate = false;
          candidateToGroup[canID] = groupIndex;
          currentCandidateID = canID;
          candidates[currentCandidateID] = groupCentroids[groupIndex];
          fprintf(stderr, "updating candidate centroid for candidate %d with %f from group %d\n",
                  currentCandidateID, groupCentroids[groupIndex], groupIndex);
          alreadyUpdated[canID] = true;
          break;
        }
      }
      if (newCandidate) {
        candidates[numberOfCandidates] = groupCentroids[groupIndex];
        candidateToGroup[numberOfCandidates] = groupIndex;
        alreadyUpdated[numberOfCandidates] = true;
        FILE * fh;
        char fileName[50];
        sprintf(fileName, "cand%d.txt", numberOfCandidates); 
        fh = fopen(fileName, "w");  // empty file if it exists
        fclose(fh);
        if (numberOfCandidates <= number) {
          numberOfCandidates++;
        }
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
      if (candidateIndex == 0 && alreadyUpdated[candidateIndex]) {
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
          assert(windowS >=0 && windowE < size);
          fprintf(fh, "------- window alignment change - candidate 0 found in group %d - windowS %d, windowE %d\n",
                  candidateToGroup[candidateIndex],
                  windowS, windowE);
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
          adjustThresholds(candidates[candidateIndex], candidateIndex);
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
          adjustThresholds(candidates[candidateIndex], candidateIndex);
          adjustTargets(candidates[candidateIndex], candidateIndex);
          fprintf(fh, "%5.2f, %d\n", candidates[candidateIndex],
                  findClosestTarget(candidates[candidateIndex], candidateIndex));
        } else {
          fprintf(fh, "%5.2f, %d, not updated on this pass\n", candidates[candidateIndex],
                  findClosestTarget(candidates[candidateIndex], candidateIndex));
        }
      }
      fclose(fh);
    }
    tic++;
  }
  reportHistory(numberOfCandidates);
  fprintf(stderr, "leaving doWork within FindNLargestF\n");
}

FindNLargestF::~FindNLargestF(void) {
  fprintf(stderr, "destructing FindNLargestF\n");
  for (std::map<int, std::map<int, float>*>::iterator iter = thresholds.begin(); iter != thresholds.end(); iter++) {
    delete ((*iter).second);
  }
  thresholds.clear();
  if (mag) free(mag);
  if (binArray) free(binArray);
  if (samples) free(samples);
}


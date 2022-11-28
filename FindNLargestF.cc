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
#include <list>
#include <map>
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
          fprintf(fh, ", %s, %5.2f\n", peaks, candidates[candidateIndex]);
        } else {
          fprintf(fh, ", %s, %5.2f, not updated in this pass\n", peaks, candidates[candidateIndex]);
        }
      } else {
        if (alreadyUpdated[candidateIndex]) {
          fprintf(fh, "%5.2f\n", candidates[candidateIndex]);
        } else {
          fprintf(fh, "%5.2f, not updated in this pass\n", candidates[candidateIndex]);
        }
      }
      fclose(fh);
    }
  }
  fprintf(stderr, "leaving doWork within FindNLargestF\n");
}

FindNLargestF::~FindNLargestF(void) {
  fprintf(stderr, "destructing FindNLargestF\n");
  if (mag) free(mag);
  if (binArray) free(binArray);
  if (samples) free(samples);
}


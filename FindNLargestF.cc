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
  int frame = 0;
  int count = 0;
  float * samplePtr;
  float * magPtr;
  fprintf(stderr, "Find %d largest magnitude frequencies in FFT\n", number);
  bool done = false;
  bool firstWrite = true;
  int windowS = 0;
  int windowE = 0;
  while (!done) {
    count = fread(samples, sizeof(float), sampleBufferSize, stdin);
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
    // make a sorted list of bin numbers that contain the highest frequency magnitudes
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
          if (abs(binArray[binIndex] - *iter) <= 1) { // this bin belongs to this group
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
    FILE * fh;
    if (firstWrite) {
      fh = fopen("grp0cent.txt", "w");
    } else {
      fh = fopen("grp0cent.txt", "a");
    }      
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
      if (groupIndex == 0) {
        if (firstWrite) {
          firstWrite = false;
          windowS = (groupToBins[groupIndex]->front() + groupToBins[groupIndex]->back())/2 - 10;
          windowE = windowS + 21;
          assert(windowS >=0 && windowE < size);
        }
        char peaks[22];
        peaks[0] = 0;
        bool windowOK = false;
        for (int i = windowS; i < windowE; i++) {
          if (groupToBins[0]->end() != std::find(groupToBins[0]->begin(), groupToBins[0]->end(), i)) {
            windowOK = true;
            break;
          }
        }
        if (! windowOK) {
          windowS = (groupToBins[groupIndex]->front() + groupToBins[groupIndex]->back())/2 - 10;
          windowE = windowS + 21;
          assert(windowS >=0 && windowE < size);
          fprintf(fh, "------- window alignment change\n");
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
              if (mag[i - 1] < mag[i] && mag[i + 1] < mag[i] && groupToBins[0]->end() != std::find(groupToBins[0]->begin(), groupToBins[0]->end(), i)) {
                strcat(peaks,"1");
              } else {
                strcat(peaks,"0");
              }
            }
          }
        }
        fprintf(fh, ", %s, %5.2f\n", peaks, weightedCentroid);
      }
      fprintf(stderr," --- weighted centroid: %f\n\n",weightedCentroid);
    }
    fclose(fh);
  }
}

FindNLargestF::~FindNLargestF(void) {
  if (mag) free(mag);
  if (binArray) free(binArray);
  if (samples) free(samples);
}


/*
 *      FT8SpotCandidate.cc - create an object that contains information regarding a potential spot
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <math.h>
#include <stdio.h>
#include "FT8SpotCandidate.h"
//#define SELFTEST
/* ---------------------------------------------------------------------- */
FT8SpotCandidate::FT8SpotCandidate(int ID, float deltaFreq, int size) {
  this->ID = ID;
  this->deltaFreq = deltaFreq;
  count = 0;
  longestSequence = 0;
  lastTimeStamp = -2;
  currentSequence = 0;
  fitInfo = 0;
  valid = false;
  slope = 0.0;
  yIntercept = 0.0;
  this->size = size;
}
/* ---------------------------------------------------------------------- */
FT8SpotCandidate::FT8SpotCandidate(int ID, const std::vector<SampleRecord> input, float deltaFreq, int size) {
  this->ID = ID;
  this->deltaFreq = deltaFreq;
  this->size = size;
  count = 0;
  longestSequence = 0;
  lastTimeStamp = -2;
  currentSequence = 0;
  fitInfo = 0;
  valid = false;
  slope = 0.0;
  yIntercept = 0.0;
  StartEnd se = { 0, 0 };
  for (auto sr : input) {
    candidateVector.push_back(sr);
    if (lastTimeStamp < 0) {
      se.start = sr.timeStamp;
    } else {
      se.end = sr.timeStamp;
    }
    currentSequence++;
    lastTimeStamp = sr.timeStamp;
  }
  sequenceDelimiters.push_back(se);
  longestSequence = currentSequence;
  count = currentSequence;
  if (longestSequence > 78) {
    valid = true;
    fitInfo = new Regression(getCentroidVector());
    slope = fitInfo->getSlope();
    yIntercept = fitInfo->getYIntercept();
    float bins = ID - WINDOW / 2.0;
    if (ID > size/2 - 1) {
      freq = (bins - size) * deltaFreq;
    } else {
      freq = bins * deltaFreq;
    }
    minCentroid = fitInfo->getMinCentroid();
    maxCentroid = fitInfo->getMaxCentroid();
  }
}
/* ---------------------------------------------------------------------- */
const std::vector<FT8SpotCandidate::SampleRecord> FT8SpotCandidate::getVector(void) {
  return candidateVector;
}
/* ---------------------------------------------------------------------- */
const std::vector<FT8SpotCandidate::SampleRecord> FT8SpotCandidate::getValidSubvector(int vectorNumber) {
  StartEnd se = { 0, 0 };
  int validCount = 0;
  aSubvector.clear();
  for (std::vector<StartEnd>::iterator iter = sequenceDelimiters.begin(); iter != sequenceDelimiters.end(); iter++) {
    se = *iter;
    if (se.end - se.start + 1 > 161) { // valid
      validCount++;
      if (validCount == vectorNumber) {  // get this list
        break;
      }
    }
    se = { 0, 0 };
  }
  if (se.start == se.end) return aSubvector;  // return an empty list if no list was found
  for (auto entry : candidateVector) {
    if (entry.timeStamp >= se.start && entry.timeStamp <= se.end) {
      aSubvector.push_back(entry);
    }
  }
  return aSubvector;
}
/* ---------------------------------------------------------------------- */
void FT8SpotCandidate::tokenize(int size, const std::vector<SampleRecord> validVector, std::vector<int> & tokens,
                                float & slope) {
  int metric = 0;
#ifdef SELFTEST
  // the vector below should result in a call sign of KG5YJE, a location of EM13, and a message of CQ
  int testInput[] = {3,1,4,0,6,5,2,0,0,0,0,0,0,0,0,1,1,1,2,2,7,4,1,5,3,2,0,5,0,4,7,3,3,0,0,0,
                     3,1,4,0,6,5,2,3,3,3,6,2,1,2,6,0,2,4,4,7,2,7,4,5,1,6,1,2,1,6,6,5,4,3,1,0,
                     3,1,4,0,6,5,2};
  slope = 0.0;
  tokens.clear();
  for (auto entry : testInput) {
    tokens.push_back(entry);
  }
  for (int metricIndex = 0; metricIndex < 7; metricIndex++) {
    if (tokens[metricIndex] == tokens[metricIndex + 29 + 7] &&
        tokens[metricIndex + 29 + 7 + 29 + 7] == tokens[metricIndex]) {
      metric++;
    }
  }
  return;
#endif
  tokens.clear();
  FT8SpotCandidate candidate(1000, validVector, 0.0, size);
  //candidate.printReport();
  std::vector<float> magnitudeAverages;
  for (int i = 0; i < WINDOW; i++) {
    float sum = 0.0;
    for (auto entry : validVector) {
      sum += entry.magSlice[i];
    }
    magnitudeAverages.push_back(sum / validVector.size());
  }
  std::vector<float> centroidVector = candidate.getCentroidVector();
  slope = candidate.getSlope();
  float base = 0.0;
  //base = candidate.getYIntercept() - 1.5;
  base = candidate.getYIntercept() - 3.5;


  int token = 0;
  float one = 0.0;
  float two = 0.0;
  float three = 0.0;
  float four = 0.0;
  float five = 0.0;
  float six = 0.0;
  float seven = 0.0;
  float zero = 0.0;
  for (size_t syncIndex = 0; syncIndex < centroidVector.size(); syncIndex++) {
    int sliceIndexZero = (int) (base - 0.5);
    int sliceIndexOne = sliceIndexZero + 1;
    int sliceIndexTwo = sliceIndexZero + 2;
    int sliceIndexThree = sliceIndexZero + 3;
    int sliceIndexFour = sliceIndexZero + 4;
    int sliceIndexFive = sliceIndexZero + 5;
    int sliceIndexSix = sliceIndexZero + 6;
    int sliceIndexSeven = sliceIndexZero + 7;
    if (sliceIndexZero < 0 || sliceIndexSeven >= WINDOW) {
      fprintf(stderr, "Can not tokenize this vector. Slope: %f, base: %f, first index: %d, last index: %d\n",
              slope, base, sliceIndexZero, sliceIndexSeven);
      tokens.clear(); // clear anything that may have been entered into the vector
      return;
    }
    zero = validVector[syncIndex].magSlice[sliceIndexZero] - magnitudeAverages[sliceIndexZero];
    one =  validVector[syncIndex].magSlice[sliceIndexOne] - magnitudeAverages[sliceIndexOne];
    two =  validVector[syncIndex].magSlice[sliceIndexTwo] - magnitudeAverages[sliceIndexTwo];
    three = validVector[syncIndex].magSlice[sliceIndexThree] - magnitudeAverages[sliceIndexThree];
    four = validVector[syncIndex].magSlice[sliceIndexFour] - magnitudeAverages[sliceIndexFour];
    five = validVector[syncIndex].magSlice[sliceIndexFive] - magnitudeAverages[sliceIndexFive];
    six = validVector[syncIndex].magSlice[sliceIndexSix] - magnitudeAverages[sliceIndexSix];
    seven = validVector[syncIndex].magSlice[sliceIndexSeven] - magnitudeAverages[sliceIndexSeven];
    if (zero > one && zero > two && zero > three && zero > four && zero > five && zero > six && zero > seven) {
      token = 0;
    } else {
      if (one > zero && one > two && one > three && one > four && one > five && one > six && one > seven) {
        token = 1;
      } else {
        if (two > zero && two > one && two > three && two > four && two > five && two > six && two > seven) {
          token = 2;
        } else {
          if (three > zero && three > one && three > two && three > four && three > five && three > six &&
              three > seven) {
            token = 3;
          } else {
            if (four > zero && four > one && four > two && four > three && four > five && four > six && four > seven) {
              token = 4;
            } else {
              if (five > zero && five > one && five > two && five > three && five > four && five > six &&
                  five > seven) {
                token = 5;
              } else {
                if (six > zero && six > one && six > two && six > three && six > four && six > five && six > seven) {
                  token = 6;
                } else {
                  token = 7;
                }
              }
            }
          }
        }
      }
    }
    fprintf(stderr, " syncIndex: %2ld, token: %3d\n", syncIndex, token);
    tokens.push_back(token);
    base += slope;
  }
  for (int metricIndex = 0; metricIndex < 7; metricIndex++) {
    if (tokens[metricIndex] == tokens[metricIndex + 29 + 7] &&
        tokens[metricIndex + 29 + 7 + 29 + 7] == tokens[metricIndex]) {
      metric++;
    }
  }
  fprintf(stderr, "Tokens metric: %d\n", metric);
}
/* ---------------------------------------------------------------------- */
std::vector<float> FT8SpotCandidate::getCentroidVector(void) {
  centroids.clear();
  for (auto entry : candidateVector) {
    centroids.push_back(entry.centroid);
  }
  return centroids;
}
/* ---------------------------------------------------------------------- */
std::vector<float> FT8SpotCandidate::getMagnitudeVector(void) {
  magnitudes.clear();
  for (auto entry : candidateVector) {
    magnitudes.push_back(entry.magnitude);
  }
  return magnitudes;
}
/* ---------------------------------------------------------------------- */
void FT8SpotCandidate::printReport(void) {
  fprintf(stderr, "Potential Candidate %d Report - samples: %5d, longest sequence: %5d, status: %s, slope: %7.4f, y-intercept: %7.2f, uncompensated center frequency of spot: %8.5f\n",
          ID, count, longestSequence, valid?"  valid":"invalid", slope, yIntercept, freq);
  int i = 0;
  if (candidateVector.size() < 1) {
    fprintf(stderr, "No information on candidate\n");
    return;
  }
  lastTimeStamp = -1;
  for (auto entry : candidateVector) {
    fprintf(stderr, "%3d: centroid: %7.2f, magnitude: %10.0f, time stamp: %5d, time in seconds: %7.2f %s\n",
            i++, entry.centroid, entry.magnitude, entry.timeStamp, entry.timeSeconds,
            ((entry.timeStamp - lastTimeStamp) == 1)?"*":" ");
    lastTimeStamp = entry.timeStamp;
  }
  for (auto entry : sequenceDelimiters) {
    if (entry.start != entry.end) fprintf(stderr, "sequence start %d, sequence end %d\n", entry.start, entry.end);
  }
  fprintf(stderr, "Magnitude slice\n");
  int line = 0;
  float acc = 0.0;
  for (auto entry : candidateVector) {
    for (int i = 0; i < WINDOW; i++) {
      acc += entry.magSlice[i];
      fprintf(stderr, "%9.0f,", entry.magSlice[i]);
    }
    fprintf(stderr, " %d\n", line++);
  }
  line = 0;
  float average = acc / (candidateVector.size() * WINDOW);
  fprintf(stderr, "Magnitude graphic - ID: %d\n", ID);
  for (auto entry : candidateVector) {
    if (entry.magSlice.size() == 0) break;
    char graphic[WINDOW + 1];
    if (entry.magSlice[0] > entry.magSlice[1] && entry.magSlice[0] > average) {
      graphic[0] = '*';
    } else {
      graphic[0] = '_';
    }
    for (int i = 1; i < WINDOW - 1 ; i++) {
      if (entry.magSlice[i - 1] < entry.magSlice[i] && entry.magSlice[i] > entry.magSlice[i + 1]
          && entry.magSlice[i] > average) {
        graphic[i] = '*';
      } else {
        graphic[i] = '_';
      }
    }
    if (entry.magSlice[WINDOW - 1] > entry.magSlice[WINDOW - 2] && entry.magSlice[WINDOW - 1] > average) {
      graphic[WINDOW - 1] = '*';
    } else {
      graphic[WINDOW - 1] ='_';
    }
    graphic[WINDOW] = '\0';
    fprintf(stderr, " %s %d\n", graphic, line++);
  }
}
/* ---------------------------------------------------------------------- */
FT8SpotCandidate::~FT8SpotCandidate(void) {
  candidateVector.clear();
  if (fitInfo) delete(fitInfo);
}

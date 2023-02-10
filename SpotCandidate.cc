/*
 *      SpotCandidate.cc - create an object that contains information regarding a potential spot
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <stdio.h>
#include "SpotCandidate.h"

/* ---------------------------------------------------------------------- */
SpotCandidate::SpotCandidate(int ID) {
  this->ID = ID;
  count = 0;
  longestSequence = 0;
  lastTimeStamp = -2;
  currentSequence = 0;
  fitInfo = 0;
  valid = false;
  slope = 0.0;
  yIntercept = 0.0;
}
/* ---------------------------------------------------------------------- */
SpotCandidate::SpotCandidate(int ID, const std::vector<SampleRecord> input) {
  this->ID = ID;
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
  if (longestSequence > 161) {
    valid = true;
    fitInfo = new Regression(getCentroidVector());
    slope = fitInfo->getSlope();
    yIntercept = fitInfo->getYIntercept();
    minCentroid = fitInfo->getMinCentroid();
    maxCentroid = fitInfo->getMaxCentroid();
  }
}
/* ---------------------------------------------------------------------- */
bool SpotCandidate::logSample(float centroid, float magnitude, int timeStamp, float timeSeconds) {
  if (timeStamp == lastTimeStamp) {
    return false;  // return that a candidate was already logged
  } else {
    if ((lastTimeStamp + 1) == timeStamp) {
      currentSequence++;
      std::vector<StartEnd>::iterator current = sequenceDelimiters.end();
      current--;
      StartEnd se = *current;
      se.end = timeStamp;
      *current = se;
    } else {
      StartEnd se;
      se.start = timeStamp;
      se.end = timeStamp;
      sequenceDelimiters.push_back(se);
      currentSequence = 1;
    }
    if (currentSequence > longestSequence) {
      longestSequence = currentSequence;
    }
    SampleRecord sr;
    sr.centroid = centroid;
    sr.magnitude = magnitude;
    sr.timeStamp = timeStamp;
    sr.timeSeconds = timeSeconds;
    candidateVector.push_back(sr);
    count++;
    lastTimeStamp = timeStamp;
    if (longestSequence > 161) {
      if (fitInfo) delete(fitInfo);
      fitInfo = new Regression(getCentroidVector());
      slope = fitInfo->getSlope();
      yIntercept = fitInfo->getYIntercept();
      valid = true;
    }
  }
  return true;
}
/* ---------------------------------------------------------------------- */
bool SpotCandidate::mergeVector(const std::vector<SampleRecord> other) {
  std::vector<SampleRecord> newVector;
  if (other.size() == 0) return false;
  if (candidateVector.size() == 0) return false;
  int workingTime = 0;
  int lastWorkingTime = -2;
  int longestSequence = 0;
  int currentSequence = 1;
  int count = 0;
  bool done = false;
  int merged = 0;
  std::vector<SampleRecord>::iterator iter1 = candidateVector.begin();
  std::vector<SampleRecord>::const_iterator iter2 = other.begin();
  sequenceDelimiters.clear();
  if (iter1 == candidateVector.end()) {
    fprintf(stderr, "Can't merge into empty vector\n");
    return false;
  }
  if (iter2 == other.end()) {
    fprintf(stderr, "Merging an empty vector does nothing\n");
    return false;
  }
  SampleRecord one = *iter1;
  SampleRecord two = *iter2;
  bool finishUsingTargetVector = false;
  bool finishUsingOtherVector = false;
  while (! done) {
    if (iter1 != candidateVector.end()) {  // don't go past last entry
      one = *iter1;
    } else {
      finishUsingOtherVector = true;
      fprintf(stderr, "The end of the target list has been reached\n");
    }
    if (iter2 != other.end()) {  // don't go past last entry
      two = *iter2;
    } else {
      fprintf(stderr, "The end of the list being merged has been reached\n");
      finishUsingTargetVector = true;
    }
    if (finishUsingTargetVector || finishUsingOtherVector) {
      if (finishUsingTargetVector) {
        workingTime = one.timeStamp;
        newVector.push_back(one);
        fprintf(stderr, "recorded an element from the target list and advancing\n");
        count++;
        iter1++;
      } else {
        workingTime = two.timeStamp;
        newVector.push_back(two);
        fprintf(stderr, "recorded an element from the list to merge in and advancing\n");
        merged++;
        count++;
        iter2++;
      }
    } else {
      workingTime = one.timeStamp;
      if (workingTime > two.timeStamp) {
        workingTime = two.timeStamp;
      }
      if (workingTime == one.timeStamp) {
        newVector.push_back(one);
        fprintf(stderr, "recorded an element from the target list and advancing\n");
        count++;
        if (iter1 != candidateVector.end()) iter1++;
        if (workingTime >= two.timeStamp) {  // we skip this record of the other vector
          while (iter2 != other.end() && workingTime >= (*iter2).timeStamp) {
            fprintf(stderr, "Incrementing other pointer\n");
            iter2++;
          }
        }
      } else {
        fprintf(stderr, "Stitching at working time %d, two.timeStamp %d\n", workingTime, two.timeStamp);
        if (workingTime == two.timeStamp) {
          newVector.push_back(two);
          fprintf(stderr, "recorded an element from the list to merge in and advancing\n");
          merged++;
          count++;
          if (iter2 != other.end()) iter2++;
          if (workingTime >= one.timeStamp) {  // we skip this record of candidate vector
            while (iter1 != candidateVector.end() && workingTime >= (*iter1).timeStamp) {
              fprintf(stderr, "Incrementing target pointer\n");
              iter1++;
            }
          }
        }
      }
    }
    if ((sequenceDelimiters.size() > 0) && (workingTime == lastWorkingTime + 1)) {
      std::vector<StartEnd>::iterator current = sequenceDelimiters.end();
      current--;
      StartEnd se = *current;
      se.end = workingTime;
      *current = se;
      currentSequence++;
      fprintf(stderr, "currentSequence %d, start %d, end %d, diff %d\n", currentSequence, se.start, se.end,
              se.end - se.start + 1);
    } else {
      currentSequence = 1;
      StartEnd se = {workingTime, workingTime};
      sequenceDelimiters.push_back(se);
    }          
    if (currentSequence > longestSequence) {
      longestSequence = currentSequence;
    }
    lastWorkingTime = workingTime;
    done = (iter1 == candidateVector.end()) && (iter2 == other.end());
  }
  this->longestSequence = longestSequence;
  this->count = count;
  candidateVector.clear();
  for (auto entry : newVector) {
    candidateVector.push_back(entry);
  }
  if (longestSequence > 161) {
    if (fitInfo) delete(fitInfo);
    fitInfo = new Regression(getCentroidVector());
    slope = fitInfo->getSlope();
    yIntercept = fitInfo->getYIntercept();
    valid = true;
  } else {
    valid = false;
  }
  fprintf(stderr, "Merged %d samples from other list\n", merged);
  return true;
}
/* ---------------------------------------------------------------------- */
const std::vector<SpotCandidate::SampleRecord> SpotCandidate::getVector(void) {
  return candidateVector;
}
/* ---------------------------------------------------------------------- */
const std::vector<SpotCandidate::SampleRecord> SpotCandidate::getValidSubvector(int vectorNumber) {
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
void SpotCandidate::tokenize(const std::vector<SampleRecord> validVector, std::vector<int> & tokens) {
  tokens.clear();
  SpotCandidate candidate(1000, validVector);
  std::vector<float> centroidVector = candidate.getCentroidVector();
  float slope = candidate.getSlope();
  float yIntercept = candidate.getYIntercept();
  float minCentroid = candidate.getMinCentroid();
  float maxCentroid = candidate.getMaxCentroid();
  float x = 0.0;
  float minExpected = 0.0;
  float maxExpected = 0.0;
  if (slope < 0.0) {  // The maximum value of the signal should occur near zero, the minimum value near count
    minExpected = yIntercept + slope * validVector.size() - 1.5;
    maxExpected = yIntercept + 1.5;
  } else {
    minExpected = yIntercept - 1.5;
    maxExpected = yIntercept + slope * validVector.size() + 1.5;
  }
  float scale = 256.0/(maxCentroid - minCentroid);
  fprintf(stderr, "expected bounds are: %7.2f and %7.2f\n", minExpected, maxExpected);
  for (auto entry : centroidVector) {
    float expectedY = yIntercept + slope * x;
    float base = expectedY - 1.5;
    //int token = std::min(std::max((int)(entry - base + 0.5),0),3);
    int token = std::min(std::max((int)((entry - minCentroid) * scale),0),255);
    tokens.push_back(token);
    //fprintf(stderr, "sample %3d - expected: %7.2f, actual: %7.2f, error: %7.2f, token: %d\n",
    //        (int) x, expectedY, entry, expectedY - entry, token);
    fprintf(stderr, "sample %3d - minCentroid: %7.2f, maxCentroid: %7.2f, actual: %7.2f, error: %7.2f, token: %d\n",
            (int) x, minCentroid, maxCentroid, entry, entry - minCentroid, token);
    x += 1.0;
  }
}
/* ---------------------------------------------------------------------- */
std::vector<float> SpotCandidate::getCentroidVector(void) {
  centroids.clear();
  for (auto entry : candidateVector) {
    centroids.push_back(entry.centroid);
  }
  return centroids;
}
/* ---------------------------------------------------------------------- */
std::vector<float> SpotCandidate::getMagnitudeVector(void) {
  magnitudes.clear();
  for (auto entry : candidateVector) {
    magnitudes.push_back(entry.magnitude);
  }
  return magnitudes;
}
/* ---------------------------------------------------------------------- */
void SpotCandidate::printReport(void) {
  fprintf(stderr, "Potential Candidate %d Report - samples: %5d, longest sequence: %5d, status: %s, slope: %7.4f, y-intercept: %7.2f\n",
          ID, count, longestSequence, valid?"  valid":"invalid", slope, yIntercept);
  int i = 0;
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
}
/* ---------------------------------------------------------------------- */
SpotCandidate::~SpotCandidate(void) {
  candidateVector.clear();
  if (fitInfo) delete(fitInfo);
}

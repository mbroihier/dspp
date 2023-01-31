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
SpotCandidate::SpotCandidate(int ID, const std::list<SampleRecord> input) {
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
  for (std::list<SampleRecord>::const_iterator iter = input.begin(); iter != input.end(); iter++) {
    SampleRecord sr = *iter;
    candidateList.push_back(sr);
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
    fitInfo = new Regression(getCentroidList());
    slope = fitInfo->getSlope();
    yIntercept = fitInfo->getYIntercept();
  }
}
/* ---------------------------------------------------------------------- */
bool SpotCandidate::logSample(float centroid, float magnitude, int timeStamp, float timeSeconds) {
  if (timeStamp == lastTimeStamp) {
    return false;  // return that a candidate was already logged
  } else {
    if ((lastTimeStamp + 1) == timeStamp) {
      currentSequence++;
      std::list<StartEnd>::iterator current = sequenceDelimiters.end();
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
    candidateList.push_back(sr);
    count++;
    lastTimeStamp = timeStamp;
    if (longestSequence > 161) {
      if (fitInfo) delete(fitInfo);
      fitInfo = new Regression(getCentroidList());
      slope = fitInfo->getSlope();
      yIntercept = fitInfo->getYIntercept();
      valid = true;
    }
  }
  return true;
}
/* ---------------------------------------------------------------------- */
bool SpotCandidate::mergeList(const std::list<SampleRecord> other) {
  std::list<SampleRecord> newList;
  if (other.size() == 0) return false;
  if (candidateList.size() == 0) return false;
  int workingTime = 0;
  int lastWorkingTime = -2;
  int longestSequence = 0;
  int currentSequence = 1;
  int count = 0;
  bool done = false;
  int merged = 0;
  std::list<SampleRecord>::iterator iter1 = candidateList.begin();
  std::list<SampleRecord>::const_iterator iter2 = other.begin();
  sequenceDelimiters.clear();
  while (! done) {
    SampleRecord one = *iter1;
    SampleRecord two = *iter2;
    workingTime = one.timeStamp;
    if (workingTime > two.timeStamp) {
      workingTime = two.timeStamp;
    }
    if (workingTime == one.timeStamp) {
      newList.push_back(one);
      count++;
      iter1++;
      if (workingTime >= two.timeStamp) {  // we skip this record of the other list
        while (iter2 != other.end() && workingTime >= (*iter2).timeStamp) {
          iter2++;
        }
      }
    } else {
      fprintf(stderr, "Stitching at working time %d, two.timeStamp %d\n", workingTime, two.timeStamp);
      if (workingTime == two.timeStamp) {
        newList.push_back(two);
        merged++;
        count++;
        iter2++;
        if (workingTime >= one.timeStamp) {  // we skip this record of candidate list
          while (iter1 != candidateList.end() && workingTime >= (*iter1).timeStamp) {
            iter1++;
          }
        }
      }
    }
    if ((sequenceDelimiters.size() > 0) && (workingTime == lastWorkingTime + 1)) {
      std::list<StartEnd>::iterator current = sequenceDelimiters.end();
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
    done = (iter1 == candidateList.end()) && (iter2 == other.end());
  }
  this->longestSequence = longestSequence;
  this->count = count;
  candidateList.clear();
  for (std::list<SampleRecord>::iterator iter = newList.begin(); iter != newList.end(); iter++) {
    candidateList.push_back(*iter);
  }
  if (longestSequence > 161) {
    if (fitInfo) delete(fitInfo);
    fitInfo = new Regression(getCentroidList());
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
const std::list<SpotCandidate::SampleRecord> SpotCandidate::getList(void) {
  return candidateList;
}
/* ---------------------------------------------------------------------- */
const std::list<SpotCandidate::SampleRecord> SpotCandidate::getValidSublist(int listNumber) {
  std::list<SampleRecord> aSublist;
  StartEnd se = { 0, 0 };
  SampleRecord sr;
  int validCount = 0;
  for (std::list<StartEnd>::iterator iter = sequenceDelimiters.begin(); iter != sequenceDelimiters.end(); iter++) {
    se = *iter;
    if (se.end - se.start + 1 > 161) { // valid
      validCount++;
      if (validCount == listNumber) {  // get this list
        break;
      }
    }
    se = { 0, 0 };
  }
  if (se.start == se.end) return aSublist;  // return an empty list if no list was found
  for (std::list<SampleRecord>::iterator iter = candidateList.begin(); iter != candidateList.end(); iter++) {
    sr = *iter;
    if (sr.timeStamp >= se.start && sr.timeStamp <= se.end) {
      aSublist.push_back(sr);
    }
  }
  return aSublist;
}
/* ---------------------------------------------------------------------- */
const std::list<int> SpotCandidate::tokenize(const std::list<SampleRecord> validList) {
  SpotCandidate candidate(1000, validList);
  std::list<float> centroidList = candidate.getCentroidList();
  float slope = candidate.getSlope();
  float yIntercept = candidate.getYIntercept();
  std::list<int> tokens;
  float x = 0.0;
  float minExpected = 0.0;
  float maxExpected = 0.0;
  if (slope < 0.0) {  // The maximum value of the signal should occur near zero, the minimum value near count
    minExpected = yIntercept + slope * validList.size() - 1.5;
    maxExpected = yIntercept + 1.5;
  } else {
    minExpected = yIntercept - 1.5;
    maxExpected = yIntercept + slope * validList.size() + 1.5;
  }
  fprintf(stderr, "expected bounds are: %7.2f and %7.2f\n", minExpected, maxExpected);
  for (std::list<float>::iterator iter = centroidList.begin(); iter != centroidList.end(); iter++) {
    float expectedY = yIntercept + slope * x;
    float base = expectedY - 1.5;
    int token = std::min(std::max((int)(*iter - base + 0.5),0),3);
    tokens.push_back(token);
    fprintf(stderr, "sample %3d - expected: %7.2f, actual: %7.2f, error: %7.2f, token: %d\n",
            (int) x, expectedY, *iter, expectedY - *iter, token);
    x += 1.0;
  }
  return tokens;
}
/* ---------------------------------------------------------------------- */
std::list<float> SpotCandidate::getCentroidList(void) {
  std::list<float> centroids;
  for (std::list<SampleRecord>::iterator iter = candidateList.begin(); iter != candidateList.end(); iter++) {
    centroids.push_back((*iter).centroid);
  }
  return centroids;
}
/* ---------------------------------------------------------------------- */
std::list<float> SpotCandidate::getMagnitudeList(void) {
  std::list<float> magnitudes;
  for (std::list<SampleRecord>::iterator iter = candidateList.begin(); iter != candidateList.end(); iter++) {
    magnitudes.push_back((*iter).magnitude);
  }
  return magnitudes;
}
/* ---------------------------------------------------------------------- */
void SpotCandidate::printReport(void) {
  fprintf(stderr, "Potential Candidate %d Report - samples: %5d, longest sequence: %5d, status: %s, slope: %7.4f, y-intercept: %7.2f\n",
          ID, count, longestSequence, valid?"  valid":"invalid", slope, yIntercept);
  int i = 0;
  lastTimeStamp = 0;
  for (std::list<SampleRecord>::iterator iter = candidateList.begin(); iter != candidateList.end(); iter++) {
    fprintf(stderr, "%3d: centroid: %7.2f, magnitude: %10.0f, time stamp: %5d, time in seconds: %7.2f %s\n",
            i++, (*iter).centroid, (*iter).magnitude, (*iter).timeStamp, (*iter).timeSeconds,
            (((*iter).timeStamp - lastTimeStamp) == 1)?"*":" ");
    lastTimeStamp = (*iter).timeStamp;
  }
  for (std::list<StartEnd>::iterator iter = sequenceDelimiters.begin(); iter != sequenceDelimiters.end(); iter++) {
    if ((*iter).start != (*iter).end) fprintf(stderr, "sequence start %d, sequence end %d\n", (*iter).start, (*iter).end);
  }
}
/* ---------------------------------------------------------------------- */
SpotCandidate::~SpotCandidate(void) {
  candidateList.clear();
  if (fitInfo) delete(fitInfo);
}

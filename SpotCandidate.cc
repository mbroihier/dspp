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
  lastTimeStamp = -1;
  fitInfo = 0;
  valid = false;
  slope = 0.0;
  yIntercept = 0.0;
}
/* ---------------------------------------------------------------------- */
bool SpotCandidate::logSample(float centroid, float magnitude, int timeStamp, float timeSeconds) {
  if (timeStamp == lastTimeStamp) {
    return false;  // return that a candidate was already logged
  } else {
    SampleRecord sr;
    sr.centroid = centroid;
    sr.magnitude = magnitude;
    sr.timeStamp = timeStamp;
    sr.timeSeconds = timeSeconds;
    candidateList.push_back(sr);
    count++;
    lastTimeStamp = timeStamp;
    if (count > 161) {
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
  fprintf(stderr, "Potential Candidate %d Report - samples: %5d, status: %s, slope: %7.2f, y-intercept: %7.2f\n",
          ID, count, valid?"  valid":"invalid", slope, yIntercept);
  int i = 0;
  lastTimeStamp = 0;
  for (std::list<SampleRecord>::iterator iter = candidateList.begin(); iter != candidateList.end(); iter++) {
    fprintf(stderr, "%3d: centroid: %7.2f, magnitude: %10.0f, time stamp: %5d, time in seconds: %7.2f %s\n",
            i++, (*iter).centroid, (*iter).magnitude, (*iter).timeStamp, (*iter).timeSeconds,
            (((*iter).timeStamp - lastTimeStamp) == 1)?"*":" ");
    lastTimeStamp = (*iter).timeStamp;
  }
}
/* ---------------------------------------------------------------------- */
SpotCandidate::~SpotCandidate(void) {
  candidateList.clear();
  if (fitInfo) delete(fitInfo);
}



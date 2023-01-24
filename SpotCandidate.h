#ifndef SPOTCANDIDATE_H_
#define SPOTCANDIDATE_H_
/*
 *      SpotCandidate.h - Class for creating a object that may be a WSPR spot
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <list>
#include "Regression.h"
/* ---------------------------------------------------------------------- */
class SpotCandidate {
 private:
  int ID;
  int count;
  int lastTimeStamp;
  bool valid;
  float slope;
  float yIntercept;
  Regression * fitInfo;
  struct SampleRecord { float centroid; float magnitude; int timeStamp; float timeSeconds; };
  std::list<SampleRecord> candidateList;
 public:
  bool logSample(float centroid, float magnitude, int timeStamp, float timeSeconds);
  std::list<float> getCentroidList(void);
  std::list<float> getMagnitudeList(void);
  bool isValid(void);
  void printReport(void);
  SpotCandidate(int ID);
  ~SpotCandidate(void);
};
#endif  // SPOTCANDIDATE_H_

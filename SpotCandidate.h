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
 public:
  struct StartEnd { int start; int end; };
  struct SampleRecord { float centroid; float magnitude; int timeStamp; float timeSeconds; };
 private:
  int ID;
  int count;
  int lastTimeStamp;
  bool valid;
  float slope;
  float yIntercept;
  int longestSequence;
  int currentSequence;
  Regression * fitInfo;
  std::list<SampleRecord> candidateList;
  std::list<StartEnd> sequenceDelimiters;
 public:
  bool logSample(float centroid, float magnitude, int timeStamp, float timeSeconds);
  std::list<float> getCentroidList(void);
  std::list<float> getMagnitudeList(void);
  int getCount(void) { return count; }; 
  const std::list<SampleRecord> getList(void);
  const std::list<SampleRecord> getValidSublist(int listNumber);
  bool isValid(void);
  void printReport(void);
  bool  mergeList(const std::list<SampleRecord> other);
  static const std::list<int> tokenize(const std::list<SampleRecord> validList);
  float getSlope() { return slope; };
  float getYIntercept() { return yIntercept; };
  SpotCandidate(int ID);
  SpotCandidate(int ID, const std::list<SampleRecord> input);
  ~SpotCandidate(void);
};
#endif  // SPOTCANDIDATE_H_

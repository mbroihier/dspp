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
#include <vector>
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
  float minCentroid;
  float maxCentroid;
  int longestSequence;
  int currentSequence;
  Regression * fitInfo;
  std::vector<float> magnitudes;
  std::vector<SampleRecord> aSubvector;
  std::vector<float> centroids;
  std::vector<SampleRecord> candidateVector;
  std::vector<StartEnd> sequenceDelimiters;
 public:
  bool logSample(float centroid, float magnitude, int timeStamp, float timeSeconds);
  std::vector<float> getCentroidVector(void);
  std::vector<float> getMagnitudeVector(void);
  int getCount(void) { return count; }; 
  const std::vector<SampleRecord> getVector(void);
  const std::vector<SampleRecord> getValidSubvector(int vectorNumber);
  bool isValid(void);
  void printReport(void);
  bool  mergeVector(const std::vector<SampleRecord> other);
  void tokenize(const std::vector<SampleRecord> validVector, std::vector<int> & tokens);
  float getSlope() { return slope; };
  float getYIntercept() { return yIntercept; };
  float getMinCentroid() { return minCentroid; };
  float getMaxCentroid() { return maxCentroid; };
  SpotCandidate(int ID);
  SpotCandidate(int ID, const std::vector<SampleRecord> input);
  ~SpotCandidate(void);
};
#endif  // SPOTCANDIDATE_H_

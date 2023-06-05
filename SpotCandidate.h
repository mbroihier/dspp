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
#include <stdlib.h>
#include <vector>
#include "Regression.h"
/* ---------------------------------------------------------------------- */
class SpotCandidate {
 public:
  struct StartEnd { int start; int end; };
  struct SampleRecord { float centroid; float magnitude; std::vector<float> r; std::vector<float> i;
    std::vector<float> magSlice; int timeStamp; float timeSeconds; };
  static const int WINDOW = 7;
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
  float freq;
  float deltaFreq;
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
  bool isValid(void) { return valid ; };
  void printReport(void);
  bool  mergeVector(const std::vector<SampleRecord> other);
  static void tokenize(const std::vector<SampleRecord> validVector, std::vector<int> & tokens, float & snr);
  float getSlope() { return slope; };
  float getYIntercept() { return yIntercept; };
  float getMinCentroid() { return minCentroid; };
  float getMaxCentroid() { return maxCentroid; };
  float getFrequency() { return freq; }
  SpotCandidate(int ID, float deltaFreq);
  SpotCandidate(int ID, const std::vector<SampleRecord> input, float deltaFreq);
  ~SpotCandidate(void);
};
#endif  // SPOTCANDIDATE_H_

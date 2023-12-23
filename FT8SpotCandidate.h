#ifndef FT8SPOTCANDIDATE_H_
#define FT8SPOTCANDIDATE_H_
/*
 *      FT8SpotCandidate.h - Class for creating a object that may be a WSPR spot
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
class FT8SpotCandidate {
 public:
  struct StartEnd { int start; int end; };
  struct SampleRecord { float centroid; float magnitude; std::vector<float> r; std::vector<float> i;
    std::vector<float> magSlice; int timeStamp; float timeSeconds; };
  static const int WINDOW = 11;
  static const int HALF_WINDOW = 5;
 private:
  int ID;
  int count;
  int size;
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
  std::vector<float> getCentroidVector(void);
  std::vector<float> getMagnitudeVector(void);
  int getCount(void) { return count; }; 
  const std::vector<SampleRecord> getVector(void);
  const std::vector<SampleRecord> getValidSubvector(int vectorNumber);
  bool isValid(void) { return valid ; };
  void printReport(void);
  static void tokenize(int size, const std::vector<SampleRecord> validVector, std::vector<int> & tokens,
                       float & slope);
  float getSlope() { return slope; };
  float getYIntercept() { return yIntercept; };
  float getMinCentroid() { return minCentroid; };
  float getMaxCentroid() { return maxCentroid; };
  float getFrequency() { return freq; }
  FT8SpotCandidate(int ID, float deltaFreq, int size);
  FT8SpotCandidate(int ID, const std::vector<SampleRecord> input, float deltaFreq, int size);
  ~FT8SpotCandidate(void);
};
#endif  // FT8SPOTCANDIDATE_H_

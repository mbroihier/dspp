#ifndef FINDNLARGESTF_H_
#define FINDNLARGESTF_H_
/*
 *      FindNLargestF.h - Find the N largest magnitude frequencies in an FFT
 *
 *      Copyright (C) 2022 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <list>
#include <map>
#include <math.h>
#include <sys/types.h>
/* ---------------------------------------------------------------------- */
class FindNLargestF {
 private:
  void init(int size, int number);
  void adjustThresholds(float centroid, int candidate);
  void adjustTargets(float centroid, int candidate);
  int findClosestTarget (float centroid, int candidate);
  void logCentroid(float centroid, int candidate);
  void logBase(float baseValue, int candidate);
  void reportHistory(int numberOfCandidates);
  int * binArray;
  float * samples;
  float * mag;
  int sampleBufferSize;
  int size;
  int number;
  int tic;

  const int FIRST = 0;
  const int SECOND = 1;
  const int THIRD = 2;
  const int REF1 = 4;
  const int REF2 = 5;

  const float BW = 3.0;
  const float BW_DELTA = BW / 3.0;
  const float BW1 = BW / 6.0;
  const float BW2 = BW1 + BW_DELTA;
  const float BW3 = BW2 + BW_DELTA;

  const float CF = BW / 2.0;
  const float LOWER = CF - BW1;
  const float HIGHER = CF + BW1;

  const int TARGET0 = 0;
  const int TARGET1 = 1;
  const int TARGET2 = 2;
  const int TARGET3 = 3;
  
  std::map<int, std::map<int, float> *>  thresholds;
  std::map<int, std::map<int, float> *>  targets;

  struct SampleRecord { float centroid; int timeStamp; };
  struct BaseRecord { float base; int timeStamp; };
  std::map<int, std::list<SampleRecord> *> centroidHistory;
  std::map<int, std::list<BaseRecord> *> baseHistory;
  
 public:
  void doWork();
  FindNLargestF(int size, int number);
  ~FindNLargestF(void);
};
#endif  // FINDNLARGESTF_H_

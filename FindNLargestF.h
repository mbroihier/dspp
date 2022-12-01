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
#include <map>
#include <math.h>
#include <sys/types.h>
/* ---------------------------------------------------------------------- */
class FindNLargestF {
 private:
  void init(int size, int number);
  void adjustThresholds(float centroid, int candidate);
  int * binArray;
  float * samples;
  float * mag;
  int sampleBufferSize;
  int size;
  int number;

  const int FIRST = 0;
  const int SECOND = 1;
  const int THIRD = 2;
  const int REF1 = 3;
  const int REF2 = 4;

  const float BW = 3.0;
  const float BW_DELTA = BW / 3.0;
  const float BW1 = BW / 6.0;
  const float BW2 = BW1 + BW_DELTA;
  const float BW3 = BW2 + BW_DELTA;
  
  std::map<int, std::map<int, float> *>  thresholds;

 public:
  void doWork();
  FindNLargestF(int size, int number);
  ~FindNLargestF(void);
};
#endif  // FINDNLARGESTF_H_

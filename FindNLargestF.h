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
#include <math.h>
#include <sys/types.h>
/* ---------------------------------------------------------------------- */
class FindNLargestF {
 private:
  void init(int size, int number);
  int * binArray;
  float * samples;
  float * mag;
  int sampleBufferSize;
  int size;
  int number;

 public:
  void doWork();
  FindNLargestF(int size, int number);
  ~FindNLargestF(void);
};
#endif  // FINDNLARGESTF_H_

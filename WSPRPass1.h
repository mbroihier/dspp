#ifndef WSPRPASS1_H_
#define WSPRPASS1_H_
/*
 *      WSPRPass1.h - Find WSPR potential WSPR signal
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <vector>
#include <map>
#include <math.h>
#include <sys/types.h>
/* ---------------------------------------------------------------------- */
class WSPRPass1 {
 private:
  const int NOMINAL_NUMBER_OF_SYMBOLS = 162;
  const int PROCESSING_SIZE = 352;  // tics in 4 minutes
  void init(int size, int number, char * prefix);
  int * binArray;
  float * samples;
  float * mag;
  float * magAcc;
  float slope;
  float yIntercept;
  int sampleBufferSize;
  int size;
  int number;
  int tic;
  float freq;
  float deltaFreq;
  char * prefix;
  float * fftOverTime;

  struct SampleRecord { float centroid; float magnitude; int timeStamp; };
  
 public:
  void doWork(void);
  WSPRPass1(int size, int number, char * prefix);
  ~WSPRPass1(void);
};
#endif  // WSPRPASS1_H_

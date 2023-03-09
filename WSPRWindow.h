#ifndef WSPRWINDOW_H_
#define WSPRWINDOW_H_
/*
 *      WSPRWindow.h - Find WSPR potential WSPR signal
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
#include "DsppFFT.h"
#include "Fano.h"
/* ---------------------------------------------------------------------- */
class WSPRWindow {
 private:
  const int NOMINAL_NUMBER_OF_SYMBOLS = 162;
  const int SHIFTS = 375;  
  const int PROCESSING_SIZE = 112;  // 112 seconds of collection time
  const int FFTS_PER_SHIFT = 165;
  //const int PROCESSING_SIZE = 352;  // tics in 4 minutes
  void init(int size, int number, char * prefix, float dialFreq, bool skipSync);
  int * binArray;
  float * mag;
  float * magAcc;
  float slope;
  float yIntercept;
  int sampleBufferSize;
  int size;
  int number;
  int tic;
  float freq;
  float dialFreq;
  float deltaFreq;
  char * prefix;
  float * fftOverTime;
  float * windowOfIQData;
  DsppFFT * fftObject;
  Fano fanoObject;
  bool skipSync;

  struct SampleRecord { float centroid; float magnitude; int timeStamp; };
  
 public:
  void doWork(void);
  WSPRWindow(int size, int number, char * prefix, float dialFreq, bool skipSync);
  ~WSPRWindow(void);
};
#endif  // WSPRWINDOW_H_

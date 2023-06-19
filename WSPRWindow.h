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
#include <time.h>
#include <queue>
#include "DsppFFT.h"
#include "Fano.h"
/* ---------------------------------------------------------------------- */
class WSPRWindow {
 private:
  const int PERIOD = 120;  // number of seconds in a WSPR period
  const int NOMINAL_NUMBER_OF_SYMBOLS = 162;
  const int SHIFTS = 375;
  const int BASE_BAND = 375;  // base band frequency width
  const int PROCESSING_SIZE = 116;  // 113 seconds of collection time - enough for 162 FFTs for every shift
  const int FFTS_PER_SHIFT = 164;   // maximum number of FFTs per sample shift (this happens only on the shift of 0)
  void init(int size, int number, char * prefix, float dialFreq);
  void remap(std::vector<int> tokens, std::vector<int> &symbols, int mapSelector);
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

  struct SampleRecord { float centroid; float magnitude; int timeStamp; };
  struct WindowOfIQDataT { time_t windowStartTime; float * data; };
  std::queue<WindowOfIQDataT> windows;
  
 public:
  void doWork(void);
  WSPRWindow(int size, int number, char * prefix, float dialFreq);
  ~WSPRWindow(void);
};
#endif  // WSPRWINDOW_H_

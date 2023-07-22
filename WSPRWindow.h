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
  const float SECONDS_PER_SHIFT = 1.0 / BASE_BAND;
  const float SECONDS_PER_SYMBOL = 256.0 / BASE_BAND;
  const float HZ_PER_BIN = BASE_BAND / 256.0;
  const float SLOPE_TO_DRIFT_UNITS = HZ_PER_BIN / SECONDS_PER_SYMBOL * 60.0; // units are Hz / minute
  void init(int size, int number, char * prefix, float dialFreq, char * reporterID, char * reporterLocation);
  int remap(std::vector<int> tokens, std::vector<int> &symbols, int mapSelector);
  int * binArray;
  float * mag;
  float * magAcc;
  float * sortedMag;
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

  struct SNRInfo { float magnitude; int bin; float SNR; };
  SNRInfo * SNRData;
  struct SampleRecord { float centroid; float magnitude; int timeStamp; };
  struct WindowOfIQDataT { time_t windowStartTime; float * data; };
  std::queue<WindowOfIQDataT> windows;

  char reporterID[13] = {0};
  char reporterLocation[7] = {0};

  static int SNRCompare(const void * a, const void * b);
  void calculateSNR(float * accumulatedMagnitude);
  float getSNR(int bin);

 public:
  void doWork(void);
  WSPRWindow(int size, int number, char * prefix, float dialFreq, char * reporterID, char * reporterLocation);
  ~WSPRWindow(void);
};
#endif  // WSPRWINDOW_H_

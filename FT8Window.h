#ifndef FT8WINDOW_H_
#define FT8WINDOW_H_
/*
 *      FT8Window.h - Find WSPR potential WSPR signal
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <vector>
#include <map>
#include <mutex>
#include <math.h>
#include <sys/types.h>
#include <time.h>
#include <queue>
#include "DsppFFT.h"
/* ---------------------------------------------------------------------- */
class FT8Window {
 private:
  const int PERIOD = 15;  // number of seconds in a WSPR period
  const int NOMINAL_NUMBER_OF_SYMBOLS = 79;
  const int SHIFTS = 512;
  const int BASE_BAND = 3200;  // base band frequency width
  const int PROCESSING_SIZE = 14;  // 14 seconds of collection time
  const int FFTS_PER_SHIFT = 92;   // maximum number of FFTs per sample shift (this used to be 164)
  const float SECONDS_PER_SHIFT = 1.0 / BASE_BAND;
  const float SECONDS_PER_SYMBOL = 512.0 / BASE_BAND;
  const float HZ_PER_BIN = BASE_BAND / 512.0;
  const float SLOPE_TO_DRIFT_UNITS = HZ_PER_BIN / SECONDS_PER_SYMBOL * 60.0; // units are Hz / minute
  void init(int size, int number, char * prefix, float dialFreq, char * reporterID, char * reporterLocation);
  int remap(std::vector<int> tokens, std::vector<int> &symbols, int mapSelector, double * ll174);
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

  struct SNRInfo { float magnitude; int bin; float SNR; };
  SNRInfo * SNRData;
  struct SampleRecord { float centroid; float magnitude; int timeStamp; };
  struct WindowOfIQDataT { time_t windowStartTime; float * data; };
  std::queue<WindowOfIQDataT> windows;
  std::mutex windowsMutex;
  
  char reporterID[13] = {0};
  char reporterLocation[7] = {0};

  static int SNRCompare(const void * a, const void * b);
  void calculateSNR(float * accumulatedMagnitude);
  float getSNR(int bin);

 public:
  void doWork(void);
  FT8Window(int size, int number, char * prefix, float dialFreq, char * reporterID, char * reporterLocation);
  ~FT8Window(void);
};
#endif  // FT8WINDOW_H_

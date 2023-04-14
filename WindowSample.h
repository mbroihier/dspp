#ifndef WINDOW_SAMPLE_H_
#define WINDOW_SAMPLE_H_
/*
 *      WindowSample.h - Pass non zero samples over a specified window of time
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <cstring>
#include <stdint.h>
/* ---------------------------------------------------------------------- */
class WindowSample {
 private:
  int samplesInWindow;
  int samplesInPeriod;
  int syncTo;
  int fillerBufferSize;
  uint8_t * filler = 0;
  uint8_t * windowOfIQSamples = 0;
 public:
  void doWork(void);
  WindowSample(int samplesInWindow, int samplesInPeriod, int syncTo);
  ~WindowSample(void);
};
#endif  // WINDOW_SAMPLE_H_

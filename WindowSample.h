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
  const int BUFFER_SIZE = 8;
  int modulo;
  int samplesInPeriod;
  int syncTo;
  float microSecondsPerSample = 0.0;
 public:
  void doWork(void);
  WindowSample(int samplesInWindow, int samplesInPeriod, int syncTo);
  ~WindowSample(void);
};
#endif  // WINDOW_SAMPLE_H_

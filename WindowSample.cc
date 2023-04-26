/*
 *      WindowSample.cc - Object that collects a window of WSPR data
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "WindowSample.h"
#define SELFTEST 1

/* ---------------------------------------------------------------------- */
WindowSample::WindowSample(int samplesInWindow, int samplesInPeriod, int syncTo) {
  fprintf(stderr, "creating WindowSample object\n");
  this->samplesInWindow = samplesInWindow;
  this->samplesInPeriod = samplesInPeriod;
  this->syncTo = syncTo;
  windowOfIQSamples = reinterpret_cast<uint8_t *>(malloc(2));
  fprintf(stderr, "done creating WindowSample object\n");
  
}

void WindowSample::doWork() {
  int currentState = time(0) % syncTo;
  int accumulator = 0;
  int count = 0;
  bool done = false;
  time_t now = 0;
  if (currentState == 0) { // if it is currently at the sync, we need to skip this window
    sleep(2.0);
  }
  while (0 != (currentState = time(0) % syncTo)) {  // wait here until time to sample
    fread(windowOfIQSamples, sizeof(uint8_t), 2, stdin);  // skip samples
  }
  while (!done) {
    now = time(0);
    fprintf(stderr, "taking samples @ %s", ctime(&now));
    accumulator = 0;
    count = -1;
    while (accumulator != samplesInPeriod && count != 0) {
      count = fread(windowOfIQSamples, sizeof(uint8_t), 2, stdin);
      fwrite(windowOfIQSamples, sizeof(uint8_t), 2, stdout);
      accumulator += count;
    }
    done =  (accumulator != samplesInPeriod);
    fprintf(stderr, "window done\n");
  }
  fprintf(stderr, "leaving doWork within WindowSample\n");
}

WindowSample::~WindowSample(void) {
  fprintf(stderr, "destructing WindowSample\n");
  if(windowOfIQSamples) free(windowOfIQSamples);
}

#ifdef SELFTEST
int main() {
  WindowSample testObj(116*1200000*2, 120*1200000*2, 120);
  testObj.doWork();
}
#endif

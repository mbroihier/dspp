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
  fillerBufferSize = samplesInPeriod - samplesInWindow;
  this->syncTo = syncTo;
  filler = reinterpret_cast<uint8_t *>(malloc(samplesInPeriod));
  windowOfIQSamples = reinterpret_cast<uint8_t *>(malloc(samplesInWindow));
  memset(filler, 0, samplesInPeriod);
  fprintf(stderr, "done creating WindowSample object\n");
  
}

void WindowSample::doWork() {
  bool currentState = time(0) % syncTo;
  int count = 0;
  bool done = false;
  time_t now = 0;
  while (!done) {
    if (currentState == 0) { // if it is currently at the sync, we need to skip this window
      sleep(2.0);
    }
    while (0 != (currentState = time(0) % syncTo)) {  // wait here until time to sample
      uint8_t skipSamples[2];
      fread(skipSamples, sizeof(uint8_t), 2, stdin);  // skip samples
    }
    now = time(0);
    fprintf(stderr, "taking sample @ %s", ctime(&now));
    count = fread(windowOfIQSamples, sizeof(uint8_t), samplesInWindow, stdin);
    done =  (count != samplesInWindow);
    if (!done) {
      fwrite(windowOfIQSamples, sizeof(uint8_t), samplesInWindow, stdout);
      fwrite(filler, sizeof(uint8_t), fillerBufferSize, stdout);
      currentState = time(0) % syncTo;
    }
    now = time(0);
    fprintf(stderr, "done @ %s", ctime(&now));
  }
  fprintf(stderr, "leaving doWork within WindowSample\n");
}

WindowSample::~WindowSample(void) {
  fprintf(stderr, "destructing WindowSample\n");
  if(filler) free(filler);
  if(windowOfIQSamples) free(windowOfIQSamples);
}

#ifdef SELFTEST
int main() {
  WindowSample testObj(112*1200000*2, 120*1200000*2, 120);
  testObj.doWork();
}
#endif

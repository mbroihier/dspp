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
WindowSample::WindowSample(int samplesInPeriod, int modulo, int syncTo) {
  fprintf(stderr, "creating WindowSample object\n");
  this->samplesInPeriod = samplesInPeriod;
  this->modulo = modulo;
  this->syncTo = syncTo;
  if (samplesInPeriod % BUFFER_SIZE != 0) {
    fprintf(stderr, "Error, BUFFER_SIZE and modulo are not consistent\n");
    exit(1);
  }
  fprintf(stderr, "done creating WindowSample object\n");
}

void WindowSample::doWork() {
  int currentState = time(0) % modulo;
  int accumulator = 0;
  int count = 0;
  bool done = false;
  time_t now = 0;
  uint8_t IQSamples[BUFFER_SIZE];
  if (currentState == syncTo) { // if it is currently at the sync, we need to skip this window
    sleep(2.0);
  }
  while (syncTo != (time(0) % modulo)) {  // wait here until time to sample
    fread(IQSamples, sizeof(uint8_t), BUFFER_SIZE, stdin);  // skip samples
  }
  while (!done) {
    now = time(0);
    fprintf(stderr, "taking samples @ %s", ctime(&now));
    accumulator = 0;
    count = -1;
    while (accumulator != samplesInPeriod && count != 0) {
      count = fread(IQSamples, sizeof(uint8_t), BUFFER_SIZE, stdin);
      fwrite(IQSamples, sizeof(uint8_t), BUFFER_SIZE, stdout);
      accumulator += count;
    }
    done =  (accumulator != samplesInPeriod);
    fprintf(stderr, "window done\n");
  }
  fprintf(stderr, "leaving doWork within WindowSample\n");
}

WindowSample::~WindowSample(void) {
  fprintf(stderr, "destructing WindowSample\n");
}

#ifdef SELFTEST
int main(int argc, char *argv[]) {
  int syncTo = 0;
  if (argc == 1) {
    syncTo = 0;
  } else {
    if (argc == 2) {
      sscanf(argv[1], "%d", &syncTo);
    } else {
      fprintf(stderr, "Usage WindowSample [sync to value]\n");
      exit(1);
    }
  }
  WindowSample testObj(120*1200000*2, 120, syncTo);
  testObj.doWork();
}
#endif

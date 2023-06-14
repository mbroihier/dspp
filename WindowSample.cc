/*
 *      WindowSample.cc - Object that collects a window of WSPR data
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "WindowSample.h"
//#define SELFTEST 1

/* ---------------------------------------------------------------------- */
WindowSample::WindowSample(int samplesInPeriod, int modulo, int syncTo) {
  fprintf(stderr, "creating WindowSample object\n");
  this->samplesInPeriod = samplesInPeriod;
  this->modulo = modulo;
  this->syncTo = syncTo;
  microSecondsPerSample = (1000000.0 * modulo)/ samplesInPeriod;
  if (samplesInPeriod % BUFFER_SIZE != 0) {
    fprintf(stderr, "Error, BUFFER_SIZE and modulo are not consistent\n");
    exit(1);
  }
  if (syncTo < 0 || syncTo >= modulo) {
    fprintf(stderr, "Error, syncTo is not in the valid range of 0 to %d\n", modulo - 1);
    exit(1);
  }
  fprintf(stderr, "done creating WindowSample object\n");
}

void WindowSample::doWork() {
  const float THRESHOLD = 40000.0;
  const int TIMES_OVER_THRESHOLD = 3;
  char displayString[128];
  int countsHigh = 0;
  int countsLow = 0;
  float highTime = 0.0;
  float lowTime = 0.0;
  int correction = 0;  // at the moment, this isn't used - a simpler approach is used where I just resync
  float halfWindowPeriod = modulo * 500000.0;
  float fullWindowPeriod = modulo * 1000000.0;
  float target = syncTo == 0 ? 960000.0 : (syncTo - 1) * 1000000.0 + 960000.0;
  timeval tv;
  int currentState = 0;
  int accumulator = 0;
  int count = 0;
  bool done = false;
  uint8_t IQSamples[BUFFER_SIZE];
  int justBefore = syncTo == 0 ? modulo - 1 : syncTo - 1;
  float windowTime = 0.0;
  bool resync = true;
  while (resync) {
    fprintf(stderr, "syncing....\n");
    resync = false;
    gettimeofday(&tv, NULL);  
    currentState = tv.tv_sec % modulo;
    if (currentState == syncTo) { // if it is currently at the sync, we need to skip this window
      sleep(2.0);
    }
    gettimeofday(&tv, NULL);
    while (justBefore != ( tv.tv_sec % modulo) || tv.tv_usec < 960000) {  // wait here until time to sample
      fread(IQSamples, sizeof(uint8_t), BUFFER_SIZE, stdin);  // skip samples
      gettimeofday(&tv, NULL);  
    }
    done = false;
    while (!done) {
      windowTime = (tv.tv_sec % modulo) * 1000000.0 + tv.tv_usec; 
      float error = target - windowTime;
      if (fabs(error) >= halfWindowPeriod) {
        if (error < 0.0) {
          error += fullWindowPeriod;
        } else {
          error -= fullWindowPeriod;
        }
      }
      snprintf(displayString, sizeof(displayString), "taking samples @ %s", ctime(&tv.tv_sec));
      snprintf(&displayString[strlen(displayString) - 1], sizeof(displayString) - strlen(displayString),
               ", microseconds: %ld, windowTime: %8.0f, error: %8.0f\n", tv.tv_usec, windowTime, error);
      fprintf(stderr, "%s", displayString);
      accumulator = 0;
      count = -1;
      if (error > THRESHOLD) {
        countsLow++;
        countsHigh = 0;
        lowTime += error;
        highTime = 0.0;
      } else if (error < -THRESHOLD) {
        countsHigh++;
        countsLow = 0;
        highTime += error;
        lowTime = 0.0;
      } else {
        countsHigh = 0;
        countsLow = 0;
        lowTime = 0.0;
        highTime = 0.0;
      }
      if (countsHigh > TIMES_OVER_THRESHOLD) {
        float microSecondsHigh = highTime / countsHigh;
        float samplesCorrection = microSecondsHigh / microSecondsPerSample;
        correction = samplesCorrection + 0.5;
        correction += correction % 2;  // make even
        countsHigh = 0;
        highTime = 0.0;
        done = true;
        resync = true;
      } else if (countsLow > TIMES_OVER_THRESHOLD) {
        float microSecondsLow = lowTime / countsLow;
        float samplesCorrection = microSecondsLow / microSecondsPerSample;
        correction = samplesCorrection + 0.5;
        correction += correction % 2;
        countsLow = 0;
        lowTime = 0.0;
        done = true;
        resync = true;
      } else {
        correction = 0;
      }
      fprintf( stderr, "countsHigh %d, countsLow %d, correction %d\n", countsHigh, countsLow, correction);
      while (accumulator < samplesInPeriod && count != 0) {
        count = fread(IQSamples, sizeof(uint8_t), BUFFER_SIZE, stdin);
        fwrite(IQSamples, sizeof(uint8_t), BUFFER_SIZE, stdout);
        accumulator += count;
      }
      done =  count == 0 || done;
      fprintf(stderr, "window done\n");
      gettimeofday(&tv, NULL);
    }
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

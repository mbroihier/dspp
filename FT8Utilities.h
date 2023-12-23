#ifndef FT8_UTILITIES_H_
#define FT8_UTILITIES_H_
/*
 *      FT8Utilities.h - various utilities to support capture and play of signals and spot reporting
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <cstring>
#include <stdint.h>
#include <stdio.h>
#include "FT8Window.h"
/* ---------------------------------------------------------------------- */
class FT8Utilities {
 private:
  time_t lastTimeReportMade = 0;
  struct byteBlocks { size_t len; uint8_t * block; float freq; time_t timeStart; float SNR; };
  std::queue<byteBlocks> data;
 public:
  static const int BUFFER_SIZE = 2 * FT8Window::BASE_BAND * FT8Window::PROCESSING_SIZE;
  static int writeFile(char * fileName, float * buffer, int size);
  static int readFile(char * fileName, float * buffer, int size);
  int reportSpot(char * reporterID, char * reporterLocation, float freq, time_t timeStart,
                        float SNR, char * message);
  FT8Utilities(void);
};
#endif  // FT8_UTILITIES_H_

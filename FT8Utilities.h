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
#include <curl/curl.h>
#include <cstring>
#include <stdint.h>
#include <stdio.h>
#include "FT8Window.h"
/* ---------------------------------------------------------------------- */
class FT8Utilities {
 private:
  static const int FILLER = 2 * 4 * 375;
 public:
  static const int BUFFER_SIZE = 2 * FT8Window::BASE_BAND * FT8Window::PROCESSING_SIZE;
  static int writeFile(char * fileName, float * buffer, int size);
  static int readFile(char * fileName, float * buffer, int size);
  static int reportSpot(char * reporterID, char * reporterLocation, float freq, float deltaT, float drift,
                        char * callID, char * callLocation, char * callPower, char * SNR, char * spotDate, char * spotTime);
};
#endif  // FT8_UTILITIES_H_

#ifndef WSPR_UTILITIES_H_
#define WSPR_UTILITIES_H_
/*
 *      WSPRUtilities.h - various utilities to support capture and play of signals and spot reporting
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
/* ---------------------------------------------------------------------- */
class WSPRUtilities {
 private:
  static const int FILLER = 2 * 4 * 375;
 public:
  static const int BUFFER_SIZE = 2 * 116 * 375;
  static int writeFile(char * fileName, float * buffer, int size);
  static int readFile(char * fileName, float * buffer, int size);
  static int reportSpot(char * reporterID, char * reporterLocation, float freq, float deltaT, float drift,
                        char * callID, char * callLocation, char * callPower, char * SNR, char * spotDate, char * spotTime);
};
#endif  // WSPR_UTILITIES_H_

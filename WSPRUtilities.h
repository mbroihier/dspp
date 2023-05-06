#ifndef WSPR_UTILITIES_H_
#define WSPR_UTILITIES_H_
/*
 *      WSPRUtilities.h - various utilities to support capture and play of signals
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <cstring>
#include <stdint.h>
#include <stdio.h>
/* ---------------------------------------------------------------------- */
class WSPRUtilities {
 private:
  static const int BUFFER_SIZE = 2 * 116 * 375;
  static const int FILLER = 2 * 4 * 375;
 public:
  static int writeFile(char * fileName, float * buffer, int size);
};
#endif  // WSPR_UTILITIES_H_

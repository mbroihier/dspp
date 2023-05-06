/*
 *      WSPRUtilities.cc - Utilites to support signal file input and output
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include "WSPRUtilities.h"

/* ---------------------------------------------------------------------- */
int WSPRUtilities::writeFile(char * fileName, float * buffer, int size) {
  char info[15] = {};  // fake header
  uint32_t type = 0;
  uint64_t freq = 0;
  FILE * fd = NULL;
  float subBuffer[BUFFER_SIZE];

  if (size < BUFFER_SIZE) {
    fprintf(stderr, "illegal buffer size - static buffer size: %d, input buffer size: %d\n", BUFFER_SIZE, size);
    return -1;
  }
  memcpy(subBuffer, buffer, BUFFER_SIZE * sizeof(float));
  fd = fopen(fileName, "wb");
  if (fd) {
    fwrite(&info, sizeof(char), 14, fd);
    fwrite(&type, sizeof(uint32_t), 1, fd);
    fwrite(&freq, sizeof(uint64_t), 1, fd);
    fwrite(subBuffer, sizeof(float), BUFFER_SIZE, fd);
    fclose(fd);
  } else {
    fprintf(stderr, "could not open file %s\n", fileName);
    return -1;
  }
  return 0;
}

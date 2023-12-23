/*
 *      FT8Utilities.cc - Utilites to support signal file input and output and spot reporting
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include "FT8Utilities.h"

/* ---------------------------------------------------------------------- */
int FT8Utilities::writeFile(char * fileName, float * buffer, int size) {
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

/* ---------------------------------------------------------------------- */
int FT8Utilities::readFile(char * fileName, float * buffer, int size) {
  char info[15] = {};  // fake header
  uint32_t type = 0;
  uint64_t freq = 0;
  FILE * fd = NULL;

  if (size < BUFFER_SIZE) {
    fprintf(stderr, "illegal buffer size - static buffer size: %d, input buffer size: %d\n", BUFFER_SIZE, size);
    return -1;
  }
  fd = fopen(fileName, "rb");
  if (fd) {
    fread(&info, sizeof(char), 14, fd);
    fread(&type, sizeof(uint32_t), 1, fd);
    fread(&freq, sizeof(uint64_t), 1, fd);
    fread(buffer, sizeof(float), BUFFER_SIZE, fd);
    fclose(fd);
  } else {
    fprintf(stderr, "could not open file %s\n", fileName);
    return -1;
  }
  return 0;
}

/* ---------------------------------------------------------------------- */
int FT8Utilities::reportSpot(char * reporterID, char * reporterLocation, float freq, time_t timeStart,
                             float SNR, char * message) {
  char * localMessage = strdup(message);
  char * ptr;
  char * nextToken;
  char * firstToken = strtok_r(localMessage, " ", &ptr);
  if (strncmp(firstToken, "CQ", 2) == 0) {  // this is a CQ, so we might want to report it
    int numberOfTokens = 0;
    char * tokens[10];
    while ((nextToken = strtok_r(NULL, " ", &ptr))) {  // while not null, store away pointers
      tokens[numberOfTokens++] = nextToken;
      if (numberOfTokens > 10 ) { // something is wrong
        fprintf(stderr, "too many tokens\n");
        free(localMessage);
        return -1;
      }
    }
    if (numberOfTokens < 2) {  // there should be at least two tokens
      fprintf(stdout, "too few tokens: %d\n", numberOfTokens);
      free(localMessage);
      return -2;
    }
    // the last entry should be a location - starts with 2 characters and then 2 numbers
    char * location = strdup(tokens[numberOfTokens - 1]);
    if ((strlen(location) >= 4) &&
        (location[0] >= 'A' && location[0] <= 'Z') &&
        (location[1] >= 'A' && location[1] <= 'Z') &&
        (location[2] >= '0' && location[2] <= '9') &&
        (location[3] >= '0' && location[3] <= '9')) {
      // location looks rational, now get call sign of sender
      char * sender = strdup(tokens[numberOfTokens - 2]);
      if ((strlen(sender) > 3) && (sender[0] != '<')) {
        // sender looks rational
        data.push({strlen(sender), (uint8_t *)sender, freq, timeStart, SNR});
        data.push({strlen(location), (uint8_t *)location, freq, timeStart, SNR});
        fprintf(stdout, "will report: %s at location %s, with frequency %1.0f, signal time %d, and SNR of %1.0f\n",
                sender, location, freq, timeStart, SNR);
      } else { // rejecting callsign
        fprintf(stderr, "call sign is suspect, message was %s, sender was %s\n", message, sender);
        free(sender);
        free(location);
        free(localMessage);
        return -3;
      }
    } else { // rejecting location
      fprintf(stderr, "location didn't parse, message was %s, location was %s\n", message, location);
      free(location);
      free(localMessage);
      return -4;
    }
  } else {
    fprintf(stderr, "not a CQ, message was %s\n", message);
  }
  if (time(0) > lastTimeReportMade + 300) {  // it's been more than 5 minutes
    while (!data.empty()) {
      free(data.front().block);  // release string memory
      data.pop();
    }
    lastTimeReportMade = time(0);
    fprintf(stderr, "report queue emptied and memory released\n");
  }
  free(localMessage);
  return 0;
}
/* ---------------------------------------------------------------------- */
FT8Utilities::FT8Utilities(void) {
}

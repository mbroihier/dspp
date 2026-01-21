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
#include <netdb.h>
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include "FT8Window.h"
/* ---------------------------------------------------------------------- */
class FT8Utilities {
 private:  
  time_t lastTimeReportMade = 0;
  uint32_t sequenceNumber = 0;
  uint32_t correlationID = 0;
  struct addrinfo * result;
  struct addrinfo * destination;
  int socketFd = 0;
  int optionNumber = 0;
  std::map<std::string, int> observed;
  
  struct infoBlock { size_t len1; uint8_t * senderC; size_t len2; uint8_t * locationC; float freq;
    time_t timeStart; float SNR; };
  std::queue<infoBlock> data;
 public:
  static const int BUFFER_SIZE = 2 * FT8Window::BASE_BAND * FT8Window::PROCESSING_SIZE;
  static int writeFile(char * fileName, float * buffer, int size);
  static int readFile(char * fileName, float * buffer, int size);
  int reportSpot(char * reporterID, char * reporterLocation, float freq, time_t timeStart,
                        float SNR, char * message);
  FT8Utilities(void);
  ~FT8Utilities(void);
};
#endif  // FT8_UTILITIES_H_

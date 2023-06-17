/*
 *      WSPRUtilities.cc - Utilites to support signal file input and output and spot reporting
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

/* ---------------------------------------------------------------------- */
int WSPRUtilities::reportSpot(char * reporterID, char * reporterLocation, float freq, float deltaT, float drift,
                              char * callID, char * callLocation, char * callPower, char * SNR, char * spotDate,
                              char * spotTime) {
  char url[1024];
  CURL * curl;
  int status = 0; //successful
  snprintf(url, sizeof(url) - 1, "http://wsprnet.org/post?function=wspr&rcall=%s&rgrid=%s&rqrg=%.6f&date=%s&time=%s"
           "&sig=%.0f&dt=%.1f&tqrg=%.6f&tcall=%s&tgrid=%s&dbm=%s&version=0.2r_wsprd&mode=2",
           reporterID,
           reporterLocation,
           freq,
           spotDate,
           spotTime,
           SNR,
           deltaT,
           freq,
           callID,
           callLocation,
           callPower);
  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      status = 1;  // operation failed
    }
    curl_easy_cleanup(curl);
  }
  return status;
}

/*
 *      FT8Utilities.cc - Utilites to support signal file input and output and spot reporting
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include "FT8Utilities.h"
//#define SELFTEST
const uint8_t PSKHeader[] = { 0x00, 0x0A, 0x00, 0xAC, 0x47, 0x95, 0x32, 0x72, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                              0x00 };
const uint8_t RECEIVERFMT[] = { 0x00, 0x03, 0x00, 0x24, 0x99, 0x92, 0x00, 0x03, 0x00, 0x00, 0x80, 0x02, 0xFF, 0xFF,
                                0x00, 0x00, 0x76, 0x8F, 0x80, 0x04, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F, 0x80, 0x08,
                                0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F, 0x00, 0x00 };
const uint8_t SENDERFMT[] = { 0x00, 0x02, 0x00, 0x2C, 0x99, 0x93, 0x00, 0x05, 0x80, 0x01, 0xFF, 0xFF, 0x00, 0x00,
                              0x76, 0x8F, 0x80, 0x05, 0x00, 0x04, 0x00, 0x00, 0x76, 0x8F, 0x80, 0x0A, 0xFF, 0xFF,
                              0x00, 0x00, 0x76, 0x8F, 0x80, 0x0B, 0x00, 0x01, 0x00, 0x00, 0x76, 0x8F, 0x00, 0x96,
                              0x00, 0x04 };
const uint8_t RECEIVERDATAHEADER[] = { 0x99, 0x92, 0x00, 0x00 };
const uint8_t SENDERDATAHEADER[] = {0x99, 0x93, 0x00, 0x00 };
const char VERSION[] = "0.1ft8window";
const char MODE[] = "FT8";
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
        if (observed.find(std::string(sender)) == observed.end()) {  // if not in the map, log for reporting
          observed[std::string(sender)] = time(0);
          data.push({strlen(sender), (uint8_t *)sender, strlen(location), (uint8_t *) location, freq, timeStart, SNR});
          fprintf(stdout, "will report: %s at location %s, with frequency %1.0f, signal time %d, and SNR of %1.0f\n",
                  sender, location, freq, timeStart, SNR);
        } else {
          fprintf(stdout, "suppressing a spot report of %s\n", sender);
        }
        // remove entries greater than an hour
        for (auto iter = observed.begin(); iter != observed.end(); ) {
          if ((*iter).second + 3600 < time(0)) {
            fprintf(stdout, "Eliminating %s from the map\n", (*iter).first.c_str());
            iter = observed.erase(iter);
          } else {
            iter++;
          }
        }
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
  if ((time(0) > lastTimeReportMade + 300) && (data.size() > 0)) {  // it's been more than 5 minutes
    uint8_t packet[4096];
    int packetSize = 0;
    lastTimeReportMade = time(0);
    sequenceNumber++;
    memcpy(packet, PSKHeader, sizeof(PSKHeader));  // put header template into buffer
    // put in current time
    packet[4] = (uint8_t) ((lastTimeReportMade & 0xff000000) >> 24); 
    packet[5] = (uint8_t) ((lastTimeReportMade & 0x00ff0000) >> 16); 
    packet[6] = (uint8_t) ((lastTimeReportMade & 0x0000ff00) >> 8); 
    packet[7] = (uint8_t) ((lastTimeReportMade & 0x000000ff));
    // put in sequence number
    packet[8] = (uint8_t) ((sequenceNumber & 0xff000000) >> 24); 
    packet[9] = (uint8_t) ((sequenceNumber & 0x00ff0000) >> 16);
    packet[10] = (uint8_t) ((sequenceNumber & 0x0000ff00) >> 8); 
    packet[11] = (uint8_t) (sequenceNumber & 0x000000ff);
    // put in correlation ID
    packet[12] = (uint8_t) ((correlationID & 0xff000000) >> 24); 
    packet[13] = (uint8_t) ((correlationID & 0x00ff0000) >> 16); 
    packet[14] = (uint8_t) ((correlationID & 0x0000ff00) >> 8);
    packet[15] = (uint8_t) (correlationID & 0x000000ff);
    packetSize += sizeof(PSKHeader);
    // copy in data formats
    memcpy(&packet[packetSize], RECEIVERFMT, sizeof(RECEIVERFMT));
    packetSize += sizeof(RECEIVERFMT);
    memcpy(&packet[packetSize], SENDERFMT, sizeof(SENDERFMT));
    packetSize += sizeof(SENDERFMT);
    // now build the receiver information
    uint8_t receiverInfo[128];
    int receiverInfoSize = 0;
    memcpy(receiverInfo, RECEIVERDATAHEADER, sizeof(RECEIVERDATAHEADER));
    receiverInfoSize += sizeof(RECEIVERDATAHEADER);
    receiverInfo[receiverInfoSize++] = strlen(reporterID);
    memcpy(&receiverInfo[receiverInfoSize], reporterID, strlen(reporterID));
    receiverInfoSize += strlen(reporterID);
    receiverInfo[receiverInfoSize++] = strlen(reporterLocation);
    memcpy(&receiverInfo[receiverInfoSize], reporterLocation, strlen(reporterLocation));
    receiverInfoSize += strlen(reporterLocation);
    receiverInfo[receiverInfoSize++] = strlen(VERSION);
    memcpy(&receiverInfo[receiverInfoSize], VERSION, strlen(VERSION));
    receiverInfoSize += strlen(VERSION);
    if (receiverInfoSize % 4 != 0) {
      while (receiverInfoSize % 4 != 0) {
        receiverInfo[receiverInfoSize++] = 0;
      }
    }
    receiverInfo[2] = (uint8_t) ((receiverInfoSize & 0x0000ff00) >> 8);
    receiverInfo[3] = (uint8_t) (receiverInfoSize & 0x000000ff);
    memcpy(&packet[packetSize], receiverInfo, receiverInfoSize);
    packetSize += receiverInfoSize;
    uint8_t senderInfo[128*data.size()];
    int senderInfoSize = 0;
    memcpy(senderInfo, SENDERDATAHEADER, sizeof(SENDERDATAHEADER));
    senderInfoSize += sizeof(SENDERDATAHEADER);
    while (!data.empty()) {
      // now build the sender information
      senderInfo[senderInfoSize++] = strlen((char *) (data.front().senderC));
      memcpy(&senderInfo[senderInfoSize], data.front().senderC, strlen((char *) (data.front().senderC)));
      senderInfoSize += strlen((char *)(data.front().senderC));
      int freq = data.front().freq;
      senderInfo[senderInfoSize++] = (uint8_t) ((freq & 0xff000000) >> 24);
      senderInfo[senderInfoSize++] = (uint8_t) ((freq & 0x00ff0000) >> 16);
      senderInfo[senderInfoSize++] = (uint8_t) ((freq & 0x0000ff00) >> 8);
      senderInfo[senderInfoSize++] = (uint8_t) (freq & 0x000000ff);
      senderInfo[senderInfoSize++] = strlen(MODE);
      memcpy(&senderInfo[senderInfoSize], MODE, strlen(MODE));
      senderInfoSize += strlen(MODE);
      senderInfo[senderInfoSize++] = 1;
      int timeStart = data.front().timeStart;
      senderInfo[senderInfoSize++] = (uint8_t) ((timeStart & 0xff000000) >> 24);
      senderInfo[senderInfoSize++] = (uint8_t) ((timeStart & 0x00ff0000) >> 16);
      senderInfo[senderInfoSize++] = (uint8_t) ((timeStart & 0x0000ff00) >> 8);
      senderInfo[senderInfoSize++] = (uint8_t) (timeStart & 0x000000ff);
      free(data.front().senderC);  // release string memory
      free(data.front().locationC);  // release string memory
      data.pop();
    }
    if (senderInfoSize % 4 != 0) {
      while (senderInfoSize % 4 != 0) {
        senderInfo[senderInfoSize++] = 0;
      }
    }
    senderInfo[2] = (uint8_t) ((senderInfoSize & 0x0000ff00) >> 8);
    senderInfo[3] = (uint8_t) (senderInfoSize & 0x000000ff);
    memcpy(&packet[packetSize], senderInfo, senderInfoSize);
    packetSize += senderInfoSize;
    packet[2] = (uint8_t) ((packetSize & 0x0000ff00) >> 8);
    packet[3] = (uint8_t) (packetSize & 0x000000ff);
    // packet that will be sent
    fprintf(stderr, "PSK Reporter Packet\n");
    for (int i = 0; i < packetSize; i++) {
      if ((i & 0xf) == 0) fprintf(stderr, "\n");
      fprintf(stderr, " %2.2x", packet[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "\nreport queue emptied and memory released\n");
    // send packet to server
    int bytes = sendto(socketFd, packet, packetSize, MSG_DONTWAIT+MSG_NOSIGNAL,
                       destination->ai_addr, destination->ai_addrlen);
    fprintf(stdout, "sent %d bytes - packet size was %d\n", bytes, packetSize);
  }
  free(localMessage);
  return 0;
}
/* ---------------------------------------------------------------------- */
FT8Utilities::FT8Utilities(void) {
  correlationID = rand();
  int status = getaddrinfo("report.pskreporter.info", "4739", 0, &result);
  if (status != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    exit(-1);
  }
  bool connectionOK = false;
  for (auto rp = result; rp != NULL; rp = rp->ai_next) {
    optionNumber++;
    destination = rp;
    socketFd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (socketFd == -1) continue;
    if (connect(socketFd, rp->ai_addr, rp->ai_addrlen) != -1) {
      connectionOK = true;
      break;
    }
  }
  if (connectionOK) {
    fprintf(stderr, "Connection was successful, number of options processed: %d\n", optionNumber);
    optionNumber--; // point to option to use
  } else {
    fprintf(stderr, "No connection made\n");
  }
}
FT8Utilities::~FT8Utilities(void) {
  if( socketFd) {
    close(socketFd);
    freeaddrinfo(result);
  }
}

#ifdef SELFTEST
int main() {
  FT8Utilities utilObject;
  char reporter[] = "KG5YJE/1";
  char reporterLoc[] = "EM13";
  char msg[] = "CQ KG5YJE EM130c";
  utilObject.reportSpot(reporter, reporterLoc, 28074000.0, time(0)/15*15, -1.0, msg);
  return 0;
}
#endif

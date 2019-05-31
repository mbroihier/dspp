/*
 *      RTLTCPClient.cc - RTL TCP client class - produces object that reads
 *                        a stream from rtl_tcp server and sends it to standard
 *                        output.
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "RTLTCPClient.h"

/* ---------------------------------------------------------------------- */
RTLTCPClient::RTLTCPClient(const char * address, int port) {
  doWork(address, port, 0, 0);
}

RTLTCPClient::RTLTCPClient(const char * address, int port, int frequency, int sampleRate) {
  doWork(address, port, frequency, sampleRate);
}

void RTLTCPClient::doWork(const char * address, int port, int frequency, int sampleRate) {
  fprintf(stderr, "Making a client connection to %s, port: %d\n", address, port);
  memset(&RTLServerSocketAddr, 0, sizeof(RTLServerSocketAddr));
  RTLServerSocketAddr.sin_family = AF_INET;
  RTLServerSocketAddr.sin_port = htons(port);
  RTLServerSocketAddr.sin_addr.s_addr = inet_addr(address);
  mySocket = socket(AF_INET, SOCK_STREAM, 0);
  if (myConnection) {
    fprintf(stderr, "Socket creation successful\n");
  } else {
    perror("RTLTCPClient (create):");
    exit(mySocket);
  }
  myConnection = connect(mySocket, (sockaddr *)&RTLServerSocketAddr, sizeof(RTLServerSocketAddr));
  if (!myConnection) {
    fprintf(stderr, "Connection to server successful\n");
  } else {
    perror("RTLTCPClient:");
    exit(mySocket);
  }
  char buffer[4096];
  bool done = false;
  bool firstMessage = true;
  int count = 0;
  while (!done) {
    count = recv(mySocket, buffer, sizeof(buffer), 0);
    if (count <= 0) {
      done = true;
      continue;
    }
    if (count != sizeof(buffer)) {
      fprintf(stderr, "Short buffer: %d\n", count);
      if ((count == 12) && firstMessage) {
	firstMessage = false;
        if (frequency <= 0) {
          continue;
        }
        commandPacket.cmd = 0x01; // set frequency
        commandPacket.bytes[1] = (frequency >> 24) & 0xff;
        commandPacket.bytes[2] = (frequency >> 16) & 0xff;
        commandPacket.bytes[3] = (frequency >> 8) & 0xff;
        commandPacket.bytes[4] = frequency & 0xff;
        send(mySocket, commandPacket.bytes, 5, 0);

        commandPacket.cmd = 0x02; // set sample rate
        commandPacket.bytes[1] = (sampleRate >> 24) & 0xff;
        commandPacket.bytes[2] = (sampleRate >> 16) & 0xff;
        commandPacket.bytes[3] = (sampleRate >> 8) & 0xff;
        commandPacket.bytes[4] = sampleRate & 0xff;
        send(mySocket, commandPacket.bytes, 5, 0);
        continue;
      }
    }
    /*
    for (int i = 0; i < count; i++) {
      fprintf(stderr, " %x", buffer[i]);
    }
    fprintf(stderr, "\n");
    */
    fwrite(buffer, sizeof(char), count, stdout);
  }  
}

RTLTCPClient::~RTLTCPClient(void) {
  close(mySocket);
};


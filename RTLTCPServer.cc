/*
 *      RTLTCPServer.cc - dspp TCP server class - produces object that reads
 *                        a stream from standard input and sends it to attached
 *                        clients.
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
#include "RTLTCPServer.h"

/* ---------------------------------------------------------------------- */
RTLTCPServer::RTLTCPServer(const char * address, int port) {
  doWork(address, port);
}

void RTLTCPServer::doWork(const char * address, int port) {
  fprintf(stderr, "Making a server listen port at %s, port: %d\n", address, port);
  memset(&RTLServerSocketAddr, 0, sizeof(RTLServerSocketAddr));
  RTLServerSocketAddr.sin_family = AF_INET;
  RTLServerSocketAddr.sin_port = htons(port);
  RTLServerSocketAddr.sin_addr.s_addr = inet_addr(address);
  mySocket = socket(AF_INET, SOCK_STREAM, 0);
  if (mySocket) {
    fprintf(stderr, "Socket creation successful, RTLTCPServer\n");
  } else {
    perror("RTLTCPServer (create):");
    exit(mySocket);
  }
  int bindStatus = bind(mySocket, (sockaddr *)&RTLServerSocketAddr, sizeof(RTLServerSocketAddr));
  if (bindStatus >= 0) {
    fprintf(stderr, "Bound...\n");
  } else {
    perror("RTLTCPServer (bind):");
    exit(mySocket);
  }
  fprintf(stderr, "Ready to accept clients, listening\n");
  listen(mySocket, 1);
  socklen_t clientAddrLen = sizeof(clientAddr);
  myConnection = accept(mySocket, (sockaddr *)&clientAddr, &clientAddrLen);
  fprintf(stderr, "Accepting a client\n");

  const int BUFFER_SIZE = 4096;
  float buffer[BUFFER_SIZE];  // always send packets that are size of full floats
  bool done = false;
  int count = 0;
  while (!done) {
    count = fread(&buffer, sizeof(float), BUFFER_SIZE, stdin);
    if (count < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream, RTLTCPServer\n");
      done = true;
      continue;
    }
    send(myConnection, buffer, sizeof(buffer), 0);
  }
  close(myConnection);  
}

RTLTCPServer::~RTLTCPServer(void) {
  close(mySocket);
};


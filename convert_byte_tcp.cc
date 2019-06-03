/*
 *      convert_byte_tcp.cc -- DSP Pipe - convert a byte stream to a TCP stream
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "RTLTCPServer.h"
/* ---------------------------------------------------------------------- */

int convert_byte_tcp(const char * IPAddress, int port) {
  fprintf(stderr, "Creating a TCP server object\n");
  RTLTCPServer client(IPAddress, port);
  fprintf(stderr, "Server terminated\n");
  return 0;
}

/* ---------------------------------------------------------------------- */

/*
 *      convert_tcp_aUnsignedByte.cc -- DSP Pipe - convert a TCP stream connection to a 
 *                                      byte(unsigned) stream
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "RTLTCPClient.h"
/* ---------------------------------------------------------------------- */

int convert_tcp_aUnsignedByte(const char * IPAddress, int port, int frequency, int sampleRate) {
  fprintf(stderr, "Creating a client object\n");
  RTLTCPClient client(IPAddress, port, frequency, sampleRate);
  fprintf(stderr, "Client terminated\n");
  for(;;);
  return 0;
}

/* ---------------------------------------------------------------------- */

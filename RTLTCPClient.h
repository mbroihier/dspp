/*
 *      RTLTCPClient.h - TCP client for rtl_tcp server - connect to server
 *                       and stream I/Q data to standard output.  On connect,
 *                       the client will send frequency and sample rate to
 *                       the server.
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
/* ---------------------------------------------------------------------- */
class RTLTCPClient {

  private:
  int frequency;          // center frequency to capture
  int sampleRate;         // rate to sample RF data

  int readSignal();
  int writeSignalPipe();

  struct sockaddr_in RTLServerSocketAddr, mySocketAddr;
  int mySocket;
  int myConnection;

  union RTLCommand {
    unsigned char bytes[5];
    unsigned char cmd;
  };

  RTLCommand commandPacket;

  void doWork(const char * address, int port, int frequency, int sampleRate);

  public:

  RTLTCPClient(const char * address, int port);
  RTLTCPClient(const char * address, int port, int frequency, int sampleRate);

  ~RTLTCPClient(void);
    
};


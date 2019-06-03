/*
 *      RTLTCPServer.h - TCP server for sending pipe data to a dspp TCP client
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
class RTLTCPServer {

  private:

  struct sockaddr_in RTLServerSocketAddr, clientAddr;
  int mySocket;
  int myConnection;

  void doWork(const char * address, int port);

  public:

  RTLTCPServer(const char * address, int port);

  ~RTLTCPServer(void);
    
};


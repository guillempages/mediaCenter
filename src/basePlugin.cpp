#include "defines.h"

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include <string>
#include <iostream>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define DEBUG
#include "utils.h"
#include "basePlugin.h"

using std::string;

int Plugin::exec(const std::string & prog, const char ** argv) {

  PID_=fork();

  if (PID_<0) {
    return PID_;
  }
  if (PID_==0) { //child
    execvp(prog.c_str(),const_cast<char**>(argv));
    perror("Plugin child");
    std::cerr << "BasePlugin::exec( " << prog << ", ...)" << std::endl;
    exit(-1);
  }

  return PID_;
}

bool Plugin::stop() {
  DBG(std::cout << "Stopping Application (PID: " << PID_ << ")" << std::endl;)
  if (PID_>0) {
    kill(PID_,SIGTERM);
  }
  PID_=-1;
  return true;
}

int Plugin::send(const string& msg) const {
  if (port_<0) {
    return -1;
  }
  int sock;
  struct sockaddr_in remote;

  sock=socket(PF_INET, SOCK_DGRAM, 0);  
  if (sock < 0) {
    return -2;
  }
  struct hostent *phe;

  remote.sin_family=AF_INET;
  remote.sin_port=htons(port_);
  if ( (remote.sin_addr.s_addr = inet_addr("127.0.0.1")) == INADDR_NONE) {
    return -2;
  }  
  
  return sendto(sock,msg.c_str(),msg.length()+1,0,(struct sockaddr *)&remote,sizeof(remote));
}

int Plugin::alive() {
  if (!PID_) {
    return 0;
  }
  if (kill(PID_,0) ) {
    stop();
    return 0;
  }
  return(1);
}

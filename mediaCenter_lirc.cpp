#include "defines.h"

#include <iostream>
#include <string>

#include <sys/time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <lirc/lirc_client.h>

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::string;

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif


//global Variables
bool end;
struct lirc_config *lircConfig=NULL;
int sock=-1;

//Only way to stop the program is to kill it; so
// catch the signal and clean everything.
RETSIGTYPE term(int result=0) {

  end=true; //superfluous, but nice ;-)

  //end lirc
  lirc_freeconfig(lircConfig);
  lirc_deinit();

  if (sock >= 0) {
    close(sock);
  }

  //quit program
  exit(result);
}


void usage(string progName) {
  cerr << "Usage:" << endl
       << "  " << progName << " [-n Name] [-s server] [-p port]" << endl;
}

int main(int argc, char * argv[]) {

  end=false;

  //signal handlers
  signal(SIGINT, term);
  signal(SIGTERM, term);

  //remove leading path from program name.
  string programName=argv[0];
  int port=10010;
  string server="127.0.0.1";
  
  int pos=programName.rfind("/");
  if (pos!=string::npos) {
    programName=programName.substr(pos+1);
  }

  const char * basename=programName.c_str();

  int optc=0;
  while ((optc=getopt(argc,argv,"n:p:s:")) != -1 ) {
    switch (optc) {
      case 'n': // name
        programName=optarg;
        break;
      case 'p': // port
        port=atoi(optarg);
        break;
      case 's': // server
        server=optarg;
        break;
      default:
        usage(basename);
        exit(-1);
    }
  }

  DBG(cout << "Program Name: " << programName << endl);
  DBG(cout << "Server: " << server << endl);
  DBG(cout << "Port: " << port << endl);

  //init lirc
  int lircSocket;

  DBG(cout << "Listening for events for " << programName << endl);
  if((lircSocket=lirc_init(const_cast<char*>(basename),true))==-1) 
  //if((lircSocket=lirc_init(const_cast<char *>(programName.c_str()),true))==-1) 
    exit(EXIT_FAILURE);
  lirc_readconfig(NULL,&lircConfig,NULL);


  //variables for lirc
  string command;
  char * code;
  char * c;
  int ret;
  struct timeval timeout;
  int error=0;
  timeout.tv_sec=5;
  timeout.tv_usec=0; 

  //tcp socket to server
  sock=socket(PF_INET,SOCK_DGRAM, 0);

  if (sock<0) {
    perror(basename);
    term(-2);
  }

  struct hostent *phe;
  struct sockaddr_in address,local;
  char buffer[512];

  address.sin_family=local.sin_family=AF_INET;
  address.sin_port=htons(port);

  local.sin_addr.s_addr=INADDR_ANY;
  local.sin_port=0;

  if (phe=gethostbyname(server.c_str()))
    memcpy((char*)&address.sin_addr, phe->h_addr, phe->h_length);
  else if ( (address.sin_addr.s_addr = inet_addr(server.c_str())) == INADDR_NONE ) {
    perror(basename);
    term(-2);
  }

  while (!end) {
    code=NULL;
    if (lirc_nextcode(&code)==0) {
      if (code == NULL)
        continue;

        if ((ret=lirc_code2char(lircConfig,code,&c) == 0) && c!= NULL) {
          DBG(cout << "Received " << c << endl);
          int length;
          length=strlen(c);
          if (length>511)
             length=511;
          strncpy(buffer,c,512);
          buffer[length]='\n';
          buffer[length+1]=0;
          if (sendto(sock,buffer,strlen(c)+1,0,(struct sockaddr *)&address,sizeof(address)) <0) {
             perror(basename);
             term(-3);
          }
      }
      free(code);
    }
  }

  term();
  return 0;
}

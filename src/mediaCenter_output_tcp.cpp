#include <iostream>
#include <string>
#include <sstream>
#include <set>

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "mediaCenter_output.h"
#include "utils.h"

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::string;
using std::set;

//global Variables
bool end=false;
int sock=-1;
int clisock=-1;

set<int> PIDs;

MediaCenter_Output *app=NULL;

//Only way to stop the program is to kill it; so
// catch the signal and clean everything.
void term(int result) {
 
  DBG(cout << "Terminating..." << endl);

  if (!end && app && (app->getPID() >= 0)) {
    //if application is running, kill it;
    //but only if we are the parent!
    app->stop();
    delete app;
    app=NULL;
  }

  end=true;

  if (sock >= 0) {
    shutdown(sock,SHUT_RDWR);
    if (close(sock) < 0) {
      DBG(perror("close(sock)");)
    }
    sock=-1;
  }
  if (clisock >= 0) {
    shutdown(sock,SHUT_RDWR);
    if (close(clisock) < 0 ) {
      DBG(perror("close(clisock)");)
    }
    clisock=-1;
  }

  exit(result);
}

void clavaEstaca(int =0) {
  union wait status;
  int PID_;
  while ((PID_=wait(&status))>=0) {
    if (app && (PID_ == app->getPID())) {
      for (set<int>::iterator i=PIDs.begin(); i!=PIDs.end(); i++)
        kill(*i,SIGTERM);
      term(0);
    }
    if (PIDs.count(PID_)>0) {
      //some socket thread died.
      //remove it from the list; we don't need to kill it again
      PIDs.erase(PID_);
    }
  }
}

void usage(string progName) {
  cerr << "Usage:" << endl
       << "  " << progName << "-t type [-p port] [-f file] [-c config_file]" << endl;
}

int MediaCenter_Output::exec(const string& program, const char * argv[]) {
  int PID=fork();

  if (PID<0) {
    return PID;
  }
  if (PID==0) {//child
    execvp(program.c_str(),const_cast<char**>(argv));
    perror("execvp");
    exit(-1);
  }

  return PID;
}


void newConnection() {
  struct sockaddr_in remote;
  socklen_t lenremote=sizeof(remote);

  clisock=accept(sock,(struct sockaddr*)&remote,&lenremote);

  if (clisock<0) {
    perror("new Connection");
    return;
  }

  int pid=fork();
  if (pid<0) { //error
    perror("fork");
    term(-3);
  } 

  if (pid==0) { //child
    DBG(cout << "mediaCenter_output. Socket forked." << endl;)
    close(sock);
    sock=-1;
  } else { //father
    close(clisock);
    clisock=-1;
    PIDs.insert(pid);
    return;
  }
  //due to last return; from hier on is just the child.

  int error=0;
  char buffer[512];
  while(!end) {
    error=recv(clisock,buffer,512,0);
    if (error<0) {
      perror("recv");
      end=true;
      term(-3);
    } else if (error==0) {
      //remote closed the socket. End.
      DBG(cout << "mediaCenter_output. Socket closed by remote" << endl;)
      end=true;
    } else {
      if (buffer[error-1]=='\n')
        buffer[error-1]=0;
      if ((error>=2) && (buffer[error-2]=='\r'))
        buffer[error-2]=0;
      buffer[error]=0;
      string command;
      command=buffer;
      command=command.substr(0,command.find(' '));
      DBG(cout << "Received: " << command << "." << endl);

      if (app) {
         if (command == "Channel") {
           string parameter="";
           string result="";
           parameter=buffer;
           parameter=parameter.substr(command.length()+1); 
           DBG(cout << "Channel " << parameter << ":" );
           if (parameter == "Up") {
             result=app->channelUp();
           } else if (parameter == "Down") {
             result=app->channelDown();
           }
         } else if (command=="Set") {
           string parameter="";
           string value="";
           string result;
           parameter=buffer+command.length()+1;
           value=parameter.substr(parameter.find(' ')+1,parameter.length());
           parameter=parameter.substr(0,parameter.find(' '));
           DBG(cout << "Setting " << parameter << " to " << value << endl);
           if (parameter=="Channel") {
             result=app->setChannel(value);
           } else {
           }
         } else if (command=="Get") {
           string parameter="";
           string result="";
           parameter=buffer;
           parameter=parameter.substr(command.length()+1); 
           DBG(cout << "Getting " << parameter << ":" );
           if (parameter=="Type") {
             result=app->getType();
           } else if (parameter=="Title") {
             result=app->getTitle();
           } else if (parameter=="Channel") {
             result=app->getChannel();
           } else if (parameter=="Time") {
             result = intToString(app->getTime());
           } else if (parameter=="TotalTime") {
             result = intToString(app->getTotalTime());
           } else if (parameter=="Track") {
             result = intToString(app->getTrack());
           } else if (parameter=="TotalTracks") {
             result = intToString(app->getTotalTracks());
           } else if (parameter=="Chapter") {
             result = intToString(app->getChapter());
           } else if (parameter=="TotalChapters") {
             result = intToString(app->getTotalChapters());
           } else if (parameter=="Paused") {
             result = intToString(app->isPaused());
           }
           DBG(cout << result << endl;)
           send(clisock,result);
         } else if (command=="Quit") {
           end=true;
           app->stop();
         }
      } 
    }
  }

  // if we got here, the socket was closed by remote. End child.
  term(0);
}

int main (int argc, char * argv[]) {

  signal(SIGCHLD, clavaEstaca);
  signal(SIGINT, term);
  signal(SIGTERM, term);

  string programName=argv[0];
  int port=10011;
  string type="";
  string filename="";
  string config="";

  int pos=programName.rfind("/");
  if (pos!=string::npos) {
    programName=programName.substr(pos+1);
  }

 int optc=0;
 while ((optc=getopt(argc,argv,"p:f:t:c:")) != -1 ) {
   switch (optc) {
     case 'p': //port
       port=atoi(optarg);
       break;
     case 'f':
       filename=optarg;
       break;
     case 't':
       type=optarg;
       break;
     case 'c':
       config=optarg;
       break;
     default:
       usage(programName);
       exit(-1);
    }
  }

  DBG(cout << "Media type: " << type << endl);
  DBG(cout << "File Name: " << filename << endl);
  DBG(cout << "Config File Name: " << config << endl);
  DBG(cout << "Port: " << port << endl);

  app=new MediaCenter_Output();
  app->setFilename(filename);
  app->setConfig(config);

  //init socket
  sock=socket(PF_INET,SOCK_STREAM,0);

  struct sockaddr_in local;

  local.sin_family==AF_INET;
  local.sin_port=htons(port);
  local.sin_addr.s_addr=INADDR_ANY;

  int optval=1;

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) {
    perror((programName+".setSockOpt").c_str());
  }

  if (bind(sock,(struct sockaddr*)&local,sizeof(local))<0) {
    perror((programName+".bind").c_str());
    term(-2);
  }

  if (listen(sock,5) < 0 ) {
    perror((programName+".listen").c_str());
    term(-2);
  }

  if (app) {
    app->setType(type);
    app->startApplication();
  }

  while(!end) {
    newConnection(); //block and listen for new connections.
                     //accept them, fork and start all over again.
  } 

  return (0);

}


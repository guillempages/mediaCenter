#include "defines.h"

#include <iostream>
#include <string>
#include <sstream>
#include <set>

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

set<int> PIDs;

MediaCenter_output *app=NULL;

//Only way to stop the program is to kill it; so
// catch the signal and clean everything.
RETSIGTYPE term(int result) {
 
  DBG(cout << "Terminating..." << endl);

  if (!end && app && (app->getPID() >= 0)) {
    //if application is running, kill it;
    //but only if we are the parent!
    app->exit();
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

  exit(result);
}

RETSIGTYPE clavaEstaca(int =0) {
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

int MediaCenter_output::exec(const string& program, const char * argv[]) {
  int PID=fork();

  if (PID<0) {
    return PID;
  }
  if (PID==0) {//child
    execvp(program.c_str(),const_cast<char**>(argv));
    perror("execvp");
    ::exit(-1);
  }

  return PID;
}


void newConnection() {
  struct sockaddr_in remote;
  socklen_t lenremote=sizeof(remote);

  int error=0;
  char buffer[512];
  struct timeval timeout;
  fd_set sockSelect;
  timeout.tv_sec=0;
  timeout.tv_usec=200000; //5Hz should be enough

  FD_ZERO(&sockSelect);
  FD_SET(sock,&sockSelect);

  error=select(sock+1,&sockSelect,NULL,NULL,&timeout);
  if (error<=0) {
    if (error<0) {
      perror("select");
    }
  } else {
    error=recvfrom(sock,buffer,512,0,(struct sockaddr*)&remote,&lenremote);
    if (error<0) {
      perror("recv");
      term(-3);
    } else if (error==0) {
    } else {
      if (buffer[error-1]=='\n')
        buffer[error-1]=0;
      if ((error>=2) && (buffer[error-2]=='\r'))
        buffer[error-2]=0;
      buffer[error]=0;
      string command;
      command=buffer;
      DBG(cout << "Received: " << command << "." << endl);
      command=command.substr(0,command.find(' '));

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
           } else if (parameter=="Artist") {
             result=app->getArtist();
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
           sendto(sock,remote,result);
         } else if (command=="Play") {
           app->play();
         } else if (command=="Pause") {
           app->pause();
         } else if (command=="Stop") {
           app->stop();
         } else if (command=="Next") {
           app->next();
         } else if (command=="Previous") {
           app->previous();
         } else if (command=="Quit") {
           end=true;
           app->exit();
         }
      } 
    }
  }
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

  app=newApp();

  app->setFilename(filename);
  app->setConfig(config);

  //init socket
  sock=socket(PF_INET,SOCK_DGRAM,0);

  struct sockaddr_in local;

  local.sin_family=AF_INET;
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

std::string MediaCenter_output::getTitle() {return "";}
std::string MediaCenter_output::getArtist() {return "";}

std::string MediaCenter_output::getChannel() {return "";}
std::string MediaCenter_output::setChannel(const std::string & newChannel) {return "";}
std::string MediaCenter_output::channelUp(int step) {return "";}
std::string MediaCenter_output::channelDown(int step) {return "";}

int MediaCenter_output::getTime() {return 0;}
int MediaCenter_output::getTotalTime() {return 0;}
int MediaCenter_output::getTrack() {return 0;}
int MediaCenter_output::getTotalTracks() {return 0;}
int MediaCenter_output::getChapter() {return 0;}
int MediaCenter_output::getTotalChapters() {return 0;}


bool MediaCenter_output::isPaused() {return false;}

void MediaCenter_output::exit() {
  if (pid_ > 0) {
    kill(pid_,SIGTERM);
  }
}



MediaCenter_output::MediaCenter_output() : pid_(0), data(0) {}
MediaCenter_output::~MediaCenter_output() {}




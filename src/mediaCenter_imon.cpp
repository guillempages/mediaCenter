#include "defines.h"

#include <string>
#include <iostream>
#include <iomanip>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h> //for memcpy

#include <lcd>

#include "utils.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::stringstream;
using std::setw;
using std::setfill;

int sock=-1;
bool end=false;

LCD * lcd;

RETSIGTYPE term(int result=0) {
  if (sock>=0) {
    close(sock);
  }

  lcd->clear();
  lcd->disconnect();

  exit(result);
}

void usage(string progName) {
  cerr << "Usage:" << endl
       << "  " << progName << " [-s server] [-p port]" << endl;
}

int ask(int sock, const sockaddr_in* to, const std::string& text, int &result) {
  int error=0;
  char buffer[512];

  result=0;

  if ((error=sendto(sock, to, text))<=0) {
    return error;
  }

  error=recv(sock,buffer,512,0);
  if (error < 0) {
    perror("Receive");
    return error;
  } else if (error==0) {
    return error;
  } else {
    buffer[error]=0;
    if (buffer[error-1]=='\n') 
      buffer[error-1]=0;
    if ((error>=2) && (buffer[error-2]=='\r'))
      buffer[error-2]=0;
    result=atoi(buffer);
  }
  return error;
}

int ask(int sock, struct sockaddr_in* to, const std::string& text, std::string &result) {
  int error=0;
  char buffer[512];

  result="";

  if ((error=sendto(sock, to, text))<=0) {
    perror("send");
    return error;
  }

  struct timeval timeout;
  fd_set sockSelect;
  timeout.tv_sec=1;
  timeout.tv_usec=0;

  FD_ZERO(&sockSelect);
  FD_SET(sock,&sockSelect);

  error=select(sock+1,&sockSelect,NULL,NULL,&timeout);
  if (error<=0) {
    return error;
  }

  error=recv(sock,buffer,512,0);
  if (error < 0) {
    perror("Receive");
    lcd->sendLine("",1);
    lcd->sendLine("",2);
    return error;
  } else {
    buffer[error]=0;
    if (buffer[error-1]=='\n') 
      buffer[error-1]=0;
    if ((error>=2) && (buffer[error-2]=='\r'))
      buffer[error-2]=0;
    result=buffer;
  }

  return error;
}

string formatTime(int current, int total, bool paused=false) { 
  stringstream sstream;
  sstream.str("");
  //if paused, print the time only every other second
  if (!paused || time(0)%2 ) {
    if (total>=3600) {
      sstream << setw(3) << setfill(' ') << (current/3600)%100 << ":" << setw(2) << setfill('0');
    } else {
      sstream << setw(6) << setfill(' ');
    }
    sstream << (current%3600)/60 << ":" << setw(2) << setfill('0') << (current%60);
  } else { //paused
    if (total>=3600) {
      sstream << "   :  :  ";
    } else {
      sstream << "      :  ";
    }
  }
  return sstream.str();
}

int main(int argc, char * argv[]) {

  //signal handlers
  signal(SIGINT,term);
  signal(SIGTERM,term);

  //remove leading path from program name.
  string programName=argv[0];
  int port=10011;
  string server="127.0.0.1";
  
  int pos=programName.rfind("/");
  if (pos!=string::npos) {
    programName=programName.substr(pos+1);
  }

  const char * basename=programName.c_str();

  int optc=0;
  while ((optc=getopt(argc,argv,"p:s:")) != -1 ) {
    switch (optc) {
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

  DBG(cout << "Server: " << server << endl);
  DBG(cout << "Port: " << port << endl);


  lcd=new LCD;

  sock=socket(PF_INET,SOCK_DGRAM,0);
  if (sock<0) {
    perror(basename);
    term(-2);
  }
  
  struct sockaddr_in remote,local;
  struct hostent *phe;

  remote.sin_family=local.sin_family=AF_INET;
  remote.sin_port=htons(port);
  
  local.sin_addr.s_addr=INADDR_ANY;
  local.sin_port=0;

  if (phe=gethostbyname(server.c_str()))
    memcpy((char*)&remote.sin_addr, phe->h_addr, phe->h_length);
  else if ( ( remote.sin_addr.s_addr = inet_addr(server.c_str())) == INADDR_NONE) {
    perror(basename);
    term(-2);
  }

  int error=0;
  char buffer[512];
  string type="";

  sock=socket(PF_INET,SOCK_DGRAM,0);
  if (sock<0) {
    perror(basename);
    term(-2);
  }

  int countFailures=0;

  while (!end) {
    error=ask(sock,&remote,"Get Type",type);
    if (error <= 0) {
      if ( countFailures < 50 ) {
        lcd->clear();
        countFailures++;
      }
      continue; //if nothing received, try again.
    } else {
      countFailures=0;
      lcd->sendLine(type,1);
      string artist,title,oldTitle,channel,oldChannel;
      stringstream sstream;
      int currTime,totalTime,track,totalTracks,chapter,totalChapters;
      int paused,shuffle;
      title="<No Title>";
      channel="<No Channel>";
      currTime=totalTime=track=totalTracks=chapter=totalChapters=0;
      while (!end) {
        sstream.str("");
        usleep(200000); //5 Hz should be more than enough
        oldTitle=title;
        artist="";
        oldChannel=channel;
        if (type=="DVD") {
          title=type;
          //just print track, chapter and time.
          //no title available
          if (ask(sock,&remote,"Get Track",track)<=0)
            break;
          if (ask(sock,&remote,"Get Chapter",chapter)<=0)
            break;
          if (ask(sock,&remote,"Get Time",currTime)<=0)
            break;
          if (ask(sock,&remote,"Get TotalTime",totalTime)<=0)
            break;
          sstream << setfill(' ') << setw(3) << track << " " << setw(3) << setfill(' ') << chapter;
          if (ask(sock,&remote,"Get Paused",paused)<=0)
            break;
          sstream << formatTime(currTime,totalTime,paused);
          if (oldTitle!=title) {
            DBG(cout << "Title: " << title << endl;)
            lcd->sendLine(title,1);
          }
          lcd->sendLine(sstream.str(),2);
           
        } else if (type == "TV") {
          //print channel name 
          //maybe I'll add EPG sometime
          if (ask(sock,&remote,"Get Channel",channel)<=0) 
            break;
          if (ask(sock,&remote,"Get Title",title)<=0) 
            break;
          if (oldChannel!=channel) {
            DBG(cout << "Channel: " << channel << endl;)
            lcd->sendLine(channel,1);
          }
          if (oldTitle!=title) {
            DBG(cout << "Title: " << title << endl;)
            lcd->sendLine(title,2);
          }

        } else if ((type == "music") || (type == "CD")) {
          if (ask(sock,&remote,"Get Artist",artist)<0)
            break;
          if (ask(sock,&remote,"Get Title",title)<=0)
            break;
          if (ask(sock,&remote,"Get Track",track)<=0)
            break;
          if (ask(sock,&remote,"Get TotalTracks",totalTracks)<=0)
            break;
          if (ask(sock,&remote,"Get Time",currTime)<=0)
            break;
          if (ask(sock,&remote,"Get TotalTime",totalTime)<=0)
            break;
          if (ask(sock,&remote,"Get Shuffle",shuffle)<=0)
            break;

          if (artist != "") {
            title=artist + " - " + title;
          }

          int digits,totalDigits;
          if (totalTracks<1000) {
            totalDigits=3; 
            digits=3;
          } else if (totalTracks < 10000) { 
            if (track < 100) {
              digits = 2;
              totalDigits = 4;
            } else if (track <1000) {
              digits = 3;
              totalDigits = 3;
            } else {
              digits = 4;
              totalDigits = 2;
            }
          } else {
            if (track < 10) {
              digits = 1;
              totalDigits = 5;
            } else if (track < 100) {
              digits = 2;
              totalDigits = 4;
            } else if (track < 1000) {
              digits = 3;
              totalDigits = 3;
            } else if (track < 10000) {
              digits = 4;
              totalDigits = 2;
            } else {
              digits = 5;
              totalDigits = 1;
            }
          }

          sstream << setfill(' ') << setw(digits) << track << "/" << setw(totalDigits) << setfill('0');

          if ( totalDigits == 5) {
            sstream << totalTracks;
          } else if ( totalDigits == 4) {
            if (totalTracks < 10000) {
              sstream << totalTracks;
            } else { 
              sstream << "++++";
            }
          } else if (totalDigits == 3 ){
            if (totalTracks < 1000) {
              sstream << totalTracks;
            } else { 
              sstream << "+++";
            }
          } else if (totalDigits == 2 ){
            if (totalTracks < 100) {
              sstream << totalTracks;
            } else { 
              sstream << "++";
            }
          } else if (totalDigits == 1 ){
            if (totalTracks < 10) {
              sstream << totalTracks;
            } else { 
              sstream << "+";
            }
          }
          if (ask(sock,&remote,"Get Paused",paused)<=0)
            break;
          sstream << formatTime(currTime,totalTime,paused);
          if (oldTitle!=title) {
            DBG(cout << "Title: " << title << endl;)
            lcd->sendLine(title,1);
          }
          lcd->sendLine(sstream.str(),2);
        } else if (type == "movie") {
          if (ask(sock,&remote,"Get Title",title)<=0)
            break;
          if (ask(sock,&remote,"Get Time",currTime)<=0)
            break;
          if (ask(sock,&remote,"Get TotalTime",totalTime)<=0)
            break;

          sstream << "       " ;
          if (ask(sock,&remote,"Get Paused",paused)<=0)
            break;
          sstream << formatTime(currTime,totalTime,paused);
DBG(cout << "Time: " << currTime << "/" << totalTime << endl);
          if (oldTitle!=title) {
            DBG(cout << "Title: " << title << endl;)
            lcd->sendLine(title,1);
          }
          lcd->sendLine(sstream.str(),2);
        }
      } //while
      //if error on receive (e.g. timeout), clear LCD
    }
  }
  
  term(0);
  return(0);
}

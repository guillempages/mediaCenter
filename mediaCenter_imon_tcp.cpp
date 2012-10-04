#include <string>
#include <iostream>
#include <iomanip>

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <lcd.h>

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

void term(int result=0) {
  if (sock>=0) {
    close(sock);
  }

  sendLine1("",0);
  sendLine2("",0);
  closeServer();

  exit(result);
}

void usage(string progName) {
  cerr << "Usage:" << endl
       << "  " << progName << " [-s server] [-p port]" << endl;
}

int ask(int sock, const std::string& text, int &result) {
  int error=0;
  char buffer[512];

  result=0;

  if ((error=send(sock, text))<=0) {
    sendLine1("",0);
    sendLine2("",0);
    return error;
  }

  error=recv(sock,buffer,512,0);
  if (error < 0) {
    perror("Receive");
    sendLine1("",0);
    sendLine2("",0);
    return error;
  } else if (error==0) {
    DBG(cout << "Remote connection closed" << endl);
    shutdown(sock,SHUT_RDWR);
    sendLine1("",0);
    sendLine2("",0);
    return error;
  } else {
    buffer[error]=0;
    if (buffer[error-1]=='\n') 
      buffer[error-1]=0;
    if ((error>=2) && (buffer[error-2]=='\r'))
      buffer[error-2]=0;
    result=atoi(buffer);
  }
  DBG(cout << "->" << text << "   <-" << result << endl;)
  return error;
}

int ask(int sock, const std::string& text, std::string &result) {
  int error=0;
  char buffer[512];

  result="";

  if ((error=send(sock, text))<=0) {
    sendLine1("",0);
    sendLine2("",0);
    return error;
  }

  error=recv(sock,buffer,512,0);
  if (error < 0) {
    perror("Receive");
    sendLine1("",0);
    sendLine2("",0);
    return error;
  } else if (error==0) {
    DBG(cout << "Remote connection closed" << endl);
    shutdown(sock,SHUT_RDWR);
    sendLine1("",0);
    sendLine2("",0);
    return error;
  } else {
    buffer[error]=0;
    if (buffer[error-1]=='\n') 
      buffer[error-1]=0;
    if ((error>=2) && (buffer[error-2]=='\r'))
      buffer[error-2]=0;
    result=buffer;
  }

  DBG(cout << "->" << text << "   <-" << result << endl;)
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


  getServer(11111,"localhost",0);

  sock=socket(PF_INET,SOCK_STREAM,0);
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

  while (!end) {
    close(sock);
    sock=socket(PF_INET,SOCK_STREAM,0);
    if (sock<0) {
      perror(basename);
      term(-2);
    }
    if (connect(sock,(struct sockaddr*)&remote,sizeof(remote))) {
      //if connection was unsuccessful, try again in one second.
      DBG(if ( errno!=ECONNREFUSED) perror("connect");)
      sleep(1);
      continue;
    } else {
      DBG(cout << "Connection Established" << endl;)
      error=ask(sock,"Get Type",type);
      if (error <= 0) {
        continue; //if socket closed, connect again.
      } else {
        sendLine1(type.c_str(),type.length());
        string title,oldTitle,channel,oldChannel;
        stringstream sstream;
        int currTime,totalTime,track,totalTracks,chapter,totalChapters;
        int paused;
        title="<No Title>";
        channel="<No Channel>";
        currTime=totalTime=track=totalTracks=chapter=totalChapters=0;
        while (!end) {
          sstream.str("");
          usleep(200000); //5 Hz should be more than enough
          oldTitle=title;
          oldChannel=channel;
          if ((type=="DVD") || (type=="CD")) {
             title=type;
             //just print track, chapter and time.
             //no title available
            if (ask(sock,"Get Track",track)<=0)
              break;
            if (ask(sock,"Get Chapter",chapter)<=0)
              break;
            if (ask(sock,"Get Time",currTime)<=0)
              break;
            if (ask(sock,"Get TotalTime",totalTime)<=0)
              break;

            sstream << setfill(' ') << setw(3) << track << " " << setw(3) << setfill(' ') << chapter;
            if (ask(sock,"Get Paused",paused)<=0)
              break;
            sstream << formatTime(currTime,totalTime,paused);
            if (oldTitle!=title) {
              DBG(cout << "Title: " << title << endl;)
              sendLine1(title.c_str(),title.length());
            }
            sendLine2(sstream.str().c_str(),sstream.str().length());
             
          } else if (type == "TV") {
             //print channel name 
             //maybe I'll add EPG sometime
             if (ask(sock,"Get Channel",channel)<=0) 
               break;
             if (ask(sock,"Get Title",title)<=0) 
               break;
             if (oldChannel!=channel) {
               DBG(cout << "Channel: " << channel << endl;)
               sendLine1(channel.c_str(),channel.length());
             }
             if (oldTitle!=title) {
               DBG(cout << "Title: " << title << endl;)
               sendLine2(title.c_str(),title.length());
             }
          } else if (type == "music") {
            if (ask(sock,"Get Title",title)<=0)
              break;
            if (ask(sock,"Get Track",track)<=0)
              break;
            if (ask(sock,"Get TotalTracks",totalTracks)<=0)
              break;
            if (ask(sock,"Get Time",currTime)<=0)
              break;
            if (ask(sock,"Get TotalTime",totalTime)<=0)
              break;

            sstream << setfill(' ') << setw(3) << track << "/" << setw(3) << setfill('0');
            if (totalTracks<1000) {
              sstream << totalTracks;
            } else {
              sstream << "+++";
            }
            if (ask(sock,"Get Paused",paused)<=0)
              break;
            sstream << formatTime(currTime,totalTime,paused);
            if (oldTitle!=title) {
              DBG(cout << "Title: " << title << endl;)
              sendLine1(title.c_str(),title.length());
            }
            sendLine2(sstream.str().c_str(),sstream.str().length());
          } else if (type == "movie") {
            if (ask(sock,"Get Title",title)<=0)
              break;
            if (ask(sock,"Get Time",currTime)<=0)
              break;
            if (ask(sock,"Get TotalTime",totalTime)<=0)
              break;

            sstream << "       " ;
            if (ask(sock,"Get Paused",paused)<=0)
              break;
            sstream << formatTime(currTime,totalTime,paused);
            if (oldTitle!=title) {
              DBG(cout << "Title: " << title << endl;)
              sendLine1(title.c_str(),title.length());
            }
            sendLine2(sstream.str().c_str(),sstream.str().length());
          }
        }
      }
    } //if connect        
  }
  
  term(0);
  return(0);
}

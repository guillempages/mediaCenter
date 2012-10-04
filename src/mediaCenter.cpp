#include "defines.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <errno.h>

#include <arpa/inet.h>

#include "config.h"
#include "remotePlugin.h"
#include "displayPlugin.h"
#include "outputPlugin.h"
#include "menuPlugin.h"
#include "recorderPlugin.h"


using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::string;

OutputPlugin oplug;
MenuPlugin menuPlugin;

static bool end;

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif

void stopOutput();

RETSIGTYPE clavaEstaca(int =0)
{
	union wait status;
	struct rusage rusage;
	int pid;
	while ((pid=wait3(&status, WNOHANG, &rusage)) >0) {
    DBG(cout << "Process " << pid << " just died." << endl;) 
	  if (pid>0) {
	    if (pid==0) {

	    } else if (oplug.getPID()==pid) {
        stopOutput();
	    }
	  }
  }
}

RETSIGTYPE term(int =0) {
	stopOutput();
  end=true;
}

inline void resetSound() {
	system("amixer set 'IEC958 Playback AC97-SPSA' 0 >/dev/null");
}

inline void stopOutput() {
  oplug.stop();
  menuPlugin.stop();
  resetSound();
}

int main(int argc, char * argv[]) {

	int sock;
	fd_set sockSelect;
  int port=10010;
  struct sockaddr_in local,remote;
  socklen_t remoteSize=sizeof(remote);
  string configPath="";

  //remove leading path from program name.
  string programName=argv[0];
  int pos=programName.rfind("/");
  if (pos!=string::npos) {
    programName=programName.substr(pos+1);
  }

  const char * basename=programName.c_str();

  int optc=0;
  while ((optc=getopt(argc,argv,"vp:c:")) != -1 ) {
    switch (optc) {
      case 'v': { // version
        cout << PACKAGE_NAME << " version " << VERSION << endl;
        cout << "Copyright (c) Guillem Pages Gassull 2009." << endl;
        exit(0);
        break;
      }
      case 'p': { //port 
        port=atoi(optarg);
        cout << "Using non default port " << port << endl;
        break;
      }
      case 'c': { // config file
        configPath=optarg;
        cout << "Reading configuration from " << configPath << endl;
        break;
      }
      default:
  //      usage(basename);
        exit(-1);
    }
  }

	configInit(configPath);
  Config::plugins.remote.port=port;
  Config::plugins.movieMenu.remotePort=port;

//global variables:
	end=false;

//signal handlers
	signal(SIGCHLD, clavaEstaca);
	signal(SIGINT, term);
	signal(SIGTERM, term);


	FD_ZERO(&sockSelect);

  sock=socket(PF_INET, SOCK_DGRAM, 0);
  
  if (sock < 0) {
    perror(argv[0]);
    exit (-1);
  }

  local.sin_family=remote.sin_family=AF_INET;
  local.sin_addr.s_addr=INADDR_ANY;
  inet_aton("127.0.0.1",&remote.sin_addr);
  local.sin_port=htons(port);

  if (bind(sock, (struct sockaddr *)&local, sizeof(local)) <0 ) {
    perror(argv[0]);
    exit(-1);
  }

	FD_SET(sock,&sockSelect);
	
  //variables for reading the commands
	string command;
  string parameter;
	char code[512];
	int ret;
	struct timeval timeout;
	int error=0;
	timeout.tv_sec=1;
	timeout.tv_usec=200000; //5Hz should be enough.


	DBG(cout << argv[0] << " ready to go" << endl);
	
  RemotePlugin remotePlugin;
  DisplayPlugin displayPlugin;
  RecordPlugin recordPlugin;

  remotePlugin.start(Config::plugins.remote);
  displayPlugin.start(Config::plugins.display);

  //make socket non-blocking
  fcntl(sock, F_SETFL, (fcntl(sock, F_GETFL) | O_NONBLOCK));

  // main loop
	while(!end) {
	  //error=select(sock+1,&sockSelect,NULL,NULL,&timeout);
	  error=select(sock+1,&sockSelect,&sockSelect,&sockSelect,&timeout);
		if (error<0) { //nothing received...
		  if (error<0) {
	      perror("select");
	    }
	    timeout.tv_sec=1;
	    timeout.tv_usec=200000; //5Hz should be enough.
  	} else {
      error=recv(sock,code,512,0);
      if (error<=0) {
        if (errno == EAGAIN) { // Nothing to read...
          if ( !remotePlugin.alive()) {
            cerr << "Remote plugin was stopped." << endl
                 << "restarting it again" << endl;
            remotePlugin.start(Config::plugins.remote);
          }
          usleep(100000); //Sleep for 0.1 seconds
        } else {
          perror(argv[0]);
        }
      } else {
        if (code[error-1]=='\n') {
          code[error-1]=0;
        }
    		command=code;
        parameter="";
        if (command.find(' ')!=string::npos) {
          command=command.substr(0,command.find(' '));
          parameter=code+command.length()+1;
        }
	      DBG(cout << "Received " << command << endl;)
        getPlugins();
		  	if (command=="DVD") {
          stopOutput();
          Config::plugins.display.port=Config::plugins.dvd.port;
          displayPlugin.start(Config::plugins.display);
          oplug.start(Config::plugins.dvd);
	  	  } else if (command=="DVB") {
          stopOutput();
          Config::plugins.display.port=Config::plugins.dvb.port;
          displayPlugin.start(Config::plugins.display);
          oplug.start(Config::plugins.dvb);
	  	  } else if (command=="CD") {
          stopOutput();
          Config::plugins.display.port=Config::plugins.cd.port;
          displayPlugin.start(Config::plugins.display);
          oplug.start(Config::plugins.cd);
        } else if (command=="Record") {
          stopOutput();
          Config::plugins.display.port=Config::plugins.record.port;
          displayPlugin.start(Config::plugins.display);
          recordPlugin.start(Config::plugins.record);
	  	  } else if (command=="TV") {
          stopOutput();
          Config::plugins.display.port=Config::plugins.tv.port;
          displayPlugin.start(Config::plugins.display);
          oplug.start(Config::plugins.tv);
	  	  } else if (command=="Movie") {
          stopOutput();
          if (parameter=="") { //open Menu Plugin
            menuPlugin.start(Config::plugins.movieMenu);
          } else { // start movie
            Config::plugins.display.port=Config::plugins.movie.port;
            displayPlugin.start(Config::plugins.display);
            Config::plugins.movie.file=parameter;
            oplug.start(Config::plugins.movie);
          }
 	 	    } else if (command=="Music") {
          stopOutput();
          Config::plugins.display.port=Config::plugins.music.port;
          displayPlugin.start(Config::plugins.display);
          oplug.start(Config::plugins.music);
 	 	    } else if (command=="Void") {
          displayPlugin.start(Config::plugins.display);
          stopOutput();
	      } else if (command=="ChUp") {
          oplug.send("Channel Up");
          menuPlugin.send("Channel Up");
	      } else if (command=="ChDown") {
          oplug.send("Channel Down");
          menuPlugin.send("Channel Down");
	      } else if (command=="Left") {
          oplug.send("Left");
          menuPlugin.send("Left");
          recordPlugin.send("Left");
	      } else if (command=="Right") {
          oplug.send("Right");
          menuPlugin.send("Right");
          recordPlugin.send("Right");
	      } else if (command=="Up") {
          oplug.send("Up");
          menuPlugin.send("Up");
          recordPlugin.send("Up");
	      } else if (command=="Down") {
          oplug.send("Down");
          menuPlugin.send("Down");
          recordPlugin.send("Down");
	      } else if (command=="Play") {
          oplug.send("Play");
          menuPlugin.send("Play");
	      } else if (command=="Pause") {
          oplug.send("Pause");
          menuPlugin.send("Pause");
	      } else if (command=="Stop") {
          oplug.send("Stop");
          menuPlugin.send("Stop");
	      } else if (command=="Prev") {
          oplug.send("Prev");
          menuPlugin.send("Prev");
	      } else if (command=="Next") {
          oplug.send("Next");
          menuPlugin.send("Next");
	      } else if (command=="1") {
          oplug.send("1");
          menuPlugin.send("1");
          recordPlugin.send("1");
	      } else if (command=="2") {
          oplug.send("2");
          menuPlugin.send("2");
          recordPlugin.send("2");
	      } else if (command=="3") {
          oplug.send("3");
          menuPlugin.send("3");
          recordPlugin.send("3");
	      } else if (command=="4") {
          oplug.send("4");
          menuPlugin.send("4");
          recordPlugin.send("4");
	      } else if (command=="5") {
          oplug.send("5");
          menuPlugin.send("5");
          recordPlugin.send("5");
	      } else if (command=="6") {
          oplug.send("6");
          menuPlugin.send("6");
          recordPlugin.send("6");
	      } else if (command=="7") {
          oplug.send("7");
          menuPlugin.send("7");
          recordPlugin.send("7");
	      } else if (command=="8") {
          oplug.send("8");
          menuPlugin.send("8");
          recordPlugin.send("8");
	      } else if (command=="9") {
          oplug.send("9");
          menuPlugin.send("9");
          recordPlugin.send("9");
	      } else if (command=="0") {
          oplug.send("0");
          menuPlugin.send("0");
          recordPlugin.send("0");
	      } else if (command=="Enter") {
          oplug.send("Enter");
          menuPlugin.send("Enter");
          recordPlugin.send("Enter");
	      } else if (command=="Quit") {
	        end=true;
          break;
        } else {
	        cerr << "WARNING: Not yet implemented" << endl;
        }
      }
	    if (ret==-1)
	      break; 
    }
  }

  remotePlugin.stop();
  displayPlugin.stop();

  DBG(cout << "Ending Program" << endl;)
 
  stopOutput();

  return 0;
}

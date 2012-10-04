#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

#include <stdio.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/select.h>

#include <arpa/inet.h>

#include <lcd.h>
#include <xmms/xmmsctrl.h>

#include "config.h"
#include "tvcontrol.h"

#include "remotePlugin.h"

#include "xineRemote.h"

//instead of including the (unexisting) header,
//it is easier to define the only function here...
//should be in "movieMenu.h"
int initGlutWindow(int argc, char* argv[]);


using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::setw;
using std::setfill;
using std::string;
using std::vector;

int dvdPID;
int tvPID;
int xinePID; //needed by the tv process
int musicPID;
int moviePID;

int lcdPID;

string tvChannel;

bool end;

//#define DEBUG

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif

void clavaEstaca(int =0)
{
	union wait status;
	struct rusage rusage;
	int pid;
	while ((pid=wait3(&status, WNOHANG, &rusage)) >=0) {
	if (pid>0) 
	  if (pid==0) {

	  } else if (pid==dvdPID) {
	    DBG(cout << "DVD died" << endl);
	    dvdPID=0;
	    if (lcdPID>0) {
	      kill(lcdPID,15);
	    }	    
	  } else if (pid==tvPID) {
	      DBG(cout << "TV died" << endl);
	    if (lcdPID>0) {
	      kill(lcdPID,15);
	    }	    
	    if (xinePID>0) {
	      kill(xinePID,15);
	    }
	    tvPID=0; 
	    xinePID=0;
	  } else if (pid==xinePID) {
	    DBG(cout << "xine TV died" << endl);
	    // the TV xine died; commit suicide.
	    exit(0);
	  } else if (pid==musicPID) {
	    DBG(cout << "music died" << endl);
		  if (lcdPID>0) {
	      kill(lcdPID,15);
	    }	    
	    musicPID=0;
	  } else if (pid==moviePID) {
	    DBG(cout << "Movie died" << endl);
	    if (lcdPID>0) {
	      kill(lcdPID,15);
	    }	    
	    moviePID=0; 
	  } else if (pid==lcdPID) {
	    DBG(cout << "LCD died" << endl);
	    lcdPID=0; 
	  }
	}
}

void term(int =0) {
	if (xinePID>0) {
	  kill(xinePID,15);
	  xinePID=0;
	}
	if (lcdPID>0) {
	  kill(lcdPID,15);
	}
	end=true;
}

void resetSound() {
	system("amixer set 'IEC958 Playback AC97-SPSA' 0 >/dev/null");
}

void killApps() {
	if (dvdPID>0) {
	  kill(dvdPID,15);
	  dvdPID=0;
		sleep(1);
	}
	if (tvPID>0) {
	  kill(tvPID,15);
		tvPID=0;
	}	
	if (musicPID>0) {
	  kill(musicPID,15);
		musicPID=0;
	}
	if (moviePID>0) {
	  kill(moviePID,15);
	  moviePID=0;
	}
	resetSound();
}

void doDVD() {
	killApps();
	dvdPID=fork();
	if (dvdPID<0) {
	  perror("doDVD");
	  dvdPID=0;
	  return;
	} else if (dvdPID==0) {
	  //the son.
          //close the socket to the LCD. There is lcdPID for this
	  closeServer();

	  //run xine fullscreen, without buttons, enable remote, 
	  //  autoplay, autosearch DVD
	  execlp("xine","xine","-g","-n","-f","-p","-s","dvd",NULL);
	  exit(-1); //should never come here...
  }
}

void doMovie(int argc, char * argv[]) {
	killApps();
	moviePID=fork();
	if (moviePID<0) {
	  perror("doMovie");
	  moviePID=0;
	  return;
	} else if (moviePID==0) {
	  //the son.
          //close the socket to the LCD. There is lcdPID for this
	  closeServer();

	  //run the movie Menu
	  initGlutWindow(argc,argv);
	  exit(-1); //should never come here...
  }
}

void doMusic() {
	killApps();
	musicPID=fork();
	if (musicPID<0) {
	  perror("doMusic");
	  musicPID=0;
	  return;
	} else if (musicPID==0) {
	  //son
          //close the socket to the LCD. There is lcdPID for this
	  closeServer();

	  // run xmms, autoplay
	  execlp("xmms","xmms","-p",NULL);
	  exit(-1); //should never come to that...
	} 
}

void doTV() {
	killApps();
	tvPID=fork();
	if (tvPID<0) {
	  perror("doTV");
	  tvPID=0;
	  return;
	} else if (tvPID==0) {
	  // the son.
          //close the socket to the LCD. There is lcdPID for this
	  closeServer();

	  // this case is unusual; we need to read a file and feed it to 
		// another process via stdin...
	  int pipeID[2];

	  if (pipe(pipeID) <0) {
	    perror("doTV. create pipe");
	    //wouldn't be able to talk to xine; so consider ourselves dead:
	    exit(-1);
	  }

	  if ((xinePID=fork())<0) {
	    perror("doTV->fork2");
	    //can't load xine, so let's die.
	    exit(-1);
	  } else if (xinePID==0) {
	    //xine thread; play the pipe!
	    close(0); //close stdin
	    dup(pipeID[0]); //(re)open the pipe
	    close(pipeID[1]); // and close the other end

	    //give xine some time to fill the buffer; delayed TV ;-)
	    usleep(500000);
	    execlp("xine","xine","-f","-g","STDIN:/",NULL); //exec xine
	    exit(-1); //should never be called.
	  }

	  //either through exit or exec; if we're here, we're xine's parent.
	  close(pipeID[0]);
	  char buf[513];
	  int bufSize;
	  int stream=open(Config::paths.tvDevice.c_str(),O_RDONLY);
	  if (stream<0) {
	    perror("doTV. Open video device");
	    end=true;
	  }
	  while (!end) {
	    bufSize=read(stream,buf,512);
	    if (bufSize<0) {
	      perror("doTV. read from buffer");
	      end=true;
	      break;
	    }
	    write(pipeID[1],buf,bufSize);
	  }

	  close(stream);
	  close(pipeID[0]);
	  close(pipeID[1]);
	  //kill xine
	  kill(xinePID,15);

	  exit(0);
	} //end son
}


/**
 * loop that reads from fin to know what mode we are in
 * The parent (or other program) should write to it.
 * Recognized modes are: "DVD" "TV" "Music" "Movie"
 * "Quit" is also recognized to end the loop
 * Modes might change in the future
 * 
 * The global boolean variable end is used for the loop; to catch ^C 
 */
void lcdWriter(string mode="") {
	char buffer[10];

	struct timeval timeout;
	timeout.tv_sec=0;
	timeout.tv_usec=333333; //3 Hz
	int error=0;

	int songTime,curTime,remTime;
	int pos=-1;
	int oldpos=-2;
	int length=0;
	string title;
	string oldTitle;
	std::stringstream sstream;

	oldTitle="";

	XineRemote *xine=NULL;
	try {
	  xine=new XineRemote(); //default host/port
	}
	catch (...) {
	  xine=NULL;
	}

	while(!end) {
	  timeout.tv_sec=0;
	  timeout.tv_usec=333333;
	  sstream.str("");
	  
	  if (mode=="Movie") {
	    title="Movie";
	    if (xine==NULL) {
	      try {
	        xine=new XineRemote(); //default host/port
	      }
	      catch (string &s) {
	        xine=NULL;
	      } 
	    } else {
	      title=xine->getTitle();

	      sstream << "       ";
	      if ((xine->getSpeed()=="XINE_SPEED_PAUSE") && (time(0)%2)) {
		      sstream << setw(9) << setfill(' ') << xine->getCurrentTime(false);
	      } else {
	        sstream << setw(9) << setfill(' ') << xine->getCurrentTime();
	      }
	      sendLine2(sstream.str().c_str(),sstream.str().length());
	    }
	    if (title!=oldTitle)
	      sendLine1(title.c_str(),title.size());
	    oldTitle=title;
	  } else if (mode=="DVD") {
	    title="DVD";
	    if (title!=oldTitle)
	      sendLine1(title.c_str(),title.size());
	    oldTitle=title;
	    if (xine==NULL) {
	      try {
	        xine=new XineRemote(); //default host/port
	      }
	      catch (string &s) {
          xine=NULL;
        } 
	    } else {
	      sstream << setw(3) << setfill(' ') << xine->getDVDTitle() 
	              << setw(4) << setfill(' ') << xine->getChapter();
	      if ((xine->getSpeed()=="XINE_SPEED_PAUSE") && (time(0)%2)) {
		      sstream << setw(9) << setfill(' ') << xine->getCurrentTime(false);
	      } else {
	        sstream << setw(9) << setfill(' ') << xine->getCurrentTime();
	      }
	      sendLine2(sstream.str().c_str(),sstream.str().length());
	    }
	  } else if (mode=="TV") {
	    title=getCurrentChannel(Config::paths.tvDevice.c_str());
	    if (title!=oldTitle)
	      sendLine1(title.c_str(),title.size());
	    oldTitle=title;
	  } else if (mode=="Music") {

	    //get xmms data if available
	    if (xmms_remote_is_running(0)) {
	      pos=xmms_remote_get_playlist_pos(0);
	      length=xmms_remote_get_playlist_length(0);
	      title=xmms_remote_get_playlist_title(0,pos);
	      curTime=xmms_remote_get_output_time(0)/1000;
	      songTime=xmms_remote_get_playlist_time(0,pos)/1000;
	      remTime=songTime-curTime;

	      // build time string
	      sstream << setfill(' ') << setw(3) << pos+1 << "/" << setw(3) << setfill('0');
	      if (length<1000) {
	        sstream << length;
	      } else {
	        sstream << "+++";
	      }
	
	      //if paused, print the time only every other second
	      if (!xmms_remote_is_paused(0) ||
	          time(0)%2 ) { 
	        if (songTime>=3600) {
  	        sstream << setw(3) << setfill(' ') << (curTime/3600)%100 << ":" << setw(2) << setfill('0');
	        } else {
	          sstream << setw(6) << setfill(' ');
	        }
	        sstream << (curTime%3600)/60 << ":" << setw(2) << setfill('0') << (curTime%60);
	      } else {
	        if (songTime>=3600) {
	          sstream << "   :  :  ";
	        } else {
	          sstream << "      :  ";
	        }
	      } 
	    } else { //xmms_running
	      title="Player not ready";
	    }
	
	    if (pos!=oldpos) {
	      sendLine1(title.c_str(),title.size());
	    }
	    sendLine2(sstream.str().c_str(),sstream.str().size());
	    oldpos=pos;
	  } else if (mode=="Quit") {
	    end=true;
	  }
	  
	  // don't hog the CPU
	  usleep(timeout.tv_usec);

	} // end while loop

	delete xine;

	//clear display on exit
	sendLine1("",0);
	sendLine2("",0);
	closeServer();
}

void setLCDMode(string mode="") {
	if (lcdPID>0) {
	  kill(lcdPID,15);
	  lcdPID=0;
	}
	lcdPID=fork();
	if (lcdPID<0) {
	  perror("setLCDMode. Error on fork");
	  return;
	}	
	if (lcdPID==0) {
	  //son
	  lcdWriter(mode);
	  return;
 	}
}


int main(int argc, char * argv[]) {

	configInit();

//global variables:
	dvdPID=0;
	tvPID=0;
	xinePID=0;
	musicPID=0;
	moviePID=0;
	lcdPID=0;
	end=false;
	tvChannel=getCurrentChannel(Config::paths.tvDevice.c_str());

//signal handlers
	signal(SIGCHLD, clavaEstaca);
	signal(SIGINT, term);
	signal(SIGTERM, term);

	int sock;
	fd_set sockSelect;
  int port=10010;
  struct sockaddr_in local,remote;
  socklen_t remoteSize=sizeof(remote);

	FD_ZERO(&sockSelect);

  sock=socket(PF_INET, SOCK_DGRAM, 0);
  
  if (sock < 0) {
    perror(argv[0]);
    exit (-1);
  }

  local.sin_family=AF_INET;
  local.sin_addr.s_addr=INADDR_ANY;
  local.sin_port=htons(port);

  if (bind(sock, (struct sockaddr *)&local, sizeof(local)) <0 ) {
    perror(argv[0]);
    exit(-1);
  }

	FD_SET(sock,&sockSelect);

	
  //variables for reading the commands
	string command;
	char code[512];
	int ret;
	struct timeval timeout;
	int error=0;
	timeout.tv_sec=0;
	timeout.tv_usec=200000; //5Hz should be enough.


  //init lcd
	getServer(11111,"localhost",0);
	DBG(cout << argv[0] << " ready to go" << endl);
	
  // get TV channel list
	vector<string> channelList;
	int currentChannel=1;
	{
	  ifstream file(Config::paths.channelsFile.c_str());
	  string channel;
	  string strtmp;
	  int freq;
	  string name;
	  int i=0;

	  while(file.good()) {
	    name="";
	    i++;
	    file >> channel >> strtmp >> freq >> strtmp >> name;
	    if (name!="") {
	      channelList.push_back(name);
	    }	
	    if (name==tvChannel) {
	      currentChannel=i;
	    }
	  }
	}

  RemotePlugin remotePlugin;

  remotePlugin.start();

  // main loop
	while(!end) {
	  error=select(sock+1,&sockSelect,NULL,NULL,&timeout);
		if (error<0) { //nothing received...
		  if (error<0) {
	      perror("select");
	    }
	    timeout.tv_sec=0;
	    timeout.tv_usec=200000; //5Hz should be enough.
  	} else {
      error=recv(sock,code,512,0);
      if (error<=0) {
        perror(argv[0]);
      } else {
        if (code[error-1]=='\n') {
          code[error-1]=0;
        }
    		command=code;
	      DBG(cout << "Received " << command << endl;)
		  	if (command=="DVD") {
	        doDVD();
	        setLCDMode("DVD");
	  	  } else if (command=="TV") {
	        setChannel(Config::paths.tvDevice.c_str(),channelList[currentChannel-1].c_str());
	        doTV();
	        setLCDMode("TV");
	  	  } else if (command=="Movie") {
		      doMovie(argc,argv);
		      setLCDMode("Movie");
 	 	    } else if (command=="Music") {
	        doMusic();
	        setLCDMode("Music");
	      } else if (command=="ChDown") {
	        if (tvPID!=0) {
	          currentChannel+=channelList.size();
	          currentChannel%=(channelList.size()+1);
	          if (currentChannel==0)
	            currentChannel=channelList.size();
	          DBG(cout << "new Channel: " << currentChannel << endl);
	          setChannel(Config::paths.tvDevice.c_str(),channelList[currentChannel-1].c_str());
	        }
	      } else if (command=="ChUp") {
	        if (tvPID!=0) {
	          currentChannel++;
	          currentChannel%=(channelList.size()+1);
	          if (currentChannel==0)
	            currentChannel=1;
	          DBG(cout << "new Channel: " << currentChannel << endl);
	          setChannel(Config::paths.tvDevice.c_str(),channelList[currentChannel-1].c_str());
	        }
	      } else if (command=="Up") {
		// do nothing; the corresponding son will do it.
	      } else if (command=="Down") {
		// do nothing; the corresponding son will do it.
	      } else if (command=="Enter") {
		// do nothing; the corresponding son will do it.
	      } else if (command=="Quit") {
	        end=true;
        } else {
	        cout << "WARNING: Not yet implemented" << endl;
        }
      }
	    if (ret==-1)
	      break; 
    }
  }

  remotePlugin.stop();

  DBG(cout << "Ending Program" << endl;)
 
	killApps();
	if (lcdPID>0) {
	  kill(lcdPID,15); //kill the LCD writer before updating the string.
	}

	sendLine1("",0);
	sendLine2("",0);
	closeServer();

  return 0;
}

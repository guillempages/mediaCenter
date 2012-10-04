#include "defines.h"

#include <iostream>
#include <string>

#include <fcntl.h>

#include "mediaCenter_mplayer.h"

#include "tvcontrol.h"

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

/**
 * TODO: Implement MplayerRemote
 */

MediaCenter_output * newApp() {
  return new MediaCenter_mplayer;
}

MediaCenter_mplayer::MediaCenter_mplayer() {
}

MediaCenter_mplayer::~MediaCenter_mplayer() {
}

bool waitForChild;

void childReady(int=0) {
  waitForChild=false;
}

int MediaCenter_mplayer::startApplication() {

  const char * argv [8];

  if (type_ == "TV") {
    //initialize Channel Vector
    TV::initChannels(config_);
  }

  DBG(cout << "Start mplayer " << type_ << endl;)

  int i;
  i=0;

  argv[i++]="mediaCenter-mplayer";
//  argv[i++]="-nogui"; //hide controls
  argv[i++]="-fs"; //full screen
//  argv[i++]="-input"; //read commands from the fifo
//  argv[i++]=DATA->fifo();
  if (type_=="DVD") {
    argv[i++]="dvd://1";
  } else if (type_=="TV") {
    argv[i++]=filename_.c_str();
  } else if (type_=="DVB") {
    argv[i++]=filename_.c_str();
  } else if (type_=="CD") {
    argv[i++]="cdda://";
  } else if (type_=="movie") {
    argv[i++]=filename_.c_str();
  }
  argv[i++]=NULL;

  DBG( cout << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << endl;)

  pid_ = exec("mplayer",argv); 

  return pid_;
}

int MediaCenter_mplayer::getTrack() {
  int result=0;
  return result;
}

int MediaCenter_mplayer::getTotalTracks() {
  int result=0;
  return result;
}

int MediaCenter_mplayer::getChapter() {
  int result=0;
  return result;
}

int MediaCenter_mplayer::getTotalChapters() {
  int result=0;
  return result;
}

int MediaCenter_mplayer::getTime() {
  int result=0;
  
  return result;
}

int MediaCenter_mplayer::getTotalTime() {
  int result=0;
  return result;
}

string MediaCenter_mplayer::getTitle() {
  string result="";
  return result;
}

string MediaCenter_mplayer::getArtist() {
  string result="";
  return result;
}

string MediaCenter_mplayer::getChannel() {
  string result="No Channel";

  result=TV::getCurrentChannel(filename_.c_str());

  return result;
}

string MediaCenter_mplayer::setChannel(const string & newChannel) {

  return getChannel();
}

string MediaCenter_mplayer::channelUp(int step) {
  if (type_=="TV") {
    const char * device=filename_.c_str();
    TV::setChannel(TV::getCurrentChannelNum(device)+step,device);
  }
  return getChannel();
}

string MediaCenter_mplayer::channelDown(int step) {
  if (type_=="TV") {
    const char * device=filename_.c_str();
    TV::setChannel(TV::getCurrentChannelNum(device)-step,device);
  }
  return getChannel();
}
bool MediaCenter_mplayer::isPaused() {
  bool result=false;
  return result;
}


#include "defines.h"

#include <iostream>
#include <string>

#include <xmms/xmmsctrl.h>

#include "mediaCenter_xmms.h"

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

MediaCenter_output * newApp() {
  return new MediaCenter_xmms;
}

MediaCenter_xmms::MediaCenter_xmms() {
}

MediaCenter_xmms::~MediaCenter_xmms() {}

int MediaCenter_xmms::startApplication() {

  const char * argv [4];

  argv[0]="mediaCenter-xmms";
  argv[1]="-p";
  if (filename_=="") 
    argv[2]=NULL;
  else
    argv[2]=filename_.c_str();
  argv[3]=NULL;

  pid_ = exec("xmms",argv); 

  return pid_;
}

int MediaCenter_xmms::getTrack() {
  int result=0;
  if (xmms_remote_is_running(0)) {
    result=xmms_remote_get_playlist_pos(0)+1;
  } 
  return result;
}

int MediaCenter_xmms::getTotalTracks() {
  int result=0;
  if (xmms_remote_is_running(0)) {
    result=xmms_remote_get_playlist_length(0);
  }
  return result;
}

int MediaCenter_xmms::getTime() {
  int result=0;
  if (xmms_remote_is_running(0)) {
    result=xmms_remote_get_output_time(0)/1000;
  }
  return result;
}

int MediaCenter_xmms::getTotalTime() {
  int result=0;
  int pos=0;
  if (xmms_remote_is_running(0)) {
    pos=xmms_remote_get_playlist_pos(0);
    result=xmms_remote_get_playlist_time(0,pos)/1000;
  }
  return result;
}

string MediaCenter_xmms::getArtist() {
  // xmms does not allow asking only for artist.
  return "";
}

string MediaCenter_xmms::getTitle() {
  int pos=0;
  string result="";
  if (xmms_remote_is_running(0)) {
    pos=xmms_remote_get_playlist_pos(0);
    result=xmms_remote_get_playlist_title(0,pos);
  }  
  return result;
}

bool MediaCenter_xmms::isPaused() {
  bool result=true;
  if (xmms_remote_is_running(0)) {
    result=xmms_remote_is_paused(0);
  }

  return result;
}

bool MediaCenter_xmms::isShuffle() {
  bool result=false;
  if (xmms_remote_is_running(0)) {
    result=xmms_remote_is_shuffle(0);
  }

  return result;
}

void MediaCenter_xmms::play() {
  if (xmms_remote_is_running(0)) {
    xmms_remote_play(0);
  }
}

void MediaCenter_xmms::pause() {
  if (xmms_remote_is_running(0)) {
    xmms_remote_play_pause(0);
  }
}

void MediaCenter_xmms::stop() {
  if (xmms_remote_is_running(0)) {
    xmms_remote_stop(0);
  }
}

void MediaCenter_xmms::next() {
  if (xmms_remote_is_running(0)) {
    xmms_remote_playlist_next(0);
  }
}

void MediaCenter_xmms::previous() {
  if (xmms_remote_is_running(0)) {
    xmms_remote_playlist_prev(0);
  }
}

void MediaCenter_xmms::exit() {
  if (xmms_remote_is_running(0)) {
    xmms_remote_quit(0);
  }
}

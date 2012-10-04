#include <iostream>
#include <string>

#include <xmms/xmmsctrl.h>

#include "mediaCenter_output.h"

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

MediaCenter_Output::MediaCenter_Output(): pid_(-1),data(NULL){
}

MediaCenter_Output::~MediaCenter_Output() {}

int MediaCenter_Output::startApplication() {

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

int MediaCenter_Output::getTrack() {
  int result=0;
  if (xmms_remote_is_running(0)) {
    result=xmms_remote_get_playlist_pos(0)+1;
  } 
  return result;
}

int MediaCenter_Output::getTotalTracks() {
  int result=0;
  if (xmms_remote_is_running(0)) {
    result=xmms_remote_get_playlist_length(0);
  }
  return result;
}

int MediaCenter_Output::getChapter() {
  int result=0;
  return result;
}

int MediaCenter_Output::getTotalChapters() {
  int result=0;
  return result;
}

int MediaCenter_Output::getTime() {
  int result=0;
  if (xmms_remote_is_running(0)) {
    result=xmms_remote_get_output_time(0)/1000;
  }
  return result;
}

int MediaCenter_Output::getTotalTime() {
  int result=0;
  int pos=0;
  if (xmms_remote_is_running(0)) {
    pos=xmms_remote_get_playlist_pos(0);
    result=xmms_remote_get_playlist_time(0,pos)/1000;
  }
  return result;
}

string MediaCenter_Output::getArtist() {
  // xmms does not allow asking only for artist.
  return "";
}

string MediaCenter_Output::getTitle() {
  int pos=0;
  string result="";
  if (xmms_remote_is_running(0)) {
    pos=xmms_remote_get_playlist_pos(0);
    result=xmms_remote_get_playlist_title(0,pos);
  }  
  return result;
}

string MediaCenter_Output::getChannel() {
  string result="No Channel";

  return result;
}

string MediaCenter_Output::setChannel(const std::string & ) {
  return getChannel();
}

string MediaCenter_Output::channelUp(int ) {
  return getChannel();
}

string MediaCenter_Output::channelDown(int ) {
  return getChannel();
}

bool MediaCenter_Output::isPaused() {
  bool result=true;
  if (xmms_remote_is_running(0)) {
    result=xmms_remote_is_paused(0);
  }

  return result;
}


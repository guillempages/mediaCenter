#include "defines.h"

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/videodev2.h>

#include <iostream>
#include <fstream>
#include <string>

#include "tvcontrol.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::string;

Channels TV::channels_;
int TV::dummy=-1;

int TV::setFrequency(int frequency, const char *device)
{
	struct v4l2_frequency vf;
	struct v4l2_tuner vt;
	int fd, result;
	
	if ((fd = open(device, O_RDWR)) < 0) {
	  perror("Set frequency failed");
	  return fd;
	}

  vf.tuner=0;
	vf.frequency = (frequency * 16)/1000;
	result = ioctl(fd,VIDIOC_S_FREQUENCY, &vf);

	vt.index = 0;
	result = ioctl(fd, VIDIOC_G_TUNER, &vt);

	close(fd);

	if (result<0) {
	  perror("Set frequency failed");
	  return result;
	}

  return 0;
}

int TV::getFrequency(const char* device)
{
	struct v4l2_frequency vf;
	struct v4l2_tuner vt;
	int fd, result;
	
	if ((fd = open(device, O_RDWR)) < 0) {
	  perror("Get frequency failed");
	  return fd;
	}

  vf.tuner=0;
	result = ioctl(fd,VIDIOC_G_FREQUENCY, &vf);
	
	close(fd);

	return 1000*vf.frequency/16;

}

int TV::setChannel(const char * channelName,const char* device )
{
	Channels::iterator it;
  int frequency=-1;
   
  for (it=channels_.begin(); it!=channels_.end(); it++) {
    if (it->name==channelName) {
      frequency=it->frequency;
      break;

    }
  }

  if (frequency>=0) { 
	  //the channel was found. Switch.
	  setFrequency(frequency,device);
	}
	return frequency;
}

int TV::setChannel(int channel,const char * device) {
  int frequency=-1;


  while (channel<0) 
    channel+=channels_.size();

  if (channels_.size()>0) {
     frequency=channels_[channel % channels_.size()].frequency;
  }  
  
  if (frequency>=0) { 
	  //the channel was found. Switch.
	  setFrequency(frequency,device);
	}
	return frequency;
}

const char* TV::getChannel(int channelFrequency, int & channelNum) {

	string channel="No Channel";

  Channels::iterator it;
  channelNum=0;
  
  for (it=channels_.begin(); it!=channels_.end(); it++, channelNum++) {
    if (it->frequency==channelFrequency) {
      channel=it->name;
      break;
    }
  }
  if (it==channels_.end()) {
    channelNum=-1;
  }
  return channel.c_str();
}

const char* TV::getCurrentChannel(const char* device, int & channelNum) {
  std::cout << "TV: getCurrentChannel " << device << std::endl; 
	return getChannel(getFrequency(device),channelNum);
}

int TV::getCurrentChannelNum(const char* device) {
  int result;
  getChannel(getFrequency(device),result);
  return result;
}

void TV::initChannels(const string & channelFile) {
  TV::channels_.clear();
  ifstream file(channelFile.c_str());

  string name;
  string strtmp;
  bool inSection=false;
 
  Channel channel;

  while (file.good()) {
    file >> name;
    if (name.empty() || name[0]=='#') {
      file.ignore(512,'\n');
      continue;
    }

    if (name=="[channels]") {
      inSection=true;
    } else if (name[0]=='[') {
      if (inSection) {
        break;
      } else {
        inSection=false;
      }
    } else if (inSection) {
      file >> strtmp; // = 
      file >> strtmp; //frequency
      channel.frequency=atoi(strtmp.c_str());
      file >> strtmp; // =
      file >> channel.name;
      TV::channels_.push_back(channel);
    }
  }
} 

#include "defines.h"

#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "mediaCenter_xine.h"

#include "xineRemote.h"
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

MediaCenter_output * newApp() {
    return new MediaCenter_xine();
}

MediaCenter_xine::MediaCenter_xine() {
}

MediaCenter_xine::~MediaCenter_xine() {
}

bool waitForChild;

void childReady(int=0) {
    waitForChild=false;
}

int MediaCenter_xine::startApplication() {

    const char * argv [8];

    if (type_ == "TV") {
        //initialize Channel Vector
        TV::initChannels(config_);

        waitForChild=true;
        int pid;
        //create the pipe to send TV data to xine
        int pipeID[2];
        if (pipe(pipeID)<0) {
            perror("mediaCenter_xine. Create pipe failed");
            term(-1);
        }
        //install the signal for the child to wait
        signal(SIGCONT,childReady);
        //create the reading process
        pid=fork();
        if (pid<0) {
            perror("mediaCenter_xine. Create reading child failed");
            term(-1);
        } else if (pid==0) { //child
            close(pipeID[0]);
            char buf[1024];
            int bufSize;
            int stream=open(filename_.c_str(),O_RDONLY);
            if (stream<0) {
                perror("mediaCenter_xine. Could not open video device");
                kill(getppid(),SIGTERM);
                term(-1);
            }
            kill(getppid(),SIGCONT);
            while(!end) {
                bufSize=read(stream,buf,1024);
                if (bufSize<0) {
                    perror("mediaCenter_xine. Could not fill buffer");
                    kill(getppid(),SIGTERM);
                    term(-3);
                }
                write(pipeID[1],buf,bufSize);
            }
            close(stream);
            close(pipeID[1]);
            term(0);
        } else { // parent
            close(0);
            dup(pipeID[0]);
            close(pipeID[1]);
            while(waitForChild); //void
        }
    }

    DBG(cout << "Start xine " << type_ << endl;)

            int i;
    i=0;

    argv[i++]="mediaCenter-xine";
    argv[i++]="-p"; //autoplay
    argv[i++]="-g"; //hide controls
    argv[i++]="-f"; //fullscreen
    argv[i++]="-n"; //enable network (remote)
    if (type_=="DVD") {
        argv[i++]="-s";
        argv[i++]="dvd";
    } else if (type_=="TV") {
        argv[i++]="stdin://";
    } else if (type_=="DVB") {
        argv[i++]=(string("dvb://")+filename_).c_str();
    } else if (type_=="CD") {
        argv[i++]="-s";
        argv[i++]="cd";
    } else if (type_=="movie") {
        argv[i++]=filename_.c_str();
    }
    argv[i++]=NULL;

    pid_ = exec("xine",argv);

    return pid_;
}

int MediaCenter_xine::getTrack() {
    int result=0;
    result=XineRemote::getDVDTitle();
    return result;
}

int MediaCenter_xine::getTotalTracks() {
    int result=0;
    result=XineRemote::getDVDTitleCount();
    return result;
}

int MediaCenter_xine::getChapter() {
    int result=0;
    result=XineRemote::getChapter();
    return result;
}

int MediaCenter_xine::getTotalChapters() {
    int result=0;
    result=XineRemote::getChapterCount();
    return result;
}

int MediaCenter_xine::getTime() {
    int result=0;
    result=XineRemote::getPosition()/1000;

    return result;
}

int MediaCenter_xine::getTotalTime() {
    int result=0;
    result=XineRemote::getLength()/1000;
    return result;
}

string MediaCenter_xine::getTitle() {
    string result="";
    result=XineRemote::getTitle();
    return result;
}

string MediaCenter_xine::getArtist() {
    string result="";
    result=XineRemote::getArtist();
    return result;
}

string MediaCenter_xine::getChannel() {
    string result="No Channel";

    result=TV::getCurrentChannel(filename_.c_str());

    return result;
}

string MediaCenter_xine::setChannel(const string & newChannel) {

    return getChannel();
}

string MediaCenter_xine::channelUp(int step) {
    if (type_=="TV") {
        const char * device=filename_.c_str();
        TV::setChannel(TV::getCurrentChannelNum(device)+step,device);
    }
    return getChannel();
}

string MediaCenter_xine::channelDown(int step) {
    if (type_=="TV") {
        const char * device=filename_.c_str();
        TV::setChannel(TV::getCurrentChannelNum(device)-step,device);
    }
    return getChannel();
}
bool MediaCenter_xine::isPaused() {
    bool result=false;
    result=(XineRemote::getSpeed()=="XINE_SPEED_PAUSE");
    return result;
}

void MediaCenter_xine::exit() {
    XineRemote::quit();

    usleep(100000);
    MediaCenter_output::exit();
}


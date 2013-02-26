#ifndef mediaCenter_xmms_h
#define mediaCenter_xmms_h

#include <string>
#include <signal.h>
#include "mediaCenter_output.h"

class MediaCenter_xmms: public MediaCenter_output {
public:
    MediaCenter_xmms();

    ~MediaCenter_xmms();

    virtual int startApplication();

    virtual std::string getTitle();
    virtual std::string getArtist();

    virtual int getTime();
    virtual int getTotalTime();
    virtual int getTrack();
    virtual int getTotalTracks();

    virtual bool isPaused();
    virtual bool isShuffle();

    virtual void play();
    virtual void pause();
    virtual void stop();
    virtual void next();
    virtual void previous();

    virtual void exit();

protected:

};

#endif


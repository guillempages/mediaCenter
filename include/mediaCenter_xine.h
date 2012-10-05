#ifndef mediaCenter_xine_h
#define mediaCenter_xine_h

#include <string>
#include <signal.h>
#include "mediaCenter_output.h"


class MediaCenter_xine: public MediaCenter_output {
public:
    MediaCenter_xine();

    ~MediaCenter_xine();

    virtual int startApplication();

    virtual std::string getTitle();
    virtual std::string getArtist();

    virtual std::string getChannel(); //only for TV; but must be implemented anyway
    virtual std::string setChannel(const std::string & newChannel);
    virtual std::string channelUp(int step=1);
    virtual std::string channelDown(int step=1);

    virtual int getTime();
    virtual int getTotalTime();
    virtual int getTrack();
    virtual int getTotalTracks();
    virtual int getChapter();
    virtual int getTotalChapters();


    virtual bool isPaused();

    virtual void exit();

protected:

};

#endif


#ifndef XINEREMOTE_H
#define XINEREMOTE_H

#include <string>

class XineRemote {
public:
    XineRemote(std::string address = "localhost", int port = 6789);
    ~XineRemote();

    static void toggleGUI();
    static int getPosition();
    static int getLength();
    static int getChapter();
    static int getChapterCount();
    static int getDVDTitle();
    static int getDVDTitleCount();
    static int getCDTrack();
    static int getCDTrackCount();

    static std::string getTitle();
    static std::string getArtist();
    static std::string getStatus();
    static std::string getSpeed();

    static bool quit();

protected:
    static int socket_;
    static int initSocket(const std::string& address = "localhost", int port = 6789);
    static std::string doStringCommand(const std::string& command, std::string arg1 = "",
            std::string arg2 = "", bool trim = true);
    static int doIntCommand(const std::string &command, std::string arg1 = "",
            std::string arg2 = "");
    static bool doVoidCommand(const std::string &command, std::string arg1 = "",
            std::string arg2 = "");
};

#endif

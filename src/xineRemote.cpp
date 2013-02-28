#include "defines.h"

#include <string>
#include <iostream>

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/poll.h>

#include "utils.h"
#include "xineRemote.h"

using std::string;

int XineRemote::socket_ = -1;

XineRemote::XineRemote(string _address, int port) {
}

int XineRemote::initSocket(const string& _address, int port) {
    if (socket_ >= 0) {
        close(socket_);
    }

    socket_ = -1;

    struct sockaddr_in address;
    struct sockaddr_in * pAddress = &address;

    socket_ = socket(PF_INET, SOCK_STREAM, 0);

    if (socket_ < 0) {
        perror("XineRemote. Could not create socket");
        return socket_;
    }

    struct hostent *phe;

    pAddress->sin_family = AF_INET;
    pAddress->sin_port = htons(port);
    phe = gethostbyname(_address.c_str());
    if (phe) {
        memcpy((char *) &pAddress->sin_addr, phe->h_addr, phe->h_length);
    } else if ((pAddress->sin_addr.s_addr = inet_addr(_address.c_str())) == INADDR_NONE) {
        perror("XineRemote. Could not resolve address");
        close(socket_);
        socket_ = -1;
        return socket_;
    }

    if (connect(socket_, (struct sockaddr *) pAddress, sizeof(address)) < 0) {
        perror("XineRemote. Could not connect socket");
        close(socket_);
        socket_ = -1;
        return socket_;
    }

    char buffer[256];
    int error;
    //Read greeting
    if (myRecv(socket_, buffer, 256) <= 0) {
        perror("XineRemote recv");
        close(socket_);
        socket_ = -1;
        return socket_;
    } else {
        DBG("XineRemote ready to work");
    }
    return socket_;
}

XineRemote::~XineRemote() {
}

bool XineRemote::doVoidCommand(const string& command, string arg1, string arg2) {
    string line;

    int error = 0;

    if (socket_ < 0) {
        if (initSocket() < 0) {
            return false;
        }
    }

    line = command;
    if (arg1 != "") {
        line += " ";
        line += arg1;
        if (arg2 != "") {
            line += " ";
            line += arg2;
        }
    }
    line += "\n";
    char buffer[257];
    while (myRecv(socket_, buffer, 256, 0) > 0) {
        //void
    }

    error = send(socket_, line.c_str(), line.length(), 0);
    return (error > 0);
}

std::string XineRemote::doStringCommand(const string& command, string arg1, string arg2,
        bool trim) {
    string result;

    char buffer[257];
    int error = 0;

    if (doVoidCommand(command, arg1, arg2)) {
        error = myRecv(socket_, buffer, 256, 3);
    }

    if (error < 0) {
        perror("Error while receiving (string)");
        return "";
    }
    if (error > 0) {
        buffer[error] = '\0';
        for (; trim && error >= 0; error--) {
            //delete trayling spaces
            if ((buffer[error] == '\0') || (buffer[error] == '\n') || (buffer[error] == ' ')
                    || (buffer[error] == '\t')) {
                buffer[error] = '\0';
            } else {
                break;
            }
        }
    }

    result = buffer;
    int pos = result.find(':');

    while ((pos < result.length() - 1) && ((result[pos] == ':') || (result[pos] == ' '))) {
        pos++;
    }

    if ((pos < 0) || (pos >= result.length() - 1)) {
        return "";
    } else {
        return result.substr(pos, result.length());
    }

}

int XineRemote::doIntCommand(const string& command, string arg1, string arg2) {

    string line;
    int result = -1;

    line = doStringCommand(command, arg1, arg2, false);

    DBG(std::cout << ">>>" << line << "<<<" << std::endl);

    sscanf(line.c_str(), "%i", &result);

    return result;
}

void XineRemote::toggleGUI() {
    doVoidCommand("gui", "panel");
}

int XineRemote::getPosition() {
    return doIntCommand("get", "position");
}

int XineRemote::getLength() {
    return doIntCommand("get", "length");
}

int XineRemote::getChapter() {
    return doIntCommand("get", "dvd", "chapter");
}

int XineRemote::getChapterCount() {
    return doIntCommand("get", "dvd", "chapter_count");
}

int XineRemote::getDVDTitle() {
    return doIntCommand("get", "dvd", "title");
}

int XineRemote::getDVDTitleCount() {
    return doIntCommand("get", "dvd", "title_count");
}

string XineRemote::getTitle() {
    string result = doStringCommand("get", "title");

    if (result == "(null)") {
        int pos;
        result = "";
        char buffer[257];
        int error;

        doVoidCommand("playlist", "show");

        error = myRecv(socket_, buffer, 256);

        if (error < 0) {
            perror("Error while receiving (title)");
            return "";
        }
        if (error > 0) {
            buffer[error] = '\0';

            result = buffer;
        }

        pos = result.find('*');
        if (pos >= 0)
            result = result.substr(pos + 1, result.length());
        pos = result.find('\n');
        if (pos >= 0)
            result = result.substr(0, pos);

        pos = result.rfind('.');
        if (pos >= 0)
            result = result.substr(0, pos);
        pos = result.rfind('/');
        if (pos >= 0)
            result = result.substr(pos + 1, result.length());

        struct pollfd ufds;
        ufds.fd = socket_;
        ufds.events = POLLIN;
        ufds.revents = 0;

        while ((error = poll(&ufds, 1, 50)) > 0) {
            if (ufds.revents & (POLLERR | POLLHUP | POLLNVAL))
                break;

            error = myRecv(socket_, buffer, 256);
            ufds.revents = 0;
        }
    }

    return result;
}

string XineRemote::getArtist() {
    string result = doStringCommand("get", "artist");
    if (result == "(null)") {
        result = "";
    }
    return result;
}

string XineRemote::getStatus() {
    return doStringCommand("get", "status");
}

string XineRemote::getSpeed() {
    return doStringCommand("get", "speed");
}

int XineRemote::getCDTrack() {
    int track = 0;

    string result = "";

    int pos;
    result = "";
    char buffer[257];
    int error;

    doVoidCommand("playlist", "show");

    error = myRecv(socket_, buffer, 256);

    if (error < 0) {
        perror("Error while receiving (Track)");
        return error;
    }
    if (error > 0) {
        buffer[error] = '\0';

        result = buffer;
    }

    pos = result.find('*');
    if (pos >= 0)
        result = result.substr(pos + 1, result.length());
    pos = result.find('\n');
    if (pos >= 0)
        result = result.substr(0, pos);

    pos = result.rfind('.');
    if (pos >= 0)
        result = result.substr(0, pos);
    pos = result.rfind('/');
    if (pos >= 0)
        result = result.substr(pos + 1, result.length());

    struct pollfd ufds;
    ufds.fd = socket_;
    ufds.events = POLLIN;
    ufds.revents = 0;

    while ((error = poll(&ufds, 1, 50)) > 0) {
        if (ufds.revents & (POLLERR | POLLHUP | POLLNVAL))
            break;

        error = myRecv(socket_, buffer, 256);
        ufds.revents = 0;
    }

    return track;
}

bool XineRemote::quit() {
    doVoidCommand("halt");
    return true;
}

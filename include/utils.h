#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "defines.h"

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif



inline std::string intToString(int value) {
    std::stringstream sstream;

    sstream.str("");
    sstream << value;

    return sstream.str();
}

inline std::string toLower(std::string &in) {
    for (int i = 0; i < in.length(); i++) {
        if (in[i] >= 'A' && in[i] <= 'Z') {
            in[i] = in[i] - 'A' + 'a';
        }
    }
    return in;
}

inline std::string toLower(const std::string &in) {
    std::string result = in; //Copy needed to reserve space

    return toLower(result);
}

inline std::string trim(std::string & in) {
    int pos;

    pos = in.find(" ");
    if (pos != std::string::npos) {
        in = in.substr(pos);
    }

    pos = in.rfind(" ");
    if (pos != std::string::npos) {
        in = in.substr(0, pos);
    }

    return in;
}
inline std::string trim(const std::string &in) {
    std::string result = in;
    return trim(result);
}

inline int send(int sock, const std::string& text) {
    return send(sock, (text + '\n').c_str(), (text + '\n').length(), 0);
}

inline int sendto(int sock, const struct sockaddr_in * to, const std::string& text) {
    return sendto(sock, (text + '\n').c_str(), (text + '\n').length(), 0, (struct sockaddr*) to,
            sizeof(sockaddr_in));
}

inline int myRecv(int sock, char * buffer, int buflen, int _timeout = 1, struct sockaddr_in * from =
        NULL) {
    struct timeval timeout;
    timeout.tv_sec = _timeout;
    timeout.tv_usec = 0;

    int error = 0;
    socklen_t addrlen = sizeof(from);

    fd_set sockSelect;
    FD_ZERO(&sockSelect);
    FD_SET(sock, &sockSelect);

    error = select(sock + 1, &sockSelect, NULL, NULL, &timeout);
    if (error < 0) {
        DBG(perror("Select error while receiving from socket"));
        return error;
    } else if (error > 0) {
        error = recvfrom(sock, buffer, buflen, 0, (struct sockaddr *) from, &addrlen);
        if (error > 0) {
            buffer[error < buflen ? error : buflen] = 0;
        } else {
            buffer[0] = 0;
        }
        //    DBG(std::cout << "Received " << error << " Bytes: ->" << buffer << "<-" << std::endl;)
    }
    return error;
}

#endif

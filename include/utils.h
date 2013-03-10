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

class File {
private:
    std::string path;
    std::string extension;
    std::string baseName;

    void init(const std::string& filename);

public:
    File(const char* filename);
    File(const std::string& filename);

    /**
     * Get the path where the file is, without the file name.
     */
    std::string getPath() const;
    /**
     * Get the extension of the file (or empty if the file has no extension).
     */
    std::string getExtension() const;
    /**
     * Get the filename without any preceding path.
     */
    std::string getBaseName() const;
    /**
     * Get the complete filename including all path parts.
     */
    std::string getFullName() const;

    /**
     * Get the filename without path or extension.
     */
    std::string getStrippedName() const;

    operator const char*() const;

    bool operator ==(const File& rvalue) const;
    bool operator ==(const std::string& rvalue) const;
    bool operator ==(const char* rvalue) const;
};

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

inline void File::init(const std::string& filename) {
    int posPath, posExtension;
    posPath = filename.rfind('/');
    posExtension = filename.rfind('.');

    if (posPath == std::string::npos) {
        path = "";
        posPath = 0;
    } else {
        ++posPath;
        path = filename.substr(0, posPath);
    }
    if (posExtension == std::string::npos) {
        extension = "";
    } else {
        extension = filename.substr(posExtension);
    }

    baseName = filename.substr(posPath, posExtension - posPath);
}

inline File::File(const char* filename) {
    init((std::string) filename);
}

inline File::File(const std::string& filename) {
    init(filename);
}
/**
 * Get the path where the file is, without the file name.
 */
inline std::string File::getPath() const {
    if (path.length() > 0) {
        return path.substr(0,path.length()-1);
    } else {
        return path;
    }
}
/**
 * Get the extension of the file (or empty if the file has no extension).
 */
inline std::string File::getExtension() const {
    if (extension.length() > 0) {
        return extension.substr(1);
    } else {
        return extension;
    }
}
/**
 * Get the filename without any preceding path.
 */
inline std::string File::getBaseName() const {
    return baseName + extension;
}
/**
 * Get the complete filename including all path parts.
 */
inline std::string File::getFullName() const {
    return path + baseName + extension;
}
/**
 * Get the filename without path or extension.
 */
inline std::string File::getStrippedName() const {
    return baseName;
}

inline File::operator const char*() const {
    return getFullName().c_str();
}

inline bool File::operator ==(const File& rvalue) const {
    return getFullName() == rvalue.getFullName();
}

inline bool File::operator ==(const std::string& rvalue) const {
    return getFullName() == rvalue;
}

inline bool File::operator ==(const char* rvalue) const {
    return getFullName() == rvalue;
}

#endif

#include "defines.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <string.h> // for memcpy
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <dirent.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "fileutils.h"
#include "utils.h"

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::string;
using std::vector;

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif

//global Variables
bool end;
int sock = -1;
int cdDrive = -1;
string mounted = "";

//Clear dead children
RETSIGTYPE clavaEstaca(int = 0) {
    union wait status;
    struct rusage rusage;
    int pid;
    while ((pid = wait3(&status, WNOHANG, &rusage)) > 0) {
    }
}

//Only way to stop the program is to kill it; so
// catch the signal and clean everything.
RETSIGTYPE term(int result = 0) {

    end = true; //superfluous, but nice ;-)

    if (sock >= 0) {
        close(sock);
    }

    if (cdDrive >= 0) {
        close(cdDrive);
    }

    if (mounted != "") {
        int error=umount(mounted.c_str());
        if (error != 0) {
            perror("umount");
        }
        mounted = "";
    }

    //quit program
    exit(result);
}


inline std::vector<File> getDirectoryContents(std::string directoryName) {
    std::vector<File> result;

    DIR * directory = opendir(directoryName.c_str());

    struct dirent *entry;

    if (directory == NULL) {
        perror("opendir failed");
        return result;
    }

    entry = readdir(directory);
    while (entry != NULL) {
        result.push_back(entry->d_name);
        entry = readdir(directory);
    };

    closedir(directory);

    return result;
}

string getDiskType(const string& mountPoint) {
    vector<File> dirContents = getDirectoryContents(mountPoint);
    string result = "";
    bool audio_ts = false;
    bool video_ts = false;

    for (vector<File>::iterator i=dirContents.begin(); i!=dirContents.end(); i++) {
        if (*i == "." || *i == "..") {
            /* void */
        } else if (*i == "AUDIO_TS") {
            audio_ts = true;
        } else if (*i == "VIDEO_TS") {
            video_ts = true;
        } else {
            /* void */
        }
    }

    if (audio_ts && video_ts) {
        result = "DVD";
    }
    return result;
}

void usage(string progName) {
    cerr << "Usage:" << endl << "  " << progName
            << " [-d device] [-m mount point] [-s server] [-p port]" << endl;
}

int main(int argc, char * argv[]) {

    end = false;

    //signal handlers
    signal(SIGCHLD, clavaEstaca);
    signal(SIGINT, term);
    signal(SIGTERM, term);

    //remove leading path from program name.
    string programName = argv[0];
    int port = 10010;
    string server = "127.0.0.1";

    int pos = programName.rfind("/");
    if (pos != string::npos) {
        programName = programName.substr(pos + 1);
    }

    const char * basename = programName.c_str();

    string deviceName = "/dev/dvd";
    string mountPoint = "/media/dvd";

    int optc = 0;
    while ((optc = getopt(argc, argv, "d:m:p:s:")) != -1) {
        switch (optc) {
        case 'd': // device 
            deviceName = optarg;
            break;
        case 'm': // mountPoint 
            mountPoint = optarg;
            break;
        case 'p': // port
            port = atoi(optarg);
            break;
        case 's': // server
            server = optarg;
            break;
        default:
            usage(basename);
            exit(-1);
        }
    }

    DBG(cout << "Server: " << server << endl);DBG(cout << "Port: " << port << endl);

    DBG(cout << "Listening for events for " << programName << endl);

    struct timeval timeout;
    int error = 0;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    //udp socket to server
    sock = socket(PF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        perror(basename);
        term(-2);
    }

    struct hostent *phe;
    struct sockaddr_in address, local;
    char buffer[512];

    address.sin_family = local.sin_family = AF_INET;
    address.sin_port = htons(port);

    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = 0;

    phe = gethostbyname(server.c_str());
    if (phe) {
        memcpy((char*) &address.sin_addr, phe->h_addr, phe->h_length);
    } else if ((address.sin_addr.s_addr = inet_addr(server.c_str())) == INADDR_NONE) {
        perror(basename);
        term(-2);
    }

    cdDrive = open(deviceName.c_str(), O_RDONLY | O_NONBLOCK);
    if (cdDrive < 0) {
        perror(programName.c_str());
        end = true;
    }

    while (!end) {
        int len = read(cdDrive, buffer, 512);
        if (len > 0) {
            lseek(cdDrive, 0, SEEK_SET);
            if (mounted == "") {
                if (mount(deviceName.c_str(), mountPoint.c_str(), "udf", MS_RDONLY, NULL) < 0) {
                    perror(("Mount " + deviceName).c_str());
                } else {
                    mounted = mountPoint;
                    string diskType = getDiskType(mountPoint);
                    cout << "Found disk: " << diskType << endl;
                    sendto(sock, &address, diskType);
                }
            }
        } else {
            if (mounted != "") {
                mounted = "";
                sendto(sock, &address, "Void");
            }
        }
        sleep(2);
    }

    term();
    return 0;
}

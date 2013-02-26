#include "defines.h"

#include <GL/glut.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "utils.h"

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::vector;
using std::string;
using std::stringstream;

void * font;

int width, height;
int pos = 0;
bool windowed = false;
vector<string> menu, menuFiles;

int sock = -1;
int orgCursor = -1;
struct sockaddr_in remote;
string dir = ".";

void finish() {
    if (sock >= 0) {
        close(sock);
        sock = -1;
    }

    if (orgCursor >= 0) {
        glutSetCursor(orgCursor);
        orgCursor = -1;
    }
}

RETSIGTYPE term(int result = 0) {
    finish();
    exit(result);
}

void writeText(double x, double y, const char* text) {
    if (text == NULL)
        return;

    glRasterPos2f(x, y);

    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(font, text[i]);
    }
}

int getTextWidth(const char* text) {
    int result = 0;

    if (text == NULL)
        return result;

    for (int i = 0; text[i] != '\0'; i++) {
        result += glutBitmapWidth(font, text[i]);
    }

    return result;
}

void showMenu(vector<string> menu, int pos = -1) {
    int pos0, inc;
    int i, start;
    int maxlines;

    inc = 25; //Max font size + 1
    pos0 = height - inc;
    maxlines = height / inc;

    start = pos - maxlines / 2 + menu.size();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, width, 0.0, height, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    if (menu.size() < maxlines) {
        for (i = 0; i < menu.size(); i++) {
            if (i != pos % menu.size()) {
                glColor3f(0.6, 0.9, 0.7);
                font = GLUT_BITMAP_HELVETICA_18;
            } else {
                glColor3f(0.9, 0.15, 0.2);
                font = GLUT_BITMAP_TIMES_ROMAN_24;
            }
            writeText((width - getTextWidth(menu[i % menu.size()].c_str())) / 2,
                    pos0 - (i - start) * inc, menu[i % menu.size()].c_str());
        }

    } else {

        for (i = start; (i < menu.size() || menu.size() >= maxlines) && i < start + maxlines; i++) {
            if (i % menu.size() != pos % menu.size()) {
                glColor3f(0.6, 0.9, 0.7);
                font = GLUT_BITMAP_HELVETICA_18;
            } else {
                glColor3f(0.9, 0.15, 0.2);
                font = GLUT_BITMAP_TIMES_ROMAN_24;
            }
            writeText((width - getTextWidth(menu[i % menu.size()].c_str())) / 2,
                    pos0 - (i - start) * inc, menu[i % menu.size()].c_str());
        }
    }

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

}

void displayFunction(void) {

    glClear(GL_COLOR_BUFFER_BIT);

    glPushMatrix();

    showMenu(menu, pos);

    glPopMatrix();

    glutSwapBuffers();
}

void sendList(const struct sockaddr_in * to) {
    int total = menu.size();

    stringstream sstream;
    sstream.str("");
    sstream << "Total: " << total;

    // send total
    sendto(sock, to, sstream.str());
    DBG(cout << sstream.str() << endl);

    //send the current position first...
    for (int i = pos; i < total; i++) {
        sendto(sock, to, menu[i]);
        DBG(cout << menu[i] << endl);
    }
    //and loop over
    for (int i = 0; i < pos; i++) {
        sendto(sock, to, menu[i]);
        DBG(cout << menu[i] << endl);
    }
}

void my_reshape(int w, int h) {
    width = w;
    height = h;

    glViewport(0, 0, w, h);
}

void my_handle_key(unsigned char key, int x, int y) {

    switch (key) {

    case 27: // Esc - Quits the program.
        exit(0);
        break;

    case '<': // Up
        pos = (pos + menu.size() - 1) % (menu.size() * 2);
        break;
    case '>': // Down
        pos = (pos + 1) % (menu.size() * 2);
        break;
    case '-': // Prev
        pos = (pos + 10 * menu.size() - 10) % (menu.size() * 2);
        break;
    case '+': // Next
        pos = (pos + 10) % (menu.size() * 2);
        break;
    case '\r':
    case '\n': { //Enter
        //send file to server
        string fileName = "Movie ";
        fileName = fileName + menuFiles[pos % menuFiles.size()];
        DBG(cout << "Sending " << fileName << endl);
        sendto(sock, fileName.c_str(), fileName.length() + 1, 0, (struct sockaddr*) &remote,
                sizeof(remote));
        term(0);
        break;
    }
    default:
        break;
    }
    glutPostRedisplay();
}

void my_idle() {
    fd_set fdSock;
    struct sockaddr_in from;
    int from_len;
    struct timeval timeout;
    int error;

    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    error = 0;

    FD_ZERO(&fdSock);
    FD_SET(sock, &fdSock);

    error = select(sock + 1, &fdSock, NULL, &fdSock, &timeout);
    if (error > 0) {
        char buf[513];

        if (error = myRecv(sock, buf, 512, 1, &from)) {
            buf[error] = 0;
            if ((buf[error - 1] == '\n') || (buf[error - 1] == '\r'))
                buf[error - 1] = 0;
            if ((error > 1) && ((buf[error - 2] == '\r') || (buf[error - 2] == '\n')))
                buf[error - 2] = 0;
            toLower(trim(buf));
            if (!strcmp(buf, "up")) {
                my_handle_key('<', 0, 0);
            } else if (!strcmp(buf, "down")) {
                my_handle_key('>', 0, 0);
            } else if (!strcmp(buf, "next")) {
                my_handle_key('+', 0, 0);
            } else if (!strcmp(buf, "prev")) {
                my_handle_key('-', 0, 0);
            } else if (!strcmp(buf, "list")) {
                DBG(cout << "Sending list" << endl);
                sendList(&from);
            } else if (!strcmp(buf, "enter")) {
                my_handle_key('\n', 0, 0);
            } else {
                //         DBG(cout << "mediaCenter_menu received " << buf << endl);
            }
        }
    } else if (error == 0) {
    } else if (error < 0) {
        perror("select");
    }
}

vector<string> listDirectory(const string& directory) {

    vector<string> result;

    DIR * films = opendir(directory.c_str());

    if (films == NULL) {
        perror("opendir failed");
        return result;
    }

    struct dirent *entry = readdir(films);
    string file, strtmp, ext;
    struct stat filestat;
    int pos = 0;

    while (entry != NULL) {
        file = entry->d_name;

        strtmp = directory + "/" + file;
        stat(strtmp.c_str(), &filestat);

        pos = file.rfind(".");
        if (pos >= 0) {
            ext = file.substr(pos + 1);
            std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
        } else {
            ext = "";
        }

        // Check that only video files are displayed
        if ((ext == "mpg") || (ext == "mpeg") || (ext == "avi") || (ext == "mov") || (ext == "wmv")
                || (ext == "divx")) {

            file = file.substr(0, pos);

            if (S_ISREG(filestat.st_mode)) {
                menuFiles.push_back(strtmp);
                result.push_back(file);
            }
        }

        entry = readdir(films);
    }
    std::sort(menuFiles.begin(), menuFiles.end());
    std::sort(result.begin(), result.end());
}

int initGlutWindow(int argc, char* argv[]) {

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(750, 600);

    DBG(cout << "glutInit..." << flush);
    glutInit(&argc, argv);
    DBG(cout << "done" << endl);

    glClearColor(0.8, 0.8, 1, 0.5);
    font = GLUT_BITMAP_TIMES_ROMAN_24;

    glutCreateWindow("window");

    if (!windowed) {
        glutFullScreen();
    }

    orgCursor = glutGet(GLUT_WINDOW_CURSOR);
    glutSetCursor(GLUT_CURSOR_NONE);
    glutReshapeFunc(my_reshape);
    glutKeyboardFunc(my_handle_key);
    glutDisplayFunc(&displayFunction);
    glutIdleFunc(&my_idle);

    menu = listDirectory(dir);

    signal(SIGTERM, term);
    signal(SIGINT, term);
    atexit(finish);

    glutMainLoop();
}

void usage(string progName) {
    std::cerr << "Usage:" << endl << "  " << progName
            << " [-p local port] [-s server] [-r remote port] [-f dir] [-t type] [-w]" << endl;
}

int main(int argc, char * argv[]) {

    //signal handlers
    signal(SIGINT, term);
    signal(SIGTERM, term);

    int port = 10011;
    int remotePort = 10010;
    string server = "127.0.0.1";
    string mediaType = "";

    //remove leading path from program name.
    string programName = argv[0];
    int pos = programName.rfind("/");
    if (pos != string::npos) {
        programName = programName.substr(pos + 1);
    }

    DBG( for (int i=0; i<argc; i++) { cout << argv[i] << " "; } cout << endl;);

    const char * basename = programName.c_str();

    int optc = 0;
    while ((optc = getopt(argc, argv, "r:p:s:f:t:w")) != -1) {
        switch (optc) {
        case 'p': // port
            port = atoi(optarg);
            break;
        case 's': // server
            server = optarg;
            break;
        case 'r': // remotePort
            remotePort = atoi(optarg);
            break;
        case 'f': // dir
            dir = optarg;
            break;
        case 't': // type
            mediaType = optarg;
            break;
        case 'w': // windowed
            windowed = true;
            break;
        default:
            usage(basename);
            exit(-1);
        }
    }

    DBG(cout << "Program Name: " << programName << endl);
    DBG(cout << "Port: " << port << endl);
    DBG(cout << "Server: " << server << endl);
    DBG(cout << "Remote Port: " << remotePort << endl);
    DBG(cout << "Directory: " << dir << endl);
    DBG(cout << "Type: " << mediaType << endl);

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror(basename);
        term(-2);
    }

    struct sockaddr_in local;
    struct hostent *phe;

    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*) &local, sizeof(local)) < 0) {
        perror((programName + ".bind local").c_str());
        term(-2);
    }

    remote.sin_family = AF_INET;
    remote.sin_port = htons(remotePort);

    if (phe = gethostbyname(server.c_str()))
        memcpy((char*) &remote.sin_addr, phe->h_addr, phe->h_length);
    else if ((remote.sin_addr.s_addr = inet_addr(server.c_str())) == INADDR_NONE) {
        perror(basename);
        term(-2);
    }

    initGlutWindow(argc, argv);

}

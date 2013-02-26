#include "defines.h"
#include <iostream>
#include <fstream>
#include <string>

#include <fcntl.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <errno.h>

#include <arpa/inet.h>


#include "hsm.h"
#include "mediaCenter_events.h"
#include "basePlugin.h"
#include "mediaCenter_main.h"

#include "displayPlugin.h"
#include "remotePlugin.h"
#include "outputPlugin.h"
#include "menuPlugin.h"
#include "recorderPlugin.h"

#include "config.h"

#define DEBUG
#include "utils.h"


using std::cout;
using std::cerr;
using std::endl;
using std::string;


class MyHSM:public HSM {
public:
    DisplayPlugin displayPlugin;
    RemotePlugin inputPlugin;
    
};


static MyHSM* hsm=NULL;

RETSIGTYPE clavaEstaca(int =0)
{
    union wait status;
    struct rusage rusage;
    int pid;

    while((pid=wait3(&status, WNOHANG, &rusage)) >0) {
        if (pid > 0) {
            hsm->receiveEvent(new_evtCHILD(pid));
        }
    }
}

RETSIGTYPE term(int =0) {
    hsm->receiveEvent(new_evtQUIT());
}

inline void resetSound() {
    int result = system("amixer set 'IEC958 Playback AC97-SPSA' 0 >/dev/null");
    if (result <0 ) {
        perror("Reset sound");
    }
}


/*******
 *
 * QUIT
 *
 *******/
void HSM_Quit::processEvent(const Event* event) {
    MyHSM * myHsm=(MyHSM*)this->hsm;
    switch (event->type) {
    case evtENTER: {
        myHsm->inputPlugin.stop();
        myHsm->displayPlugin.stop();
        kill(0,SIGTERM); /* kill all child processes */
        delete myHsm;
        ::hsm=NULL; /* Ugly hack */
        break;
    }
    case evtERROR: {
        if (event->msg != "" ) {
            cerr << event->msg << endl;
        }
        break;
    }
    default: {
        DBG(cout << "Nothing to do" << endl);
    }
    }
}

/*********
 *
 * START
 *
 *********/
void HSM_Start::processEvent(const Event* event) {
    MyHSM * myHsm=(MyHSM*)this->hsm;
    switch (event->type) {
    case evtENTER: {
        myHsm->inputPlugin.start(Config::plugins.remote);
        myHsm->displayPlugin.start(Config::plugins.display);
        break;
    }
    case evtQUIT: {
        if (event->msg != "" ) {
            cerr << event->msg << endl;
        }
        HSM_Quit *stQuit=new HSM_Quit(this->hsm);
        this->hsm->changeState(stQuit);
        break;
    }
    case evtERROR: {
        if (event->msg != "" ) {
            cerr << event->msg << endl;
        }
        break;
    }
    case evtNEXT: {
        Event_Next* myEvt=(Event_Next*)event;
        /* Start can only go to IDLE, so ignore nextState */
        if (myEvt->nextState) {
            delete myEvt->nextState;
        }
        /* fall-through */
    }
    case evtIDLE:
    {
        HSM_Idle *stIdle=new HSM_Idle(this->hsm);
        this->hsm->changeState(stIdle);
        break;
    }
    case evtEXIT:  {
        break;
    }
    default: {
        DBG(cout << "Nothing to do" << endl);
    }
    }
}


/*************
 *
 * WAIT CHILD
 *
 *************/
void HSM_WaitChild::processEvent(const Event* event) {
    MyHSM * myHsm=(MyHSM*)this->hsm;
    switch (event->type) {
    case evtENTER:  {
        break;
    }
    case evtCHILD: {
        EventChild* myEvent=(EventChild*)event;

        if (myEvent->pid == myHsm->inputPlugin.getPID()) {
            DBG(cout << "WARNING: Input plugin died. Restarting it" << endl);
            myHsm->inputPlugin.start(Config::plugins.remote);
        } else if (myEvent->pid == myHsm->displayPlugin.getPID()) {
            DBG(cout << "WARNING: Display plugin died. Restarting it" << endl);
            myHsm->displayPlugin.start(Config::plugins.display);
        } else if (myEvent->pid == pid) {
            DBG(cout << "Plugin (finally) died." << endl);
            HSM_Idle *stIdle=new HSM_Idle(myHsm);
            Event * exitEvt=this->exitEvt;
            myHsm->changeState(stIdle);
            myHsm->receiveEvent(exitEvt);
        }
        break;
    }
    case evtQUIT: {
        HSM_Quit *stQuit=new HSM_Quit(this->hsm);
        this->hsm->changeState(stQuit);
        break;
    }
    case evtIDLE: {
        /* Delete the next event (i.e. go to IDLE and stay there) */
        /* and send the pid as if the child stopped normally) */
        delete exitEvt;
        exitEvt=NULL;
        myHsm->receiveEvent(new_evtCHILD(pid));
        break;
    }

    case evtTIMER:
    {
        timerCount++;
        if (timerCount == 10) { //10 ticks; with a 5Hz default = 2 seconds. Should be enough
            kill(pid,SIGKILL); // Kill with -9, just in case
            break;
        }
        if (timerCount >= 15) { // 5 ticks after "kill -9". If nothing received,
            //somethin is very wrong
            cerr << "ERROR: Child process (" << pid << ") did not die within the allowed time."
                 << endl;
            /* Delete the next event (i.e. go to IDLE and stay there) */
            /* and send the pid as if the child stopped normally) */
            delete exitEvt;
            exitEvt=NULL;
            myHsm->receiveEvent(new_evtCHILD(pid));
        }
        break;
    }
    case evtERROR: {
        if (event->msg != "" ) {
            cerr << event->msg << endl;
        }
        break;
    }
    case evtEXIT:  {
        break;
    }
    default: {
        DBG(cout << "Nothing to do" << endl);
    }
    }
}

/*******
 *
 * IDLE
 *
 *******/
void HSM_Idle::processEvent(const Event* event) {
    MyHSM * myHsm=(MyHSM*)this->hsm;
    switch (event->type) {
    case evtENTER: {
        resetSound();
        break;
    }
    case evtCHILD: {
        EventChild* myEvent=(EventChild*)event;

        if (myEvent->pid == myHsm->inputPlugin.getPID()) {
            DBG(cout << "WARNING: Input plugin died. Restarting it" << endl);
            myHsm->inputPlugin.start(Config::plugins.remote);
        } else if (myEvent->pid == myHsm->displayPlugin.getPID()) {
            DBG(cout << "WARNING: Display plugin died. Restarting it" << endl);
            myHsm->displayPlugin.start(Config::plugins.display);
        }
        break;
    }
    case evtQUIT: {
        HSM_Quit *stQuit=new HSM_Quit(this->hsm);
        this->hsm->changeState(stQuit);
        break;
    }
    case evtMUSIC:
    {
        HSM_State *nextState=new HSM_Music(myHsm);
        myHsm->changeState(nextState);
        myHsm->receiveEvent(new_evtSTART(event->msg));
        Config::plugins.display.port=Config::plugins.dvd.port;
        myHsm->displayPlugin.stop(); /* will be restarted with a sigCHLD */

        break;
    }
    case evtMOVIE:
    {
        HSM_State *nextState=new HSM_Movie(myHsm);
        myHsm->changeState(nextState);
        myHsm->receiveEvent(new_evtSTART(event->msg));
        Config::plugins.display.port=Config::plugins.dvd.port;
        myHsm->displayPlugin.stop(); /* will be restarted with a sigCHLD */

        break;
    }
    case evtDVD:
    {
        HSM_State *nextState=new HSM_DVD(myHsm);
        myHsm->changeState(nextState);
        myHsm->receiveEvent(new_evtSTART(event->msg));
        Config::plugins.display.port=Config::plugins.dvd.port;
        myHsm->displayPlugin.stop(); /* will be restarted with a sigCHLD */

        break;
    }
    case evtCD:
    {
        HSM_State *nextState=new HSM_CD(myHsm);
        myHsm->changeState(nextState);
        myHsm->receiveEvent(new_evtSTART(event->msg));
        Config::plugins.display.port=Config::plugins.cd.port;
        myHsm->displayPlugin.stop(); /* will be restarted with a sigCHLD */

        break;
    }
    case evtTIMER:
    {
        break;
    }
    case evtNEXT:
    {
        Event_Next* myEvt=(Event_Next*)event;
        this->hsm->changeState(myEvt->nextState);
        break;
    }
    case evtERROR: {
        if (event->msg != "" ) {
            cerr << event->msg << endl;
        }
        break;
    }
    case evtEXIT:  {
        break;
    }
    default: {
        DBG(cout << "Nothing to do" << endl);
    }
    }
}





/*********
 *
 * MENU
 *
 *********/
void HSM_Menu::processEvent(const Event* event) {
    MyHSM * myHsm=(MyHSM*)this->hsm;
    switch (event->type) {
    case evtENTER:  {
        break;
    }
    case evtSTART: {
        type=toLower(event->msg);
        plugin=new MenuPlugin();
        if (type == "movie") {
            ((MenuPlugin*)plugin)->start(Config::plugins.movieMenu);
        }
        break;
    }
    case evtQUIT: {
        HSM_Quit *stQuit=new HSM_Quit(this->hsm);
        this->hsm->changeState(stQuit);
        break;
    }
    case evtCHILD: {
        EventChild* myEvent=(EventChild*)event;

        if (myEvent->pid == myHsm->inputPlugin.getPID()) {
            DBG(cout << "WARNING: Input plugin died. Restarting it" << endl);
            myHsm->inputPlugin.start(Config::plugins.remote);
        } else if (myEvent->pid == myHsm->displayPlugin.getPID()) {
            DBG(cout << "WARNING: Display plugin died. Restarting it" << endl);
            myHsm->displayPlugin.start(Config::plugins.display);
        } else if (myEvent->pid == plugin->getPID()) {
            DBG(cout << "WARNING: Output plugin died." << endl);
            myHsm->receiveEvent(new_evtSTOP());
        } else {
            DBG(cerr << "ERROR: Unexpected process died: " << myEvent->pid << endl);
        }
        break;
    }
    case evtTIMER:
    {
        break;
    }
    case evtSTOP:
    {
        HSM_Idle *stIdle=new HSM_Idle(this->hsm);
        this->hsm->changeState(stIdle);

        break;
    }
    case evtIDLE:
    case evtMOVIE:
    case evtDVD:
    case evtCD:
    case evtTV:
    case evtDVB:
    {
        /* Wait for the child to die ... */
        /* ... and resend the event */
        HSM_WaitChild *stWait=new HSM_WaitChild(this->hsm,event->clone(),plugin->getPID());
        myHsm->changeState(stWait);
        break;
    }
    case evtMSG:
    {
        if (plugin) {
            plugin->send(event->msg);
        }
        break;
    }
    case evtNEXT: {
        Event_Next* myEvt=(Event_Next*)event;
        this->hsm->changeState(myEvt->nextState);
    }
    case evtERROR: {
        if (event->msg != "" ) {
            cerr << event->msg << endl;
        }
    }
    case evtEXIT:  {
        break;
    }
    default: {
        DBG(cout << "Nothing to do" << endl);
    }
    }
}




/*********
 *
 * MUSIC
 *
 *********/
void HSM_Music::processEvent(const Event* event) {
    MyHSM * myHsm=(MyHSM*)this->hsm;
    switch (event->type) {
    case evtENTER:  {
        break;
    }
    case evtSTART: {
        plugin=new OutputPlugin();
        ((OutputPlugin*)plugin)->start(Config::plugins.music);
        break;
    }
    case evtQUIT: {
        HSM_Quit *stQuit=new HSM_Quit(this->hsm);
        this->hsm->changeState(stQuit);
        break;
    }
    case evtCHILD: {
        EventChild* myEvent=(EventChild*)event;

        if (myEvent->pid == myHsm->inputPlugin.getPID()) {
            DBG(cout << "WARNING: Input plugin died. Restarting it" << endl);
            myHsm->inputPlugin.start(Config::plugins.remote);
        } else if (myEvent->pid == myHsm->displayPlugin.getPID()) {
            DBG(cout << "WARNING: Display plugin died. Restarting it" << endl);
            myHsm->displayPlugin.start(Config::plugins.display);
        } else if (myEvent->pid == plugin->getPID()) {
            DBG(cout << "WARNING: Output plugin died." << endl);
            myHsm->receiveEvent(new_evtSTOP());
        }
        break;
    }
    case evtTIMER:
    {
        break;
    }
    case evtSTOP:
    {
        HSM_Idle *stIdle=new HSM_Idle(this->hsm);
        this->hsm->changeState(stIdle);

        break;
    }
    case evtIDLE:
    case evtMOVIE:
    case evtDVD:
    case evtCD:
    case evtTV:
    case evtDVB:
    {
        /* Wait for the child to die ... */
        /* ... and resend the event */
        HSM_WaitChild *stWait=new HSM_WaitChild(this->hsm,event->clone(),plugin->getPID());
        myHsm->changeState(stWait);
        break;
    }
    case evtMSG:
    {
        if (plugin) {
            plugin->send(event->msg);
        }
        break;
    }
    case evtNEXT: {
        Event_Next* myEvt=(Event_Next*)event;
        this->hsm->changeState(myEvt->nextState);
    }
    case evtERROR: {
        if (event->msg != "" ) {
            cerr << event->msg << endl;
        }
    }
    case evtEXIT:  {
        break;
    }
    default: {
        DBG(cout << "Nothing to do" << endl);
    }
    }
}


/*********
 *
 * DVD
 *
 *********/
void HSM_DVD::processEvent(const Event* event) {
    MyHSM * myHsm=(MyHSM*)this->hsm;
    switch (event->type) {
    case evtENTER:  {
        break;
    }
    case evtSTART: {
        plugin=new OutputPlugin();
        ((OutputPlugin*)plugin)->start(Config::plugins.dvd);
        break;
    }
    case evtQUIT: {
        HSM_Quit *stQuit=new HSM_Quit(this->hsm);
        this->hsm->changeState(stQuit);
        break;
    }
    case evtCHILD: {
        EventChild* myEvent=(EventChild*)event;

        if (myEvent->pid == myHsm->inputPlugin.getPID()) {
            DBG(cout << "WARNING: Input plugin died. Restarting it" << endl);
            myHsm->inputPlugin.start(Config::plugins.remote);
        } else if (myEvent->pid == myHsm->displayPlugin.getPID()) {
            DBG(cout << "WARNING: Display plugin died. Restarting it" << endl);
            myHsm->displayPlugin.start(Config::plugins.display);
        } else if (myEvent->pid == plugin->getPID()) {
            DBG(cout << "WARNING: Output plugin died." << endl);
            myHsm->receiveEvent(new_evtSTOP());
        }
        break;
    }
    case evtSTOP:
    {
        HSM_Idle *stIdle=new HSM_Idle(this->hsm);
        this->hsm->changeState(stIdle);

        break;
    }
    case evtIDLE:
    case evtMOVIE:
    case evtMUSIC:
    case evtCD:
    case evtTV:
    case evtDVB:
    {
        /* Wait for the child to die ... */
        /* ... and resend the event */
        HSM_WaitChild *stWait=new HSM_WaitChild(this->hsm,event->clone(),plugin->getPID());
        myHsm->changeState(stWait);
        break;
    }
    case evtMSG:
    {
        if (plugin) {
            plugin->send(event->msg);
        }
        break;
    }
    case evtTIMER:
    {
        break;
    }
    case evtNEXT: {
        Event_Next* myEvt=(Event_Next*)event;
        this->hsm->changeState(myEvt->nextState);
    }
    case evtERROR: {
        if (event->msg != "" ) {
            cerr << event->msg << endl;
        }
    }
    case evtEXIT:  {
        break;
    }
    default: {
        DBG(cout << "Nothing to do" << endl);
    }
    }
}



/*********
 *
 * CD
 *
 *********/
void HSM_CD::processEvent(const Event* event) {
    MyHSM * myHsm=(MyHSM*)this->hsm;
    switch (event->type) {
    case evtENTER:  {
        break;
    }
    case evtSTART: {
        plugin=new OutputPlugin();
        ((OutputPlugin*)plugin)->start(Config::plugins.cd);
        break;
    }
    case evtQUIT: {
        HSM_Quit *stQuit=new HSM_Quit(this->hsm);
        this->hsm->changeState(stQuit);
        break;
    }
    case evtCHILD: {
        EventChild* myEvent=(EventChild*)event;

        if (myEvent->pid == myHsm->inputPlugin.getPID()) {
            DBG(cout << "WARNING: Input plugin died. Restarting it" << endl);
            myHsm->inputPlugin.start(Config::plugins.remote);
        } else if (myEvent->pid == myHsm->displayPlugin.getPID()) {
            DBG(cout << "WARNING: Display plugin died. Restarting it" << endl);
            myHsm->displayPlugin.start(Config::plugins.display);
        } else if (myEvent->pid == plugin->getPID()) {
            DBG(cout << "WARNING: Output plugin died." << endl);
            myHsm->receiveEvent(new_evtSTOP());
        }
        break;
    }
    case evtSTOP:
    {
        HSM_Idle *stIdle=new HSM_Idle(this->hsm);
        this->hsm->changeState(stIdle);

        break;
    }
    case evtIDLE:
    case evtMOVIE:
    case evtMUSIC:
    case evtCD:
    case evtTV:
    case evtDVB:
    {
        /* Wait for the child to die ... */
        /* ... and resend the event */
        HSM_WaitChild *stWait=new HSM_WaitChild(this->hsm,event->clone(),plugin->getPID());
        myHsm->changeState(stWait);
        break;
    }
    case evtMSG:
    {
        if (plugin) {
            plugin->send(event->msg);
        }
        break;
    }
    case evtTIMER:
    {
        break;
    }
    case evtNEXT: {
        Event_Next* myEvt=(Event_Next*)event;
        this->hsm->changeState(myEvt->nextState);
    }
    case evtERROR: {
        if (event->msg != "" ) {
            cerr << event->msg << endl;
        }
    }
    case evtEXIT:  {
        break;
    }
    default: {
        DBG(cout << "Nothing to do" << endl);
    }
    }
}



/*********
 *
 * MOVIE
 *
 *********/
void HSM_Movie::processEvent(const Event* event) {
    MyHSM * myHsm=(MyHSM*)this->hsm;
    switch (event->type) {
    case evtENTER:  {
        break;
    }
    case evtSTART: {
        if (event->msg == "") {
            //        plugin=new MenuPlugin();
            //        ((MenuPlugin*)plugin)->start(Config::plugins.movieMenu);
            HSM_Menu *stMenu=new HSM_Menu(this->hsm);
            myHsm->changeState(stMenu);
            myHsm->receiveEvent(new_evtSTART("movie"));
        } else {
            plugin=new OutputPlugin();
            Config::plugins.movie.file=event->msg;
            ((OutputPlugin*)plugin)->start(Config::plugins.movie);
        }
        break;
    }
    case evtQUIT: {
        HSM_Quit *stQuit=new HSM_Quit(this->hsm);
        this->hsm->changeState(stQuit);
        break;
    }
    case evtCHILD: {
        EventChild* myEvent=(EventChild*)event;

        if (myEvent->pid == myHsm->inputPlugin.getPID()) {
            DBG(cout << "WARNING: Input plugin died. Restarting it" << endl);
            myHsm->inputPlugin.start(Config::plugins.remote);
        } else if (myEvent->pid == myHsm->displayPlugin.getPID()) {
            DBG(cout << "WARNING: Display plugin died. Restarting it" << endl);
            myHsm->displayPlugin.start(Config::plugins.display);
        } else if (myEvent->pid == plugin->getPID()) {
            DBG(cout << "WARNING: Output plugin died." << endl);
            myHsm->receiveEvent(new_evtSTOP());
        }
        break;
    }
    case evtSTOP:
    {
        HSM_Idle *stIdle=new HSM_Idle(this->hsm);
        this->hsm->changeState(stIdle);

        break;
    }
    case evtIDLE:
    case evtMUSIC:
    case evtMOVIE:
    case evtDVD:
    case evtCD:
    case evtTV:
    case evtDVB:
    {
        /* Wait for the child to die ... */
        /* ... and resend the event */
        HSM_WaitChild *stWait=new HSM_WaitChild(this->hsm,event->clone(),plugin->getPID());
        myHsm->changeState(stWait);
        break;
    }
    case evtMSG:
    {
        if (plugin) {
            plugin->send(event->msg);
        }
        break;
    }
    case evtTIMER:
    {
        break;
    }
    case evtNEXT: {
        Event_Next* myEvt=(Event_Next*)event;
        myHsm->changeState(myEvt->nextState);
    }
    case evtERROR: {
        if (event->msg != "" ) {
            cerr << event->msg << endl;
        }
    }
    case evtEXIT:  {
        break;
    }
    default: {
        DBG(cout << "Nothing to do" << endl);
    }
    }
}


void parseCommand(HSM* hsm, string command, string parameter) {
    string myCommand=toLower(command);

    if (myCommand == "quit") {
        hsm->receiveEvent(new_evtQUIT());
    } else if (myCommand == "music") {
        hsm->receiveEvent(new_evtMUSIC(parameter));
    } else if (myCommand == "movie") {
        hsm->receiveEvent(new_evtMOVIE(parameter));
    } else if (myCommand == "dvd") {
        hsm->receiveEvent(new_evtDVD(parameter));
    } else if (myCommand == "cd") {
        hsm->receiveEvent(new_evtCD(parameter));
    } else if (myCommand == "tv") {
        hsm->receiveEvent(new_evtTV(parameter));
    } else if (myCommand == "dvb") {
        hsm->receiveEvent(new_evtDVB(parameter));
    } else if (myCommand == "void") {
        hsm->receiveEvent(new_evtIDLE());
    } else if (myCommand == "chup") {
        hsm->receiveEvent(new_evtMSG("Channel Up"));
    } else if (myCommand == "chdown") {
        hsm->receiveEvent(new_evtDVB("Channel Down"));
    } else {
        string strtmp=command;
        if (parameter != "") {
            strtmp+=" " + parameter;
        }
        hsm->receiveEvent(new_evtMSG(strtmp));
    }
}



int main(int argc, char* argv[]) {
    int sock;
    fd_set sockSelect;
    int port=10101;
    struct sockaddr_in local,remote;
    socklen_t remoteSize=sizeof(remote);
    string configPath="";

    //remove leading path from program name.
    string programName=argv[0];
    int pos=programName.rfind("/");
    if (pos!=string::npos) {
        programName=programName.substr(pos+1);
    }

    const char * basename=programName.c_str();

    int optc=0;
    while ((optc=getopt(argc,argv,"vp:c:")) != -1 ) {
        switch (optc) {
        case 'v': { // version
            cout << PACKAGE_NAME << " version " << VERSION << endl;
            cout << "Copyright (c) Guillem Pages Gassull 2009." << endl;
            exit(0);
            break;
        }
        case 'p': { // port
            port=atoi(optarg);
            cout << "Using non default port " << port << endl;
            break;
        }
        case 'c': { // config file
            configPath=optarg;
            cout << "Reading configuration from " << configPath << endl;
            break;
        }
        default:
            //      usage(basename);
            exit(-1);
        }
    }

    configInit(configPath);
    Config::plugins.movieMenu.remotePort=port;
    Config::plugins.remote.port=port;

    DBG(cout << Config::plugins << endl);

    hsm=new MyHSM();

    HSM_Start * st_Start=new HSM_Start(hsm);
    hsm->setStartState(st_Start);

    //signal handlers
    signal(SIGCHLD, clavaEstaca);
    signal(SIGINT, term);
    signal(SIGTERM, term);


    FD_ZERO(&sockSelect);

    //variables for reading the commands
    string command;
    string parameter;
    char code[512];
    int ret;
    struct timeval timeout;
    int error=0;
    timeout.tv_sec=1;
    timeout.tv_usec=200000; //5Hz should be enough.

    sock=socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror(argv[0]);
        hsm->receiveEvent(new_evtQUIT("Socket create"));
    } else {

        local.sin_family=remote.sin_family=AF_INET;
        local.sin_addr.s_addr=INADDR_ANY;
        inet_aton("127.0.0.1",&remote.sin_addr);
        local.sin_port=htons(port);

        if (bind(sock, (struct sockaddr *)&local, sizeof(local)) <0 ) {
            perror(argv[0]);
            hsm->receiveEvent(new_evtQUIT("Bind error"));
        } else {

            FD_SET(sock,&sockSelect);

            //make socket non-blocking
            fcntl(sock, F_SETFL, (fcntl(sock, F_GETFL) | O_NONBLOCK));

            hsm->receiveEvent(new_evtNEXT(NULL));
        } // bind
    } // socket create

    while (hsm) { /* Endless loop; Program exits through the Exit state in the HSM */

        error=select(sock+1,&sockSelect,&sockSelect,&sockSelect,&timeout);
        if (error<0) { //nothing received...
            if (error <0) { // error
                perror("select");
            }
            timeout.tv_sec=1;
            timeout.tv_usec=200000; //5Hz should be enough.
        } else {
            if (error == 0) {  //timer expired
                hsm->receiveEvent(new_evtTIMER(),1);
            }
            error=recv(sock,code,512,0);
            if (error<=0) {
                if (errno == EAGAIN) { // Nothing to read...
                    usleep(100000); //Sleep for 0.1 seconds
                } else {
                    perror(argv[0]);
                }
            } else {
                if (code[error-1]=='\n') {
                    code[error-1]=0;
                }
                command=code;
                parameter="";
                if (command.find(' ')!=string::npos) {
                    command=command.substr(0,command.find(' '));
                    parameter=code+command.length()+1;
                }
                DBG(cout << "Received " << command << endl;)

                        parseCommand(hsm,command,parameter);
            }
        }
    }

    cout << "Goodbye!" << endl;

    return 0;
}


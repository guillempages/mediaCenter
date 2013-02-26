#include "defines.h"

#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "config.h"

#include "utils.h"
#include "menuPlugin.h"

using Config::plugins;
using std::string;

MenuPlugin::MenuPlugin() : ::Plugin() {
    DBG(std::cout << "Constructing MenuPlugin" << std::endl;)
}

MenuPlugin::~MenuPlugin() {
}

bool MenuPlugin::start(const Config::MenuPluginConf & plugin) {
    if (PID_)
        stop();


    port_=plugin.port;
    char port[10];
    const char * argv[12];
    sprintf(port,"%i",port_);
    char remotePort[10];
    sprintf(remotePort,"%i",plugin.remotePort);

    string basename=plugin.path;

    int pos=basename.rfind("/");
    if (pos != string::npos) {
        basename=basename.substr(pos+1);
    }
    int i=0;

    argv[i++]=basename.c_str();
    argv[i++]="-p";
    argv[i++]=port;
    argv[i++]="-t";
    argv[i++]=plugin.type.c_str();
    if (plugin.file!="") {
        argv[i++]="-f";
        argv[i++]=plugin.file.c_str();
    }
    argv[i++]="-s";
    argv[i++]=plugin.server.c_str();
    argv[i++]="-r";
    argv[i++]=remotePort;
    argv[i++]=NULL;

    DBG(std::cout << "Starting " << plugin.path << std::endl;)

            PID_=exec(plugin.path,argv);
    if (PID_<=0) {
        perror("MenuPlugin");
        return false;
    }

    return true;
}

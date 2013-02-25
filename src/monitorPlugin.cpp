#include "defines.h"
#include "config.h"
#include "monitorPlugin.h"

#include <string>
#include <iostream>
#include <stdio.h>

using Config::plugins;
using std::string;

MonitorPlugin::MonitorPlugin() : ::Plugin() {
}

MonitorPlugin::~MonitorPlugin() {
}

bool MonitorPlugin::start(const Config::MonitorPluginConf& plugin) {

    if (PID_)
        stop();

    port_=plugin.port;

    char port[10];
    const char * argv[10];
    sprintf(port,"%i",port_);

    std::string basename=plugin.path;

    int pos=basename.rfind("/");
    if (pos!=string::npos) {
        basename=basename.substr(pos+1);
    }
    int i=0;
    argv[i++]=basename.c_str();
    argv[i++]="-d";
    argv[i++]=plugin.device.c_str();
    argv[i++]="-m";
    argv[i++]=plugin.mountPoint.c_str();
    argv[i++]="-s";
    argv[i++]=plugin.server.c_str();
    argv[i++]="-p";
    argv[i++]=port;
    argv[i++]=NULL;

    PID_=exec(plugin.path,argv);
    if (PID_<=0) {
        perror("MonitorPlugin");
        return false;
    }

    return true;
}

#include "defines.h"
#include "config.h"
#include "displayPlugin.h"


#include <string>
#include <stdio.h>
#include <unistd.h>

using Config::plugins;
using std::string;

DisplayPlugin::DisplayPlugin() : ::Plugin() {
}

DisplayPlugin::~DisplayPlugin() {
}

bool DisplayPlugin::start(const Config::DisplayPluginConf& plugin) {

    if (PID_)
        stop();

    port_=plugin.port;

    char port[10];
    const char * argv[6];
    sprintf(port,"%i",port_);

    std::string basename=plugin.path;

    int pos=basename.rfind("/");
    if (pos!=string::npos) {
        basename=basename.substr(pos+1);
    }
    argv[0]=basename.c_str();
    argv[1]="-s";
    argv[2]=plugin.server.c_str();
    argv[3]="-p";
    argv[4]=port;
    argv[5]=NULL;


    PID_=exec(plugin.path,argv);
    if (PID_<=0) {
        perror("DisplayPlugin");
        return false;
    }

    return true;
}

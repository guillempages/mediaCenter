#include "defines.h"

#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "config.h"
#include "utils.h"
#include "outputPlugin.h"

using Config::plugins;
using std::string;

OutputPlugin::OutputPlugin() :
        ::Plugin() {
    DBG(std::cout << "Constructing OutputPlugin" << std::endl;)
}

OutputPlugin::~OutputPlugin() {
}

bool OutputPlugin::start(const Config::OutputPluginConf & plugin) {
    DBG(std::cout << "Starting OutputPlugin" << std::endl);
    if (PID_)
        stop();

    port_ = plugin.port;
    char port[10];
    const char * argv[10];
    sprintf(port, "%i", port_);

    string basename = plugin.path;

    int pos = basename.rfind("/");
    if (pos != string::npos) {
        basename = basename.substr(pos + 1);
    }
    int i = 0;

    argv[i++] = basename.c_str();
    argv[i++] = "-p";
    argv[i++] = port;
    argv[i++] = "-t";
    argv[i++] = plugin.type.c_str();
    if (plugin.file != "") {
        argv[i++] = "-f";
        argv[i++] = plugin.file.c_str();
    }
    if (plugin.configFile != "") {
        argv[i++] = "-c";
        argv[i++] = plugin.configFile.c_str();
    }
    argv[i++] = NULL;

    DBG(std::cout << "Starting " << plugin.path << std::endl;)

    PID_ = exec(plugin.path, argv);
    if (PID_ <= 0) {
        perror("OutputPlugin");
        return false;
    }

    return true;
}

bool OutputPlugin::stop() {
    DBG(std::cout << "Stopping OutputPlugin" << std::endl);
    send("Quit");
}

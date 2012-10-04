#include "defines.h"
#include "config.h"
#include "remotePlugin.h"

#include <string>
#include <iostream>
#include <stdio.h>

using Config::plugins;
using std::string;

RemotePlugin::RemotePlugin() : ::Plugin() {
}

RemotePlugin::~RemotePlugin() {
}

bool RemotePlugin::start(const Config::RemotePluginConf& plugin) {

  if (PID_) 
    stop();
  
  port_=plugin.port;

  char port[10];
  const char * argv[8];
  sprintf(port,"%i",port_);
  
  std::string basename=plugin.path;

  int pos=basename.rfind("/");
  if (pos!=string::npos) {
    basename=basename.substr(pos+1);
  }
  argv[0]=basename.c_str();
  argv[1]="-n";
  argv[2]=plugin.name.c_str();
  argv[3]="-s";
  argv[4]=plugin.server.c_str();
  argv[5]="-p";
  argv[6]=port;
  argv[7]=NULL;

  PID_=exec(plugin.path,argv);
  if (PID_<=0) {
    perror("RemotePlugin");
    return false;
  }

  return true;
}

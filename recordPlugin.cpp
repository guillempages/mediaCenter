#include "config.h"
#include "recordPlugin.h"

//#include <sys/types.h>
//#include <unistd.h>

#include <string>
#include <stdio.h>

using Config::plugins;
using std::string;

RecordPlugin::RecordPlugin() : ::Plugin() {
}

RecordPlugin::~RecordPlugin() {
}

bool RecordPlugin::start(const Config::RecordPluginConf& plugin) {

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
  int i=0;
  argv[i++]=basename.c_str();
  argv[i++]="-p";
  argv[i++]=port;
  argv[i++]="-f";
  argv[i++]=plugin.file.c_str();
  argv[i++]=NULL;


  PID_=exec(plugin.path,argv);
  if (PID_<=0) {
    perror("RecordPlugin");
    return false;
  }

  return true;
}

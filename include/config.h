#ifndef MEDIACENTER_CONFIG_H
#define MEDIACENTER_CONFIG_H

#include <string>
#include <iostream>
#include "utils.h"

namespace Config {

class PluginConf {
 public:
  PluginConf(const std::string& type="", const std::string& path="", const std::string& configFile="");

  std::string class_;
  std::string type;
  std::string path;
  std::string configFile;
  int port;

};

class RemotePluginConf : public PluginConf {
 public:
  RemotePluginConf(const std::string& type="remote", const std::string& path="mediaCenter_lirc", const std::string & name="", const std::string & server="localhost");
  std::string name;
  std::string server;
};

class OutputPluginConf : public PluginConf {
 public:
  OutputPluginConf(const std::string& type="output", const std::string& path="mediaCenter_output",const std::string & file="");
  std::string file;
};

class DisplayPluginConf : public PluginConf {
 public:
  DisplayPluginConf(const std::string& type="display", const std::string& path="mediaCenter_display", const std::string & server="localhost");
  std::string server;
};

class MenuPluginConf : public OutputPluginConf {
 public:
  MenuPluginConf(const std::string& type="menu", const std::string& path="mediaCenter_menu", const std::string & file="", const std::string & server="localhost", int remotePort=10010);
  std::string server;
  int remotePort;
};

class RecordPluginConf : public PluginConf {
 public:
  RecordPluginConf(const std::string& type="record", const std::string& path="mediaCenter_record", const std::string & file="", const std::string & format= "mpg");
  std::string file;
  std::string format;
};

class Plugins {
 public:
  RemotePluginConf remote;
  DisplayPluginConf display;
  OutputPluginConf dvd;
  OutputPluginConf cd;
  OutputPluginConf tv;
  OutputPluginConf dvb;
  OutputPluginConf movie;
  MenuPluginConf movieMenu;
  OutputPluginConf music;
  RecordPluginConf record;
  
};


//extern Paths paths;
extern Plugins plugins;
}; //namespace Config

std::ostream & operator<<(std::ostream& ostr, const Config::PluginConf & plugin);
std::ostream & operator<<(std::ostream& ostr, const Config::Plugins & plugins);

//Config::Paths & getPaths();
Config::Plugins & getPlugins();

void configInit();

#endif

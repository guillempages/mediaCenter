#ifndef BASE_PLUGIN_H
#define BASE_PLUGIN_H

#include <string>
#include <sys/types.h>
#include <signal.h>

#include "config.h"

namespace Config {
  class PluginConf;
};

class Plugin {

 public:
  Plugin();
  virtual ~Plugin();

  virtual bool start(const Config::PluginConf& plugin);
  virtual bool stop();


  int getPID() const;
  void setPort(int port);
  int getPort() const;
  int send(const std::string& msg) const;
  int alive();

 protected:
  int PID_;
  int port_;
  int exec(const std::string &prog,const char ** argv);
};

inline Plugin::Plugin() : PID_(-1),port_(0) {
}

inline Plugin::~Plugin() {
  stop();
}

inline int Plugin::getPID() const {
  return PID_;
}

inline bool Plugin::start(const Config::PluginConf&) {
  return true;
}

inline void Plugin::setPort(int _port) {
  port_=_port;
}

inline int Plugin::getPort() const {
  return port_;
}

#endif


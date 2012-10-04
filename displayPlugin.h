#ifndef DISPLAY_PLUGIN_H
#define DISPLAY_PLUGIN_H

#include "basePlugin.h"

namespace Config {
  class DisplayPluginConf;
}

class DisplayPlugin : public Plugin {

  public:
    DisplayPlugin();
    ~DisplayPlugin();

   virtual bool start(const Config::DisplayPluginConf & plugin);

  protected:
};

#endif

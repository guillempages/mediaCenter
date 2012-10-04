#ifndef MENU_PLUGIN_H
#define MENU_PLUGIN_H

#include "basePlugin.h"

namespace Config {
  class MenuPluginConf;
}

class MenuPlugin : public Plugin {

  public:
    MenuPlugin();
    ~MenuPlugin();

   virtual bool start(const Config::MenuPluginConf & plugin);

  protected:
};

#endif

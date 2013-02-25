#ifndef MONITOR_PLUGIN_H
#define MONITOR_PLUGIN_H

#include "basePlugin.h"

namespace Config {
class MonitorPluginConf;
}

class MonitorPlugin : public Plugin {

public:
    MonitorPlugin();
    ~MonitorPlugin();

    virtual bool start(const Config::MonitorPluginConf & plugin);

protected:
};

#endif


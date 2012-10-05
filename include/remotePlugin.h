#ifndef REMOTE_PLUGIN_H
#define REMOTE_PLUGIN_H

#include "basePlugin.h"

namespace Config {
class RemotePluginConf;
}

class RemotePlugin : public Plugin {

public:
    RemotePlugin();
    ~RemotePlugin();

    virtual bool start(const Config::RemotePluginConf & plugin);

protected:
};

#endif


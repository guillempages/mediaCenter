#ifndef RECORD_PLUGIN_H
#define RECORD_PLUGIN_H

#include "basePlugin.h"

namespace Config {
class RecordPluginConf;
}

class RecordPlugin: public Plugin {

public:
    RecordPlugin();
    ~RecordPlugin();

    virtual bool start(const Config::RecordPluginConf & plugin);

protected:
};

#endif

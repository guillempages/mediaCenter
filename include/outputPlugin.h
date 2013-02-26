#ifndef OUTPUT_PLUGIN_H
#define OUTPUT_PLUGIN_H

#include "basePlugin.h"

namespace Config {
class OutputPluginConf;
}

class OutputPlugin: public Plugin {

public:
    OutputPlugin();
    ~OutputPlugin();

    virtual bool start(const Config::OutputPluginConf & plugin);
    virtual bool stop();

protected:
};

#endif

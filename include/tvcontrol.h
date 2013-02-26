#include <string>
#include <vector>

class Channel {
public:
    std::string name;
    int frequency;
};

typedef std::vector<Channel> Channels;

class TV {

public:
    TV();

    static int setFrequency(int frequency, const char *device = "");
    static int getFrequency(const char *device = "");

    static int setChannel(const char* channelName, const char *device = "");
    static int setChannel(int channel, const char *device = "");
    static const char* getChannel(int channelFrequency, int & channelNum = dummy);
    static const char* getCurrentChannel(const char *device = "", int & channelNum = dummy);
    static int getCurrentChannelNum(const char *device = "");

    static void initChannels(const std::string& channelFile);

protected:
    static Channels channels_;
    static int dummy;
};


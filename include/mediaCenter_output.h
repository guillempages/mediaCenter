#ifndef mediaCenter_stub_h
#define mediaCenter_stub_h

#include <string>
#include <signal.h>

extern bool end;

void term(int result=0);

class MediaCenter_Output {
public:
  MediaCenter_Output();

  ~MediaCenter_Output();

  int startApplication(); 

  std::string getTitle();
  std::string getArtist();

  std::string getChannel(); //only for TV; but must be implemented anyway
  std::string setChannel(const std::string & newChannel);
  std::string channelUp(int step=1);
  std::string channelDown(int step=1);

  int getTime();
  int getTotalTime();
  int getTrack();
  int getTotalTracks();
  int getChapter();
  int getTotalChapters();


  bool isPaused();
  /**
   * Implemented inline below
   */
  std::string getType();
  void setType(const std::string& type);
  std::string getFilename();
  std::string getConfig();
  void setConfig(const std::string& type);
  void setFilename(const std::string& type);

  int getPID();

  void stop();

protected:
  /**
   * Implemented in stub.cpp
   */
  int exec(const std::string & program, const char * argv[]);

  std::string type_;
  std::string filename_;
  std::string config_;
  int pid_;

  void * data; //user data. Use at own discretion.
};

inline std::string MediaCenter_Output::getType() {
  return type_;
}
inline void MediaCenter_Output::setType(const std::string& value) {
  type_=value;
}

inline std::string MediaCenter_Output::getFilename() {
  return filename_;
}
inline void MediaCenter_Output::setFilename(const std::string& value) {
  filename_=value;
}

inline std::string MediaCenter_Output::getConfig() {
  return config_;
}
inline void MediaCenter_Output::setConfig(const std::string& value) {
  config_=value;
}

inline int MediaCenter_Output::getPID() {
  return pid_;
}

inline void MediaCenter_Output::stop() {
  if (pid_ > 0) {
    kill(pid_,SIGTERM);
  }
}

#endif


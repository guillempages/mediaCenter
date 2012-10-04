#ifndef mediaCenter_output_h
#define mediaCenter_output_h

#include <string>
#include <signal.h>

extern bool end;

void term(int result=0);

class MediaCenter_output {
public:
  MediaCenter_output();

  virtual ~MediaCenter_output();

  virtual int startApplication(); 

  virtual std::string getTitle();
  virtual std::string getArtist();

  virtual std::string getChannel(); //only for TV; but must be implemented anyway
  virtual std::string setChannel(const std::string & newChannel);
  virtual std::string channelUp(int step=1);
  virtual std::string channelDown(int step=1);

  virtual int getTime();
  virtual int getTotalTime();
  virtual int getTrack();
  virtual int getTotalTracks();
  virtual int getChapter();
  virtual int getTotalChapters();


  virtual void play() {};
  virtual void pause() {};
  virtual void stop() {};
  virtual void next() {};
  virtual void previous() {};

  virtual bool isPaused();

  virtual void exit();
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

inline std::string MediaCenter_output::getType() {
  return type_;
}
inline void MediaCenter_output::setType(const std::string& value) {
  type_=value;
}

inline std::string MediaCenter_output::getFilename() {
  return filename_;
}
inline void MediaCenter_output::setFilename(const std::string& value) {
  filename_=value;
}

inline std::string MediaCenter_output::getConfig() {
  return config_;
}
inline void MediaCenter_output::setConfig(const std::string& value) {
  config_=value;
}

inline int MediaCenter_output::getPID() {
  return pid_;
}

inline int MediaCenter_output::startApplication () {};

extern MediaCenter_output* newApp();

#endif


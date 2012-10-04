#include <iostream>
#include "tvcontrol.h"
#include "utils.h"

#define DEBUG

int main(int argc, char* argv[]) {
  TV::initChannels("/home/guillem/.mediaCenter/channels");
  
  if (argc<2) {
    TV::setChannel("ARD","/dev/video0");
  } else {
    TV::setChannel(argv[1],"/dev/video0");
  }

  std::cout << "Channel: " << TV::getCurrentChannel("/dev/video0") << std::endl;
  
}

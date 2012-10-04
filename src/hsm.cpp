#include "defines.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

#include "hsm.h"
#define DEBUG
#include "utils.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;


Event::Event(int _type, std::string _name, std::string _msg) : 
                    type(_type), name(_name), msg(_msg) {}

Event* Event::clone() const {
  return new Event(type,name,msg);
}

Event_Next::Event_Next(int _type, std::string _name, HSM_State* _nextState, std::string _msg) : 
                              Event(_type,_name,_msg), nextState(_nextState) {}

Event* Event_Next::clone() const {
  return new Event_Next(type,name,nextState,msg);
}



HSM_State::HSM_State(HSM* _hsm,const string &_name) : hsm(_hsm),name_(_name) {};

HSM_State::~HSM_State() {};

void HSM_State::receiveEvent(Event *event,bool silent) {
  if (!event) {
    if (!silent) {
      cerr << "ERROR: Received a NULL event in state >>" << name() << "<<" <<  endl;
    }
    return;
  }
  
  if (!silent) {
    DBG(
      cout << "INFO: State >>" << name() << "<< received event >>" << event->name << "<<";
      if (event->msg != "") {
        cout << " with message: <<" << event->msg << ">>";
      }
      cout << endl;
    );
  }
  
  this->processEvent(event);
  delete event;
}





HSM::HSM() : state(NULL) {};

HSM::~HSM() { delete state; };

void HSM::setStartState(HSM_State * start) {
  if (state) {
    delete state;
  }
  state=start;
  receiveEvent(new_evtENTER());
}


void HSM::receiveEvent(Event *event,bool silent) {
  if (state) {
     state->receiveEvent(event,silent);
  } else {
    cerr << "FATAL Error: State machine in wrong state" << endl;
    exit(-1);
  } 
}

void HSM::changeState(HSM_State *newState) {
  DBG(cout << "Changing from state >>" << state->name() << "<< to >>" << newState->name() << "<<" << endl);
  receiveEvent(new_evtEXIT());
  if (state) {
     delete state;
  }
  state=newState;
  receiveEvent(new_evtENTER());
}






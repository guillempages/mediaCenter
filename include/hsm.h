#ifndef HSM_H
#define HSM_H

/* forward declarations */
class HSM;
class HSM_State;
class Event;

/**
  * You should define an enum Events in your application,
  * to easily reference the different event types
  * IMPORTANT:
  * keep evtENTER and evtEXIT with it current numbers, as they are used both
  * internally and externally in the HSM
  * that means, start your event definitions with 0 or above
  */
enum HSM_Events {
    evtENTER=-2,
    evtEXIT=-1
};



class Event {
public:
    Event(int type, std::string name, std::string msg="");

    virtual Event* clone() const;

    int type;
    std::string name;
    std::string msg;
};

class Event_Next : public Event {
public:
    Event_Next(int type, std::string name, HSM_State * nextState, std::string msg="");
    virtual Event* clone() const;
    
    HSM_State * nextState;
};

inline Event* new_evtENTER(std::string msg="") {
    return new Event(evtENTER, "evtENTER",msg);
}
inline Event* new_evtEXIT(std::string msg="") {
    return new Event(evtEXIT, "evtEXIT",msg);
}



/**
 * Base class for the State Machine states
 */
class HSM_State {

public:
    HSM_State(HSM* hsm,const std::string &name);
    virtual ~HSM_State();

    void receiveEvent(Event *event,bool silent=false);

    inline std::string name() {return name_;};

private:
    virtual void processEvent(const Event *event)=0;

protected:
    HSM * hsm;
    std::string name_;
};



class HSM {

public:
    HSM();
    ~HSM();
    void receiveEvent(Event *event,bool silent=false);

    /**
     * start is a pointer to a HSM_state, where the State machine starts.
     * the state machine takes ownership of the pointer
     */
    void setStartState(HSM_State *start);
    /**
     * newState is a pointer to the next HSM_state.
     * the state machine takes ownership of the pointer
     */
    void changeState(HSM_State *newState);
    
private:
    HSM_State *state;
};
















#endif

#ifndef MEDIACENTER_EVENTS_H
#define MEDIACENTER_EVENTS_H

enum Events {
    evtSTART,
    evtSTOP,
    evtQUIT,
    evtERROR,
    evtTIMER,
    evtCHILD,
    evtNEXT,
    evtMSG,

    evtIDLE,
    evtDVD,
    evtCD,
    evtTV,
    evtDVB,
    evtMUSIC,
    evtMOVIE,
};

class EventChild : public Event {
public:
    EventChild(int type, std::string name, int pid, std::string msg="");
    
    int pid;
};

inline EventChild::EventChild(int _type, std::string _name, int _pid, std::string _msg) :
    Event(_type, _name, _msg), pid(_pid)
{
}

inline EventChild* new_evtCHILD(int pid, std::string msg="") {
    return new EventChild(evtCHILD, "evtCHILD",pid, msg);
}

inline Event* new_evtSTART(std::string msg="") {
    return new Event(evtSTART, "evtSTART",msg);
}
inline Event* new_evtSTOP(std::string msg="") {
    return new Event(evtSTOP, "evtSTOP",msg);
}
inline Event* new_evtQUIT(std::string msg="") {
    return new Event(evtQUIT, "evtQUIT",msg);
}
inline Event* new_evtERROR(std::string msg="") {
    return new Event(evtERROR, "evtERROR",msg);
}
inline Event* new_evtTIMER(std::string msg="") {
    return new Event(evtTIMER, "evtTIMER",msg);
}
inline Event* new_evtNEXT(HSM_State* nextState, std::string msg="") {
    return new Event_Next(evtNEXT, "evtNEXT", nextState, msg);
}
inline Event* new_evtMSG(std::string msg="") {
    return new Event(evtMSG, "evtMSG",msg);
}
inline Event* new_evtIDLE(std::string msg="") {
    return new Event(evtIDLE, "evtIDLE",msg);
}

inline Event* new_evtDVD(std::string msg="") {
    return new Event(evtDVD, "evtDVD",msg);
}
inline Event* new_evtCD(std::string msg="") {
    return new Event(evtCD, "evtCD",msg);
}
inline Event* new_evtTV(std::string msg="") {
    return new Event(evtTV, "evtTV",msg);
}
inline Event* new_evtDVB(std::string msg="") {
    return new Event(evtDVB, "evtDVB",msg);
}
inline Event* new_evtMUSIC(std::string msg="") {
    return new Event(evtMUSIC, "evtMUSIC",msg);
}
inline Event* new_evtMOVIE(std::string msg="") {
    return new Event(evtMOVIE, "evtMOVIE",msg);
}


#endif

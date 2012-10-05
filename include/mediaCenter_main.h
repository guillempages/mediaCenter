#ifndef MEDIACENTER_MAIN_H
#define MEDIACENTER_MAIN_H

class Plugin;

class HSM_BasicState: public HSM_State {
public:
    HSM_BasicState(HSM *hsm,const std::string &name);
    virtual ~HSM_BasicState();

protected:
    Plugin* plugin;
};

inline HSM_BasicState::HSM_BasicState(HSM *_hsm,const std::string& _name) : HSM_State(_hsm,_name), plugin(NULL) {};
inline HSM_BasicState::~HSM_BasicState() {if (plugin) delete plugin;};


class HSM_WaitChild: public HSM_State {
public:
    HSM_WaitChild(HSM *hsm,Event* exitEvt, int pid);
    
protected:
    Event* exitEvt;
    int pid;
    int timerCount;
    
private:
    virtual void processEvent(const Event* event);
};
inline HSM_WaitChild::HSM_WaitChild(HSM *_hsm, Event* _exitEvt, int _pid) :
    HSM_State(_hsm, "WaitChild"), exitEvt(_exitEvt), pid(_pid), timerCount(0) {};


class HSM_Start:public HSM_State {
public:
    HSM_Start(HSM *hsm);

private:
    virtual void processEvent(const Event* event);
};
inline HSM_Start::HSM_Start(HSM *_hsm) : HSM_State(_hsm,"Start") {};


class HSM_Quit:public HSM_State {
public:
    HSM_Quit(HSM *hsm);
private:
    virtual void processEvent(const Event* event);
};
inline HSM_Quit::HSM_Quit(HSM* _hsm) : HSM_State(_hsm,"Quit") {};


class HSM_Idle:public HSM_State {
public:
    HSM_Idle(HSM *hsm);

private:
    virtual void processEvent(const Event* event);

};
inline HSM_Idle::HSM_Idle(HSM *_hsm) : HSM_State(_hsm,"Idle") {};


class HSM_Menu:public HSM_BasicState {
public:
    HSM_Menu(HSM *hsm);

private:
    virtual void processEvent(const Event* event);
    std::string type;

};
inline HSM_Menu::HSM_Menu(HSM *_hsm) : HSM_BasicState(_hsm,"Menu") {};


class HSM_Music:public HSM_BasicState {
public:
    HSM_Music(HSM *hsm);

private:
    virtual void processEvent(const Event* event);

};
inline HSM_Music::HSM_Music(HSM *_hsm) : HSM_BasicState(_hsm,"Music") {};


class HSM_DVD:public HSM_BasicState {
public:
    HSM_DVD(HSM *hsm);

private:
    virtual void processEvent(const Event* event);

};
inline HSM_DVD::HSM_DVD(HSM *_hsm) : HSM_BasicState(_hsm,"DVD") {};


class HSM_CD:public HSM_BasicState {
public:
    HSM_CD(HSM *hsm);

private:
    virtual void processEvent(const Event* event);

};
inline HSM_CD::HSM_CD(HSM *_hsm) : HSM_BasicState(_hsm,"CD") {};


class HSM_Movie:public HSM_BasicState {
public:
    HSM_Movie(HSM *hsm);

private:
    virtual void processEvent(const Event* event);

    Plugin* plugin;
};
inline HSM_Movie::HSM_Movie(HSM *_hsm) : HSM_BasicState(_hsm,"Movie") {};


#endif

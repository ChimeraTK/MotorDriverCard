/*
 * StateMachine.h
 *
 *  Created on: Jan 19, 2017
 *      Author: vitimic
 */

#ifndef INCLUDE_STATEMACHINE_H_
#define INCLUDE_STATEMACHINE_H_

#include <iostream>
#include <functional>
#include <vector>
#include <string>
#include <map>
#include <mutex>
#include <future>
#include <atomic>

class TestStateMachine;


namespace ChimeraTK{

  class Event{
  public:
    Event(std::string eventName) :  _eventName(eventName){}
    Event() : _eventName("undefinedEvent"){}

    bool operator==(Event &event) const{
      if (this->_eventName == event._eventName){
        return true;
      }
      else{
        return false;
      }
    }

    operator std::string() const {return _eventName;}
    friend bool operator<(const Event &event1, const Event &event2);
  private:
    std::string _eventName;
  };

  bool operator<(const Event &event1, const Event &event2);

  class State;

  //structure for target and action
  struct TargetAndAction{
    TargetAndAction(State *target, std::function<void(void)> callback);
    TargetAndAction(const TargetAndAction& targetAndAction);
    TargetAndAction& operator=(const TargetAndAction& targetAndAction);
    State *targetState;
    std::function<void(void)> callbackAction;
  };


  using TransitionTable = std::map<Event, TargetAndAction >;

  class TestStateMachine;


  //State class
  class State{
  public:
    State(std::string stateName = "");
    virtual ~State();
    virtual void setTransition(Event event, State *target, std::function<void(void)> callbackAction);
    TransitionTable& getTransitionTable();
    std::string getName() const;
    //bool isEventUnknown(){return _isEventUnknown;}

  protected:
    std::string _stateName;
    TransitionTable _transitionTable;
  };


  // Base class for a state machine
  class StateMachine {

    // FIXME Make state a nested class once this works
    friend class State;
    friend class ::TestStateMachine;

  public:
    StateMachine();
    StateMachine(const StateMachine& stateMachine);
    StateMachine& operator=(const StateMachine& stateMachine);
    virtual ~StateMachine();
    State* getCurrentState();
    void setAndProcessUserEvent(Event event);
    Event getUserEvent();
    static Event noEvent;
    static Event undefinedEvent;

  protected:
    State _initState;
    State _endState;
    State *_currentState; //FIXME Use references?
    State *_requestedState;
    Event _userEvent; /*FIXME user event is not required anymore */
    //Event _internEvent;
    //Event _propagatedEvent;
    std::mutex _stateMachineMutex;
    std::future<void> _asyncActionActive;
    bool _isEventUnknown; //FIXME This is not used anymore
    std::atomic<bool> _boolAsyncActionActive;
    std::function<void(void)> _internalEventCallback; //FIXME Change interface to this
    void performTransition(Event event);
    void moveToRequestedState();
//    Event getAndResetInternalEvent();
//    Event getInternalEvent();
  };
}

#endif /* INCLUDE_STATEMACHINE_H_ */

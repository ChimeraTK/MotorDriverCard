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

  //Declaration machine state class
  class State{
  public:
    State(std::string stateName = "");
    virtual ~State();
    virtual void setTransition(Event event, State *target, std::function<void(void)> callbackAction);
    virtual State* performTransition(Event event);
    std::string getName() const;
    bool isEventUnknown(){return _isEventUnknown;}

    friend class TestStateMachine;
  protected:
    std::string _stateName;
    std::map<Event, TargetAndAction > _transitionTable;
    bool _isEventUnknown;
  };

  //Base class for a state machine
  class StateMachine {
  public:
    StateMachine();
    StateMachine(const StateMachine& stateMachine);
    StateMachine& operator=(const StateMachine& stateMachine);
    virtual ~StateMachine();
    State* getCurrentState();
    void setUserEvent(Event event);
    virtual void processEvent();
    void setAndProcessUserEvent(Event event);
    Event getAndResetUserEvent();
    Event getUserEvent();
    static Event noEvent;
    static Event undefinedEvent;

  friend class TestStateMachine;
  protected:
    State _initState;
    State _endState;
    State *_currentState;
    Event _userEvent;
    Event _internEvent;
    Event getAndResetInternalEvent();
    Event getInternalEvent();
  };
}

#endif /* INCLUDE_STATEMACHINE_H_ */

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
#include <thread>
#include <atomic>

struct StateMachineTestFixture;


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
  struct TransitionData{
    TransitionData(State *target, std::function<void(void)> callback);
    TransitionData(const TransitionData& data);
    TransitionData& operator=(const TransitionData& data) = delete;
    State *targetState;
    std::function<void(void)> callbackAction;
  };


  using TransitionTable = std::map<Event, TransitionData >;

  class TestStateMachine;


  //State class
  class State{
  public:
    State(std::string stateName = "");
    virtual ~State();
    virtual void setTransition(Event event, State *target, std::function<void(void)> callbackAction);
    const TransitionTable& getTransitionTable() const;
    std::string getName() const;

  protected:
    std::string _stateName;
    TransitionTable _transitionTable;
  };


  // Base class for a state machine
  class StateMachine {

    // FIXME Make state a nested class once this works
    friend class State;
    friend struct ::StateMachineTestFixture;

  public:
    StateMachine();
    StateMachine(const StateMachine& stateMachine) = delete;
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
    State *_currentState;
    State *_requestedState;

    std::mutex _stateMachineMutex;
    std::atomic<bool> _asyncActionActive;
    std::function<void(void)> _internalEventCallback;
    void performTransition(Event event);
    void moveToRequestedState();
  };
}

#endif /* INCLUDE_STATEMACHINE_H_ */

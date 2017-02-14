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
    Event(std::string eventName = "undefinedEvent") :  _eventName(eventName){}
    bool operator==(Event &event) const{
      if (this->_eventName == event._eventName){
	return true;
      }else{
	return false;
      }
    }
    operator std::string() const {return _eventName;}
  private:
    std::string _eventName;
  };

  class State;

  //structure for target and action

  struct TargetAndAction{
    TargetAndAction(State *target, std::function<void(void)> callback);
    State *targetState;
    std::function<void(void)> callbackAction;
  };

  TargetAndAction::TargetAndAction(State *target, std::function<void(void)> callback) : targetState(target), callbackAction(callback){}

  //Declaration machine state class

  class State{
  public:
    State(std::string stateName = "");
    virtual ~State();
    void setTransition(Event event, State *target, std::function<void(void)> callbackAction);
    void setEventGeneration(std::function<Event(void)> callbackGenerateEvent);
    virtual Event getEvent();
    State* performTransition(Event event);
    std::string getName();
    bool isEventUnknown(){return _unknownEvent;}
    static Event noEvent;
    static Event undefinedEvent;
  private:
    std::string _stateName;
    std::map<Event, TargetAndAction > _transitionTable;
    std::function<Event(void)> _callbackGenerateEvent;
    void noAction(){};
    bool _unknownEvent;
  };

  Event State::noEvent("noEvent");
  Event State::undefinedEvent("undefinedEvent");

  State::State(std::string stateName) :
  _stateName(stateName),
  _transitionTable(),
  _callbackGenerateEvent(),
  _unknownEvent(false){}

  State::~State(){}

  void State::setTransition(Event event, State *target, std::function<void(void)> callbackAction){
    TargetAndAction targetAndAction(target, callbackAction);
    _transitionTable.insert(std::pair< Event, TargetAndAction >(event, targetAndAction));
  }

  void State::setEventGeneration(std::function<Event(void)> callbackGenerateEvent){
    _callbackGenerateEvent = callbackGenerateEvent;
  }

  Event State::getEvent(){
    try{
      return _callbackGenerateEvent();
    }catch(const std::bad_function_call& e){
      return State::noEvent;
    }
  }

  State* State::performTransition(Event event){
    typename std::map< Event, TargetAndAction >::iterator it;
    it = _transitionTable.find(std::string(event));
    if (it !=_transitionTable.end()){
      (it->second).callbackAction();
      _unknownEvent = false;
      return ((it->second).targetState);
    }else{
      _unknownEvent = true;
      return this;
    }
  }

  std::string State::getName(){
    return _stateName;
  }

  //base class for a state machine

  template <class T>
  class StateMachine : public State{
  public:
    StateMachine(T &frontEnd, std::string name);
    virtual ~StateMachine();
    virtual void processEvent();
  private:
    State _initState;
    State _endState;
    State *_currentState;
    T &_frontEnd;
  };

  template <class T>
  StateMachine<T>::StateMachine(T &frontEnd, std::string name) :
  State(name),
  _initState("initState"),
  _endState("endState"),
  _currentState(&_initState),
  _frontEnd(frontEnd)
  {}

  template <class T>
  StateMachine<T>::~StateMachine(){}

  template<class T>
  void StateMachine<T>::processEvent(){
    _currentState->performTransition(_currentState->getEvent());
  }

}

#endif /* INCLUDE_STATEMACHINE_H_ */

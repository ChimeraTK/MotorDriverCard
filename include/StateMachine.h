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
    virtual void setTransition(Event event, State *target, std::function<void(void)> callbackAction);
    //void setEventGeneration(std::function<Event(void)> callbackGenerateEvent);
    //virtual Event getEvent();
    virtual State* performTransition(Event event);
    std::string getName();
    bool isEventUnknown(){return _unknownEvent;}
  private:
    std::string _stateName;
    std::map<Event, TargetAndAction > _transitionTable;
    //std::function<Event(void)> _callbackGenerateEvent;
    //void noAction(){};
    bool _unknownEvent;
    Event _latestEvent;
  };

  State::State(std::string stateName) :
  _stateName(stateName),
  _transitionTable(),
  //_callbackGenerateEvent(),
  _unknownEvent(false){}

  State::~State(){}

  void State::setTransition(Event event, State *target, std::function<void(void)> callbackAction){
    TargetAndAction targetAndAction(target, callbackAction);
    _transitionTable.insert(std::pair< Event, TargetAndAction >(event, targetAndAction));
  }

//  void State::setEventGeneration(std::function<Event(void)> callbackGenerateEvent){
//    _callbackGenerateEvent = callbackGenerateEvent;
//  }
//
//  Event State::getEvent(){
//    try{
//      return _callbackGenerateEvent();
//    }catch(const std::bad_function_call& e){
//      return State::noEvent;
//    }
//  }

  State* State::performTransition(Event event){
    _latestEvent = event;
    typename std::map< Event, TargetAndAction >::iterator it;
    it = _transitionTable.find(event);
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

  class StateMachine : public State{
  public:
    StateMachine(std::string name);
    virtual ~StateMachine();
    virtual void processEvent();
    virtual State* performTransition(Event event);
    State* getCurrentState();
    void setUserEvent(Event event);
    Event getAndResetUserEvent();
    Event getUserEvent();
    static Event noEvent;
    static Event undefinedEvent;
  protected:
    State _initState;
    State _endState;
    State *_currentState;
    Event _userEvent;
    Event _internEvent;
    Event getAndResetInternalEvent();
    Event getInternalEvent();
    virtual bool propagateEvent();
  };

  Event StateMachine::noEvent("noEvent");
  Event StateMachine::undefinedEvent("undefinedEvent");

  StateMachine::StateMachine(std::string name) :
      State(name),
      _initState("initState"),
      _endState("endState"),
      _currentState(&_initState),
      _userEvent(noEvent),
      _internEvent(noEvent)
  {}


  StateMachine::~StateMachine(){}

  void StateMachine::processEvent(){
    if (_userEvent == noEvent){
      _currentState->performTransition(getAndResetInternalEvent());
    }else{
      _currentState->performTransition(getAndResetUserEvent());
    }
  }

  State* StateMachine::getCurrentState(){
    return _currentState;
  }

  State* StateMachine::performTransition(Event event){
    setUserEvent(event);
    processEvent();
    if (propagateEvent()){
      return State::performTransition(event);
    }else{
      return this;
    }
  }

  void StateMachine::setUserEvent(Event event){
    _userEvent = event;
  }

  Event StateMachine::getAndResetUserEvent(){
    Event tempEvent = _userEvent;
    _userEvent = StateMachine::noEvent;
    return tempEvent;
  }

  Event StateMachine::getUserEvent(){
    return _userEvent;
  }

  Event StateMachine::getAndResetInternalEvent(){
    Event tempEvent = _internEvent;
    _internEvent = StateMachine::noEvent;
    return tempEvent;
  }

  Event StateMachine::getInternalEvent(){
    return _internEvent;
  }
}

#endif /* INCLUDE_STATEMACHINE_H_ */

/*
 * StateMachine.cc
 *
 *  Created on: Feb 16, 2017
 *      Author: vitimic
 */

#include "StateMachine.h"

namespace ChimeraTK{

  bool operator<(const Event &event1, const Event &event2){
    return event1._eventName < event2._eventName;
  }

  TargetAndAction::TargetAndAction(State *target, std::function<void(void)> callback) : targetState(target), callbackAction(callback){}

  TargetAndAction::TargetAndAction(const TargetAndAction& targetAndAction) :
      targetState(targetAndAction.targetState),
      callbackAction(targetAndAction.callbackAction){}

  TargetAndAction& TargetAndAction::operator=(const TargetAndAction& targetAndAction){
    this->targetState = targetAndAction.targetState;
    this->callbackAction = targetAndAction.callbackAction;
    return *this;
  }

  State::State(std::string stateName) :
      _stateName(stateName),
      _transitionTable(),
      _unknownEvent(false){}

  State::~State(){}

  void State::setTransition(Event event, State *target, std::function<void(void)> callbackAction){
    TargetAndAction targetAndAction(target, callbackAction);
    _transitionTable.insert(std::pair< Event, TargetAndAction >(event, targetAndAction));
  }

  State* State::performTransition(Event event){
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

  std::string State::getName() const{
    return _stateName;
  }

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

  StateMachine::StateMachine(const StateMachine &stateMachine) :
      State(stateMachine.getName()),
      _initState(stateMachine._initState),
      _endState("endState"),
      _currentState(&_initState),
      _userEvent(noEvent),
      _internEvent(noEvent){
  }

  StateMachine& StateMachine::operator =(const StateMachine &stateMachine){
    this->_stateName = stateMachine._stateName;
    this->_transitionTable = stateMachine._transitionTable;
    this->_unknownEvent = stateMachine._unknownEvent;
    this->_initState = stateMachine._initState;
    this->_endState = stateMachine._endState;
    this->_currentState = stateMachine._currentState;
    this->_userEvent = stateMachine._userEvent;
    this->_internEvent = stateMachine._internEvent;
    return *this;
  }

  StateMachine::~StateMachine(){}

  void StateMachine::processEvent(){
    _unknownEvent = false;
    if (_userEvent == noEvent){
      _currentState = _currentState->performTransition(getAndResetInternalEvent());
    }else{
      _currentState = _currentState->performTransition(getAndResetUserEvent());
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

  bool StateMachine::propagateEvent(){
    return _currentState->isEventUnknown();
  }
}



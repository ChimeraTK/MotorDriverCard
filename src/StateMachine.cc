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
      _isEventUnknown(false){}

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
      _isEventUnknown = false;

      // We need to distinguish if an async action
      if(_asyncActionActive.valid()){
        _requestedState = *(it->second).targetState;
      }
      else{
        return ((it->second).targetState);
      }
    }
    else{
      _isEventUnknown = true;
      return this;
    }
  }

  std::string State::getName() const{
    return _stateName;
  }

  Event StateMachine::noEvent("noEvent");
  Event StateMachine::undefinedEvent("undefinedEvent");

  StateMachine::StateMachine() :
             _initState("initState"),
             _endState("endState"),
             _currentState(&_initState),
             _userEvent(noEvent),
             _internEvent(noEvent),
             _mutex()
  {}

  StateMachine::StateMachine(const StateMachine &stateMachine) :
      _initState(stateMachine._initState),
      _endState("endState"),
      _currentState(&_initState),
      _userEvent(noEvent),
      _internEvent(noEvent),
      _mutex()
  {}

//  StateMachine& StateMachine::operator =(const StateMachine &stateMachine){
//    this->_initState = stateMachine._initState;
//    this->_endState = stateMachine._endState;
//    this->_currentState = stateMachine._currentState;
//    this->_userEvent = stateMachine._userEvent;
//    this->_internEvent = stateMachine._internEvent;
//    return *this;
//  }

  StateMachine::~StateMachine(){}

  void StateMachine::processEvent(){
    if (_userEvent == noEvent){
      //TODO Maybe initiate internal events only from the callbacks
      _currentState = _currentState->performTransition(getAndResetInternalEvent());
    }else{
      _currentState = _currentState->performTransition(getAndResetUserEvent());
    }
  }

  State* StateMachine::getCurrentState(){
    return _currentState;
  }

  void StateMachine::setUserEvent(Event event){
    _userEvent = event;
  }

  void StateMachine::setAndProcessUserEvent(Event event){
    _userEvent = event;
    processEvent();
    return;
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

} /* namespace ChimeraTK */



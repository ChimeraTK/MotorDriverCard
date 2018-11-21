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
      _transitionTable(){}

  State::~State(){}

  void State::setTransition(Event event, State *target, std::function<void(void)> callbackAction){
    TargetAndAction targetAndAction(target, callbackAction);
    _transitionTable.insert(std::pair< Event, TargetAndAction >(event, targetAndAction));
  }

//  State* State::performTransition(Event event){
//    typename std::map< Event, TargetAndAction >::iterator it;
//    it = _transitionTable.find(event);
//    if(it !=_transitionTable.end()){
//      (it->second).callbackAction();
//      _isEventUnknown = false;
//      return ((it->second).targetState);
//    }
//    else{
//      _isEventUnknown = true;
//      return this;
//    }
//  }

  TransitionTable& State::getTransitionTable(){
    return _transitionTable;
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
             _requestedState(nullptr),
             _userEvent(noEvent),
             _stateMachineMutex(),
             _asyncActionActive(),
             _isEventUnknown(false),
             _boolAsyncActionActive(false),
             _internalEventCallback([]{})
  {}

  StateMachine::StateMachine(const StateMachine &stateMachine) :
      _initState(stateMachine._initState),
      _endState("endState"),
      _currentState(&_initState),
      _requestedState(nullptr),
      _userEvent(noEvent),
      _stateMachineMutex(),
      _asyncActionActive(),
      _isEventUnknown(false),
      _boolAsyncActionActive(false),
      _internalEventCallback([]{})
  {}

  StateMachine::~StateMachine(){}


  State* StateMachine::getCurrentState(){
    std::lock_guard<std::mutex> lck(_stateMachineMutex);

    _internalEventCallback();
    return _currentState;
  }


  void StateMachine::setAndProcessUserEvent(Event event){
    //{
      std::lock_guard<std::mutex> lck(_stateMachineMutex);
      _userEvent = event;
    //}
   performTransition(_userEvent);
  }


  Event StateMachine::getUserEvent(){
    std::lock_guard<std::mutex> lck(_stateMachineMutex);
    return _userEvent;
  }


  void StateMachine::performTransition(Event event){
    std::map< Event, TargetAndAction >::iterator it;

    TransitionTable& transitionTable = _currentState->getTransitionTable();
    it = transitionTable.find(event);
    if(it != transitionTable.end()){
      _isEventUnknown = false;

      _requestedState = ((it->second).targetState);

      // Apply new state right away, if no async action active
      if(!_boolAsyncActionActive.load()){
        _currentState = _requestedState;
        _requestedState = nullptr;
      }
      (it->second).callbackAction();
    }
    else{
      _isEventUnknown = true;
    }
  }

  void StateMachine::moveToRequestedState(){
    if(_requestedState != nullptr){
      _currentState = _requestedState;
      _requestedState = nullptr;
    }
  }
} /* namespace ChimeraTK */



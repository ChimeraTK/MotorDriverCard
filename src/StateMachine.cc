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

  TransitionData::TransitionData(State *target, std::function<void(void)> entryCallback, std::function<void(void)> internalCallback)
    : targetState(target), entryCallbackAction(entryCallback), internalCallbackAction(internalCallback){}

  TransitionData::TransitionData(const TransitionData& targetAndAction) :
      targetState(targetAndAction.targetState),
      entryCallbackAction(targetAndAction.entryCallbackAction),
      internalCallbackAction(targetAndAction.internalCallbackAction){}

  State::State(std::string stateName) :
      _stateName(stateName),
      _transitionTable(){}

  State::~State(){}

  void State::setTransition(Event event, State *target, std::function<void(void)> entryCallback, std::function<void(void)> internalCallback){
    TransitionData transitionData(target, entryCallback, internalCallback);
    _transitionTable.insert(std::pair< Event, TransitionData >(event, transitionData));
  }


  const TransitionTable& State::getTransitionTable() const{
    return _transitionTable;
  }

  std::string State::getName() const{
    return _stateName;
  }

  Event StateMachine::noEvent("noEvent");
  //Event StateMachine::undefinedEvent("undefinedEvent");

  StateMachine::StateMachine() :
             _initState("initState"),
             _endState("endState"),
             _currentState(&_initState),
             _requestedState(nullptr),
             _stateMachineMutex(),
             _asyncActionActive(false),
             _internalEventCallback([]{}),
             _requestedInternalCallback([]{})
  {}

  StateMachine::~StateMachine(){}


  State* StateMachine::getCurrentState(){
    std::lock_guard<std::mutex> lck(_stateMachineMutex);

    _internalEventCallback();
    return _currentState;
  }


  void StateMachine::setAndProcessUserEvent(Event event){
    std::lock_guard<std::mutex> lck(_stateMachineMutex);
   performTransition(event);
  }


  void StateMachine::performTransition(Event event){
    std::map< Event, TransitionData >::const_iterator it;

    const TransitionTable& transitionTable = _currentState->getTransitionTable();
    it = transitionTable.find(event);
    if(it != transitionTable.end()){

      _requestedState = ((it->second).targetState);
      _requestedInternalCallback = (it->second).internalCallbackAction;

      // Apply new state right away, if no async action active
      if(!_asyncActionActive.load()){
        _currentState = _requestedState;
        _requestedState = nullptr;
        _internalEventCallback = _requestedInternalCallback;
      }
      (it->second).entryCallbackAction();
    }
  }

  void StateMachine::moveToRequestedState(){
    if(_requestedState != nullptr){
      _currentState = _requestedState;
      _internalEventCallback = _requestedInternalCallback;
      _requestedState = nullptr;
    }
  }
} /* namespace ChimeraTK */



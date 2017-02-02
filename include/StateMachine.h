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

typedef std::string Event;

template <class T>
class TargetAndAction;

//Declaration machine state class

template <class T>
class State{
public:
  State(std::shared_ptr<T> stateMachine, std::string stateName = "");
  void setTransition(Event event, State<T> *target, std::function<void(T*)> callbackAction);
  void setEventGeneration(std::function<Event(T*)> callbackGenerateEvent);
  Event generateEvent();
  State<T>* performTransition(Event event);
  std::string getName();
private:
  std::string _stateName;
  std::map<Event, TargetAndAction<T> > _transitionTable;
  std::function<Event(T*)> _callbackGenerateEvent;
  std::shared_ptr<T> _stateMachine;
  bool _eventUnknown;
};

template <class T>
State<T>::State(std::shared_ptr<T> stateMachine, std::string stateName) :
_stateName(stateName),
_transitionTable(),
_callbackGenerateEvent(),
_stateMachine(stateMachine),
_eventUnknown(false){}

template <class T>
void State<T>::setTransition(Event event, State<T> *target, std::function<void(T*)> callbackAction){
  TargetAndAction<T> targetAndAction(target, callbackAction);
  _transitionTable.insert(std::pair< Event, TargetAndAction<T> >(event, targetAndAction));
  //callbackAction(_stateMachine);
  //std::cout << "State Name " << _stateName << " transitionTable pointer " << &_transitionTable << std::endl;
}

template <class T>
void State<T>::setEventGeneration(std::function<Event(T*)> callbackGenerateEvent){
  _callbackGenerateEvent = callbackGenerateEvent;
}

template <class T>
Event State<T>::generateEvent(){
  Event event = _callbackGenerateEvent(_stateMachine);
  return event;
}

template <class T>
State<T>* State<T>::performTransition(Event event){
  typename std::map< Event, TargetAndAction<T> >::iterator it;
  it = _transitionTable.find(std::string(event));
  if (it !=_transitionTable.end()){
    (it->second).callbackAction(_stateMachine);

    return ((it->second).targetState);
  }
  std::cout << "not found" << std::endl;
  return this;
}

template <class T>
std::string State<T>::getName(){
  return _stateName;
}

//structure for target and action

template <class T>
class TargetAndAction{
public:
  TargetAndAction(State<T> *target, std::function<void(T*)> callback);
  State<T> *targetState;
  std::function<void(T*)> callbackAction;
};

template <class T>
TargetAndAction<T>::TargetAndAction(State<T> *target, std::function<void(T*)> callback) : targetState(target), callbackAction(callback){}

//base class for a state machine

template <class T>
class StateMachine : public State<T>{
public:
  StateMachine(T &frontEnd, std::string name, std::shared_ptr< StateMachine<T> > parentStateMachine);
  virtual ~StateMachine();
  virtual void processEvent();
  void init();
  bool isTerminated();
protected:

  Event noEvent;
  T &_frontEnd;
  std::shared_ptr <StateMachine> _parentStateMachine;
  State<StateMachine> _startState;
  State<StateMachine> _endState;
};

template <class T>
StateMachine<T>::StateMachine(T &frontEnd, std::string name, std::shared_ptr< StateMachine<T> > parentStateMachine) :
State<T>(parentStateMachine, name),
noEvent("noEvent"),
_frontEnd(frontEnd),
_parentStateMachine(parentStateMachine)
{}

template <class T>
StateMachine<T>::~StateMachine(){}

template <class T>
void StateMachine<T>::processEvent(){

}

#endif /* INCLUDE_STATEMACHINE_H_ */

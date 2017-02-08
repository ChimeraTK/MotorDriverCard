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

  typedef std::string Event;

  template <class T>
  class TargetAndAction;

  //Declaration machine state class

  template <class T>
  class State{
  public:
    State(T &frontEnd, std::string stateName = "");
    virtual ~State();
    void setTransition(Event event, State<T> *target, std::function<void(void)> callbackAction);
    void setEventGeneration(std::function<Event(void)> callbackGenerateEvent);
    virtual Event generateEvent();
    State<T>* performTransition(Event event);
    std::string getName();
  private:
    std::string _stateName;
    std::map<Event, TargetAndAction<T> > _transitionTable;
    std::function<Event(void)> _callbackGenerateEvent;
    T &_frontEnd;
    bool _eventUnknown;
  };

  template <class T>
  State<T>::State(T &frontEnd, std::string stateName) :
  _stateName(stateName),
  _transitionTable(),
  _frontEnd(frontEnd),
  _eventUnknown(false){}

  template<class T>
  State<T>::~State(){}

  template <class T>
  void State<T>::setTransition(Event event, State<T> *target, std::function<void(void)> callbackAction){
    TargetAndAction<T> targetAndAction(target, callbackAction);
    _transitionTable.insert(std::pair< Event, TargetAndAction<T> >(event, targetAndAction));
  }

  template <class T>
  void State<T>::setEventGeneration(std::function<Event(void)> callbackGenerateEvent){
    _callbackGenerateEvent = callbackGenerateEvent;
  }

  template <class T>
  Event State<T>::generateEvent(){
    try{
      return _callbackGenerateEvent();
    }catch(const std::bad_function_call& e){
      std::cout << e.what() << std::endl;
      Event noEvent("noEvent");
      return noEvent;
    }
  }

  template <class T>
  State<T>* State<T>::performTransition(Event event){
    typename std::map< Event, TargetAndAction<T> >::iterator it;
    it = _transitionTable.find(std::string(event));
    if (it !=_transitionTable.end()){
      (it->second).callbackAction();
      _eventUnknown = false;
      return ((it->second).targetState);
    }else{
      _eventUnknown = true;
      return this;
    }
  }

  template <class T>
  std::string State<T>::getName(){
    return _stateName;
  }

  //structure for target and action

  template <class T>
  class TargetAndAction{
  public:
    TargetAndAction(State<T> *target, std::function<void(void)> callback);
    State<T> *targetState;
    std::function<void(void)> callbackAction;
  };

  template <class T>
  TargetAndAction<T>::TargetAndAction(State<T> *target, std::function<void(void)> callback) : targetState(target), callbackAction(callback){}

  //base class for a state machine

  template <class T>
  class StateMachine : public State<T>{
  public:
    StateMachine(T &frontEnd, std::string name);
    virtual ~StateMachine();
    virtual void processEvent() = 0;
    virtual void init() = 0;
    bool isTerminated() = 0;
  protected:
    Event _noEvent;
    T &_frontEnd;
  };

  template <class T>
  StateMachine<T>::StateMachine(T &frontEnd, std::string name) :
  State<T>(frontEnd, name),
  _noEvent("noEvent"),
  _frontEnd(frontEnd)
  {}

  template <class T>
  StateMachine<T>::~StateMachine(){}

  template <class T>
  void StateMachine<T>::processEvent(){

  }
}

#endif /* INCLUDE_STATEMACHINE_H_ */

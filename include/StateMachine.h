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

  /**
   * @brief Base class for a state machine
   */
  class StateMachine {

    friend struct ::StateMachineTestFixture;

  public:

    /**
     * @brief Class describing events triggering the StateMachine
     */
    class Event{
    public:
      Event(std::string eventName) :  _eventName(eventName){}
      Event() = delete;

      //operator std::string() const {return _eventName;}
      friend bool operator<(const Event &event1, const Event &event2);
    private:
      std::string _eventName;
    };


    //State class
    class State{

      friend class StateMachine;

      // Structure for target and callbacks of a transition
      // FIXME Defining the internal callback here is a hack, it belongs to the state rather than the transition
      struct TransitionData{
        TransitionData(State *target, std::function<void(void)> entryCallback, std::function<void(void)> internalCallback);
        TransitionData(const TransitionData& data);
        TransitionData& operator=(const TransitionData& data) = delete;
        State *targetState;
        std::function<void(void)> entryCallbackAction;
        std::function<void(void)> internalCallbackAction;
      };

      using TransitionTable = std::map<Event, TransitionData >;

    public:
      State(std::string stateName = "");
      virtual ~State();
      virtual void setTransition(Event event, State *target, std::function<void(void)> entryCallback, std::function<void(void)> internalCallback = []{});
      const TransitionTable& getTransitionTable() const;
      std::string getName() const;

    protected:
      std::string _stateName;
      TransitionTable _transitionTable;
    };



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
    std::function<void(void)> _requestedInternalCallback;
    void performTransition(Event event);
    void moveToRequestedState();
  };

  bool operator<(const StateMachine::Event &event1, const StateMachine::Event &event2);


}

#endif /* INCLUDE_STATEMACHINE_H_ */

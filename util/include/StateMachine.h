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


namespace ChimeraTK {
namespace MotorDriver{
namespace utility{

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

      friend bool operator<(const Event &event1, const Event &event2);
    private:
      std::string _eventName;
    };


    /**
     * @brief Class describing state of the StateMachine
     *
     * This object holds the state name and a TransitionTable describing transitions from an
     * instance to any target states.
     */
    class State{

      friend class StateMachine;

      /**
       * @brief TransitionData struct
       *
       * This object holds the target state and two callbacks. The entryCallback is performed when the target state is entered.
       * The internalCallback can be used to describe internal events of the StateMachine.
       *
       * FIXME Defining the internal callback here is a hack, it belongs to the state rather than the transition
       */
      struct TransitionData{
        TransitionData(State *target, std::function<void(void)> entryCallback, std::function<void(void)> internalCallback);
        TransitionData(const TransitionData& data);
        TransitionData& operator=(const TransitionData& data) = delete;
        State *targetState;
        std::function<void(void)> entryCallbackAction;
        std::function<void(void)> internalCallbackAction;
      };

      /**
       * TransitionTable type, maps TranssitionData to an event.
       */
      using TransitionTable = std::map<Event, TransitionData >;

    public:
      State(std::string stateName = "");
      virtual ~State();

      /**
       * This function will append a new state the the TransitionTable of this state.
       *
       * @param event - Event causing the transition
       * @param target - Pointer to the target state
       * @param entryCallback - A callback function that will be executed when entering the target state
       * @param internalCallback - An optional internal callback that can be used to implement internal events.
       *                           To achieve this, the callback function must check the condition for a state transition
       *                           and might then call StateMachine::performTransition. This callback is performed on each call
       *                           to StateMachine::getCurrentState.
       */
      void setTransition(Event event, State *target, std::function<void(void)> entryCallback, std::function<void(void)> internalCallback = []{});
      const TransitionTable& getTransitionTable() const;
      /**
       * @brief getName
       * @return
       */
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
    bool hasRequestedState();
    void moveToRequestedState();
  };

  bool operator<(const StateMachine::Event &event1, const StateMachine::Event &event2);

}
}
}
#endif /* INCLUDE_STATEMACHINE_H_ */

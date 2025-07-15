// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <utility>

struct StateMachineTestFixture;

namespace ChimeraTK::MotorDriver::utility {

  /**
   * @brief Base class for a state machine
   */
  class StateMachine {
    friend struct ::StateMachineTestFixture;

   public:
    /**
     * @brief Class describing events triggering the StateMachine
     */
    class Event {
     public:
      explicit Event(std::string eventName) : _eventName(std::move(eventName)) {}
      Event() = delete;

      friend bool operator<(const Event& event1, const Event& event2);
      [[nodiscard]] const std::string& getName() const { return _eventName; }

     private:
      std::string _eventName;
    };

    /**
     * @brief Class describing state of the StateMachine
     *
     * This object holds the state name and a TransitionTable describing
     * transitions from an instance to any target states.
     */
    class State {
      friend class StateMachine;

      /**
       * @brief TransitionData struct
       *
       * This object holds the target state and two callbacks. The entryCallback
       * is performed when the target state is entered. The internalCallback can
       * be used to describe internal events of the StateMachine.
       *
       * FIXME Defining the internal callback here is a hack, it belongs to the
       * state rather than the transition
       */
      struct TransitionData {
        TransitionData(
            State* target, std::function<void(void)> entryCallback, std::function<void(void)> internalCallback);
        TransitionData(const TransitionData& data);
        TransitionData& operator=(const TransitionData& data) = delete;
        State* targetState;
        std::function<void(void)> entryCallbackAction;
        std::function<void(void)> internalCallbackAction;
      };

      /**
       * TransitionTable type, maps TransitionData to an event.
       */
      using TransitionTable = std::map<Event, TransitionData>;

     public:
      explicit State(std::string stateName = "");
      virtual ~State();

      /**
       * This function will append a new state the the TransitionTable of this
       * state.
       *
       * @param event - Event causing the transition
       * @param target - Pointer to the target state
       * @param entryCallback - A callback function that will be executed when
       * entering the target state
       * @param internalCallback - An optional internal callback that can be used
       * to implement internal events. To achieve this, the callback function must
       * check the condition for a state transition and might then call
       * StateMachine::performTransition. This callback is performed on each call
       * to StateMachine::getCurrentState.
       */
      void setTransition(
          const Event& event, State* target, std::function<void(void)> entryCallback,
          std::function<void(void)> internalCallback = [] {});
      [[nodiscard]] const TransitionTable& getTransitionTable() const;
      /**
       * @brief get the name of the state
       * @return name of state
       */
      [[nodiscard]] std::string getName() const;

     protected:
      std::string _stateName;
      TransitionTable _transitionTable;
    };

    StateMachine() = default;
    StateMachine(const StateMachine& stateMachine) = delete;
    StateMachine& operator=(const StateMachine& stateMachine);
    virtual ~StateMachine() = default;

    State* getCurrentState();
    void setAndProcessUserEvent(const Event& event);
    Event getUserEvent();

   protected:
    State _initState{"initState"};
    State _endState{"endState"};
    State* _currentState{&_initState};
    State* _requestedState{nullptr};

    std::mutex _stateMachineMutex;
    /// Whether or not the current state transition is waiting for something to happen asynchronously
    std::atomic<bool> _asyncActionActive{false};
    /// ???
    std::function<void(void)> _internalEventCallback{[]() {}};
    /// ???
    std::function<void(void)> _requestedInternalCallback{[]() {}};
    void performTransition(const Event& event);
    bool hasRequestedState();
    void moveToRequestedState();
  };

  bool operator<(const StateMachine::Event& event1, const StateMachine::Event& event2);

} // namespace ChimeraTK::MotorDriver::utility

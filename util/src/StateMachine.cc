// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "StateMachine.h"

#include "DebugLog.h"
#include "StepperMotorUtil.h"

#include <cassert>
#include <string>

namespace ChimeraTK::MotorDriver {
  namespace utility {

    /******************************************************************************************************************/

    bool operator<(const StateMachine::Event& event1, const StateMachine::Event& event2) {
      return event1._eventName < event2._eventName;
    }

    /******************************************************************************************************************/

    StateMachine::State::TransitionData::TransitionData(
        State* target, std::function<void(void)> entryCallback, std::function<void(void)> internalCallback)
    : targetState(target), entryCallbackAction(std::move(entryCallback)),
      internalCallbackAction(std::move(internalCallback)) {}

    /******************************************************************************************************************/

    StateMachine::State::TransitionData::TransitionData(const State::TransitionData& targetAndAction) = default;

    /******************************************************************************************************************/

    StateMachine::State::State(std::string stateName) : _stateName(std::move(stateName)) {}

    /******************************************************************************************************************/

    StateMachine::State::~State() = default;

    /******************************************************************************************************************/

    void StateMachine::State::setTransition(const Event& event, State* target, std::function<void(void)> entryCallback,
        std::function<void(void)> internalCallback) {
      TransitionData transitionData(target, std::move(entryCallback), std::move(internalCallback));
      _transitionTable.insert({event, transitionData});
    }

    /******************************************************************************************************************/

    const StateMachine::State::TransitionTable& StateMachine::State::getTransitionTable() const {
      return _transitionTable;
    }

    /******************************************************************************************************************/

    std::string StateMachine::State::getName() const {
      return _stateName;
    }

    /******************************************************************************************************************/

    StateMachine::State* StateMachine::getCurrentState() {
      std::lock_guard<std::mutex> lck(_stateMachineMutex);

      auto* state = _currentState;
      _internalEventCallback();

      return state;
    }

    /******************************************************************************************************************/

    void StateMachine::setAndProcessUserEvent(const Event& event) {
      std::lock_guard<std::mutex> lck(_stateMachineMutex);
      DEBUG_LOG(
          "StateMachine: processing user event '", event.getName(), "' in state '", _currentState->getName(), "'");
      performTransition(event);
    }

    /******************************************************************************************************************/

    void StateMachine::performTransition(const Event& event) {
      auto const& transitionTable = _currentState->getTransitionTable();
      if(auto it = transitionTable.find(event); it != transitionTable.end()) {
        _requestedState = ((it->second).targetState);
        _requestedInternalCallback = (it->second).internalCallbackAction;

        DEBUG_LOG("StateMachine: transition from '", _currentState->getName(), "' on event '", event.getName(),
            "' to '", _requestedState->getName(), "'",
            _asyncActionActive.load() ? " (deferred, async action active)" : "");

        // Apply new state right away, if no async action active
        if(!_asyncActionActive.load()) {
          moveToRequestedState();
        }
        (it->second).entryCallbackAction();
      }
      else {
        DEBUG_LOG(
            "StateMachine: no transition for event '", event.getName(), "' in state '", _currentState->getName(), "'");
      }
    }

    /******************************************************************************************************************/

    bool StateMachine::hasRequestedState() {
      return _requestedState != nullptr;
    }

    /******************************************************************************************************************/

    void StateMachine::moveToRequestedState() {
      if(_requestedState != nullptr) {
        DEBUG_LOG("StateMachine: state change '", _currentState->getName(), "' -> '", _requestedState->getName(), "'");
        _currentState = _requestedState;
        _internalEventCallback = _requestedInternalCallback;
        _requestedState = nullptr;
      }
    }

  } // namespace utility

  /********************************************************************************************************************/

  std::string toString(ExitStatus status) {
    switch(status) {
      case ExitStatus::ERR_INVALID_PARAMETER:
        return "Invalid parameter";
      case ExitStatus::ERR_SYSTEM_IN_ACTION:
        return "System in action";
      case ExitStatus::ERR_SYSTEM_NOT_CALIBRATED:
        return "System not calibrated";
      case ExitStatus::SUCCESS:
        return "Success";
      default:
        assert(false);
        return "Unknown Error";
    }
  }

  /********************************************************************************************************************/

  std::string toString(Error error) {
    switch(error) {
      case Error::ACTION_ERROR:
        return "Error during action";
      case Error::NO_ERROR:
        return "No error";
      case Error::CALIBRATION_ERROR:
        return "Calibration error";
      case Error::BOTH_END_SWITCHES_ON:
        return "Both endswitches on";
      case Error::MOVE_INTERRUPTED:
        return "Movement was interrupted";
      case Error::EMERGENCY_STOP:
        return "Emergency stop";
      default:
        break;
    }
    // Several error flags are set at once. Render them as a comma separated list.
    std::string result;
    auto append = [&result](const char* name) {
      if(!result.empty()) {
        result += ", ";
      }
      result += name;
    };
    if((static_cast<unsigned int>(error) & static_cast<unsigned int>(Error::ACTION_ERROR)) != 0U) {
      append("Error during action");
    }
    if((static_cast<unsigned int>(error) & static_cast<unsigned int>(Error::CALIBRATION_ERROR)) != 0U) {
      append("Calibration error");
    }
    if((static_cast<unsigned int>(error) & static_cast<unsigned int>(Error::BOTH_END_SWITCHES_ON)) != 0U) {
      append("Both endswitches on");
    }
    if((static_cast<unsigned int>(error) & static_cast<unsigned int>(Error::MOVE_INTERRUPTED)) != 0U) {
      append("Movement was interrupted");
    }
    if((static_cast<unsigned int>(error) & static_cast<unsigned int>(Error::EMERGENCY_STOP)) != 0U) {
      append("Emergency stop");
    }
    if(result.empty()) {
      return "Unknown error (" + std::to_string(static_cast<unsigned int>(error)) + ")";
    }
    return result;
  }
} // namespace ChimeraTK::MotorDriver

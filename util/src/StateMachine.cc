/*
 * StateMachine.cc
 *
 *  Created on: Feb 16, 2017
 *      Author: vitimic
 */

#include "StateMachine.h"
#include "StepperMotorUtil.h"
#include <cassert>

namespace ChimeraTK { namespace MotorDriver { namespace utility {

  bool operator<(const StateMachine::Event& event1, const StateMachine::Event& event2) {
    return event1._eventName < event2._eventName;
  }

  StateMachine::State::TransitionData::TransitionData(
      State* target, std::function<void(void)> entryCallback, std::function<void(void)> internalCallback)
  : targetState(target), entryCallbackAction(entryCallback), internalCallbackAction(internalCallback) {}

  StateMachine::State::TransitionData::TransitionData(const State::TransitionData& targetAndAction)
  : targetState(targetAndAction.targetState), entryCallbackAction(targetAndAction.entryCallbackAction),
    internalCallbackAction(targetAndAction.internalCallbackAction) {}

  StateMachine::State::State(std::string stateName) : _stateName(stateName), _transitionTable() {}

  StateMachine::State::~State() {}

  void StateMachine::State::setTransition(
      Event event, State* target, std::function<void(void)> entryCallback, std::function<void(void)> internalCallback) {
    TransitionData transitionData(target, entryCallback, internalCallback);
    _transitionTable.insert(std::pair<Event, TransitionData>(event, transitionData));
  }

  const StateMachine::State::TransitionTable& StateMachine::State::getTransitionTable() const {
    return _transitionTable;
  }

  std::string StateMachine::State::getName() const { return _stateName; }

  StateMachine::StateMachine()
  : _initState("initState"), _endState("endState"), _currentState(&_initState), _requestedState(nullptr),
    _stateMachineMutex(), _asyncActionActive(false), _internalEventCallback([] {}), _requestedInternalCallback([] {}) {}

  StateMachine::~StateMachine() {}

  StateMachine::State* StateMachine::getCurrentState() {
    std::lock_guard<std::mutex> lck(_stateMachineMutex);

    _internalEventCallback();
    return _currentState;
  }

  void StateMachine::setAndProcessUserEvent(Event event) {
    std::lock_guard<std::mutex> lck(_stateMachineMutex);
    performTransition(event);
  }

  void StateMachine::performTransition(Event event) {
    std::map<Event, State::TransitionData>::const_iterator it;

    const State::TransitionTable& transitionTable = _currentState->getTransitionTable();
    it = transitionTable.find(event);
    if(it != transitionTable.end()) {
      _requestedState = ((it->second).targetState);
      _requestedInternalCallback = (it->second).internalCallbackAction;

      // Apply new state right away, if no async action active
      if(!_asyncActionActive.load()) {
        _currentState = _requestedState;
        _requestedState = nullptr;
        _internalEventCallback = _requestedInternalCallback;
      }
      (it->second).entryCallbackAction();
    }
  }

  bool StateMachine::hasRequestedState() { return _requestedState != nullptr; }

  void StateMachine::moveToRequestedState() {
    if(_requestedState != nullptr) {
      _currentState = _requestedState;
      _internalEventCallback = _requestedInternalCallback;
      _requestedState = nullptr;
    }
  }
  }
  std::string toString(ExitStatus& status) {
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

  std::string toString(Error& error) {
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
        assert(false);
        return ("Unknown error");
    }
  }
}} // namespace ChimeraTK::MotorDriver

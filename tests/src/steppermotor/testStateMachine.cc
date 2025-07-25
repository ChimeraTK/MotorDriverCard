
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE StateMachineTest
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "StateMachine.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

using namespace ChimeraTK::MotorDriver::utility;

class DerivedStateMachine : public StateMachine {
 public:
  DerivedStateMachine();
  ~DerivedStateMachine() override = default;

 protected:
  static StateMachine::Event initEvent;
  static StateMachine::Event userEvent1;
  static StateMachine::Event userEvent2;
  static StateMachine::Event userEvent3;

  StateMachine::State _firstState;
  StateMachine::State _secondState;
  StateMachine::State _thirdState;
  StateMachine::State _fourthState;
  StateMachine::State _fifthState;
  StateMachine::Event _state1to2Event;
  StateMachine::Event _state2to3Event;
  StateMachine::Event _state3to1Event;
  StateMachine::Event _state1to4Event;
  void actionIdleToFirstState();
  void actionFirstToSecondState();
  void actionFirstStateExit();
  void actionThirdToFirstState();
  void actionFirstToFourthState();
  void actionExtern1();
  void actionExtern2();

  // This flag allows to emulate external events,
  // in a real application, we would wait for e.g. halt of a motor
  std::atomic<bool> _transitionAllowed;

  // Atomic variables to report properties between main thread
  // and async task
  std::atomic<bool> _isCorrectCurrentState;
  std::atomic<bool> _isCorrectRequestedState;
  std::atomic<int> _counter;

 private:
  // To be used inside the callbacks which are guarded, so no locking here
  void assertRequestedState(StateMachine::State* referenceState);
};

StateMachine::Event DerivedStateMachine::initEvent("initEvent");
StateMachine::Event DerivedStateMachine::userEvent1("userEvent1");
StateMachine::Event DerivedStateMachine::userEvent2("userEvent2");
StateMachine::Event DerivedStateMachine::userEvent3("userEvent3");

DerivedStateMachine::DerivedStateMachine()
: _firstState("firstState"), _secondState("secondState"), _thirdState("thirdState"), _fourthState("fourthState"),
  _fifthState("fifthState"), _state1to2Event("changeState1"), _state2to3Event("changeState2"),
  _state3to1Event("changeState3"), _state1to4Event("changeState4"), _transitionAllowed(false),
  _isCorrectCurrentState(false), _isCorrectRequestedState(false), _counter(0) {
  _initState.setTransition(
      DerivedStateMachine::initEvent, &_firstState, [this] { actionIdleToFirstState(); },
      [this] { actionFirstStateExit(); });
  _firstState.setTransition(DerivedStateMachine::userEvent1, &_thirdState, [this] { actionExtern1(); });
  _firstState.setTransition(_state1to2Event, &_secondState, [this] { actionFirstToSecondState(); });
  _firstState.setTransition(_state1to4Event, &_fourthState, [this] { actionFirstToFourthState(); });
  _secondState.setTransition(DerivedStateMachine::userEvent2, &_firstState, [this] { actionExtern2(); });
  _thirdState.setTransition(DerivedStateMachine::userEvent3, &_firstState, [this] { actionThirdToFirstState(); });
}

void DerivedStateMachine::assertRequestedState(StateMachine::State* referenceState) {
  if(_requestedState == nullptr) {
    _isCorrectRequestedState.exchange(referenceState == nullptr);
  }
  else {
    _isCorrectRequestedState.exchange(
        referenceState != nullptr && referenceState->getName() == _requestedState->getName());
  }
}

void DerivedStateMachine::actionIdleToFirstState() {
  _asyncActionActive.exchange(true);
  std::thread asyncActionIdleToFirst([this] {
    std::lock_guard<std::mutex> lck(_stateMachineMutex);
    performTransition(_state1to2Event);
    _asyncActionActive.exchange(false);
  });
  asyncActionIdleToFirst.detach();
}

void DerivedStateMachine::actionFirstToSecondState() {
  // Wait for main test thread, check current and requested state
  if(_currentState->getName() == "firstState") {
    _isCorrectCurrentState.exchange(true);
  }
  else {
    _isCorrectCurrentState.exchange(false);
  }
  assertRequestedState(&_secondState);

  _transitionAllowed.exchange(false);
  while(!_transitionAllowed.load()) {
  }

  _transitionAllowed.exchange(false);
}

void DerivedStateMachine::actionFirstStateExit() {
  moveToRequestedState();
  // Requested state should now be reset
  assertRequestedState(nullptr);
}

void DerivedStateMachine::actionExtern1() {
  _counter.fetch_add(1);
}

void DerivedStateMachine::actionExtern2() {
  _counter.fetch_add(2);
}

void DerivedStateMachine::actionThirdToFirstState() {
  _asyncActionActive.exchange(true);
  std::thread asyncAction3rdTo1st([this] {
    std::lock_guard<std::mutex> lck(_stateMachineMutex);
    performTransition(_state1to4Event);
    _asyncActionActive.exchange(false);
  });
  asyncAction3rdTo1st.detach();

  _counter.fetch_add(31);
}

void DerivedStateMachine::actionFirstToFourthState() {
  _counter.fetch_add(14);
  moveToRequestedState();
}

BOOST_AUTO_TEST_SUITE(StateMachineTestSuite)

// Test of the base state machine provided by the StateMachine class
BOOST_FIXTURE_TEST_CASE(testBaseStateMachine, StateMachine) {
  BOOST_CHECK_EQUAL(getCurrentState()->getName(), "initState");

  // Default-constructed event should be undefined
  StateMachine::Event undefinedEvent("undefinedEvent");
  BOOST_CHECK_NO_THROW(setAndProcessUserEvent(undefinedEvent));
  BOOST_CHECK_EQUAL(getCurrentState()->getName(), "initState");
}

BOOST_FIXTURE_TEST_CASE(testDerivedStateMachine, DerivedStateMachine) {
  _transitionAllowed.exchange(true);

  // noEvent triggers transition from idle to first state
  BOOST_CHECK_EQUAL(getCurrentState()->getName(), "initState");
  BOOST_CHECK_NO_THROW(setAndProcessUserEvent(initEvent));

  // Wait until the async task sets the flag to false
  while(_transitionAllowed.load()) {
  }

  // Now, we should be in first state, second state should be requested
  // (the flags are computed in actionFirstToSecondState, before the waiting)
  BOOST_CHECK(_isCorrectCurrentState.load());
  BOOST_CHECK(_isCorrectRequestedState.load());

  // Let async task transition to second state
  _transitionAllowed.exchange(true);

  // Wait for the async task to finish, we should then be in second state and
  // the requested state pointer should be reset
  while(_asyncActionActive.load()) {
  }
  BOOST_CHECK_EQUAL(getCurrentState()->getName(), "firstState");
  BOOST_CHECK(_isCorrectRequestedState.load());

  BOOST_CHECK_EQUAL(getCurrentState()->getName(), "secondState");
  BOOST_CHECK(_isCorrectRequestedState.load());

  // Invoke event to move back to 1st state
  _counter.store(0);
  BOOST_CHECK_NO_THROW(setAndProcessUserEvent(userEvent2));
  BOOST_CHECK_EQUAL(getCurrentState()->getName(), "firstState");

  // Transition from 1st to 3rd state
  BOOST_CHECK_NO_THROW(setAndProcessUserEvent(userEvent1));
  BOOST_CHECK_EQUAL(getCurrentState()->getName(), "thirdState");
  BOOST_CHECK_EQUAL(_counter.load(), 3);

  // Start transition from 3rd, via 1st, to 4th state
  // In contrast to the sequence idle->1st->2nd above, this processes
  // the internal event without interruption
  _counter.store(0);
  BOOST_CHECK_NO_THROW(setAndProcessUserEvent(DerivedStateMachine::userEvent3));

  while(_asyncActionActive.load() == true) {
  }

  BOOST_CHECK_EQUAL(getCurrentState()->getName(), "fourthState");
  BOOST_CHECK_EQUAL(_counter.load(), 45);

} // BOOST_FIXTURE_TEST_CASE( testDerivedStateMachine, DerivedStateMachine )

BOOST_AUTO_TEST_SUITE_END()

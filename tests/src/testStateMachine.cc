/*
 * testStateMachine.cc
 *
 *  Created on: Feb 22, 2017
 *      Author: vitimic
 */

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#ifdef MOTORDRIVERCARD_HELGRIND_DEBUG
  #include "helgrind.h"
#endif

#include "StateMachine.h"
#include <functional>
#include <memory>
#include <atomic>
#include <mutex>

#ifdef MOTORDRIVERCARD_HELGRIND_DEBUG
  #define HG_ANNOTATE_HAPPENS_BEFORE(VAR) ANNOTATE_HAPPENS_BEFORE(&VAR);
  #define HG_ANNOTATE_HAPPENS_AFTER(VAR)  ANNOTATE_HAPPENS_AFTER(&VAR);
#else
  #define HG_ANNOTATE_HAPPENS_BEFORE(VAR)
  #define HG_ANNOTATE_HAPPENS_AFTER(VAR)
#endif

class TestStateMachine;

class DerivedStateMachine : public ChimeraTK::StateMachine{

  friend class TestStateMachine;

public:
  DerivedStateMachine();
  ~DerivedStateMachine();

protected:
  ChimeraTK::State _firstState;
  ChimeraTK::State _secondState;
  ChimeraTK::State _thirdState;
  ChimeraTK::State _fourthState;
  ChimeraTK::State _fifthState;
  ChimeraTK::Event _state1to2Event;
  ChimeraTK::Event _state2to3Event;
  ChimeraTK::Event _state3to1Event;
  ChimeraTK::Event _state1to4Event;
  ChimeraTK::Event _state4to5Event;
  void actionIdleToFirstState();
  void actionFirstToSecondState();
  void actionThirdToFirstState();
  void actionFirstToFourthState();
  void actionFourthToFifthState();
  void actionExtern1();
  void actionExtern2();
  // This flag allows to emulate external events,
  // in a real application, we would wait for e.g. halt of a motor
  std::atomic<bool> _transitionAllowed;

  // Atomic variables to report properties between main thread
  // and async task
  std::atomic<bool> _isCorrectCurrentState;
  std::atomic<bool> _isCorrectRequestedState;
  std::atomic<int>  _counter;

private:
  // To be used inside the callbacks which are guarded, so no locking here
  void assertRequestedState(ChimeraTK::State* referenceState);

};


//class StateMachineComplete : public DerivedStateMachine{
//public:
//  StateMachineComplete();
//  ~StateMachineComplete();
//
//  friend class TestStateMachine;
//private:
//  ChimeraTK::State _firstUp;
//  ChimeraTK::State _secondUp;
//  ChimeraTK::Event _fromSubStateMachineToFirstUp;
//  ChimeraTK::Event _fromFirstUpToSecondUp;
//  ChimeraTK::Event _fromSecondUpToSubStateMachine;
//};

class TestStateMachine{
public:
  TestStateMachine();
  void testBaseStateMachine();
  void testDerivedStateMachine();
  void testSubStateMachine();
  static ChimeraTK::Event userEvent1;
  static ChimeraTK::Event userEvent2;

private:
  std::unique_ptr<ChimeraTK::StateMachine> _baseStateMachine;
  DerivedStateMachine _derivedStateMachine;
  //StateMachineComplete _stateMachineComplete;
};


ChimeraTK::Event TestStateMachine::userEvent1("userEvent1");
ChimeraTK::Event TestStateMachine::userEvent2("userEvent2");

DerivedStateMachine::DerivedStateMachine() :
    StateMachine(),
    _firstState("firstState"),
    _secondState("secondState"),
    _thirdState("thirdState"),
    _fourthState("fourthState"),
    _fifthState("fifthState"),
    _state1to2Event("changeState1"),
    _state2to3Event("changeState2"),
    _state3to1Event("changeState3"),
    _state1to4Event("changeState4"),
    _state4to5Event("changeState5"),
    _transitionAllowed(false),
    _isCorrectCurrentState(false),
    _isCorrectRequestedState(false),
    _counter(0)
{

  _initState.setTransition(ChimeraTK::StateMachine::noEvent, &_firstState, std::bind(&DerivedStateMachine::actionIdleToFirstState, this));

  _firstState.setTransition(TestStateMachine::userEvent1, &_thirdState, std::bind(&DerivedStateMachine::actionExtern1, this));
  _firstState.setTransition(_state1to2Event, &_secondState, std::bind(&DerivedStateMachine::actionFirstToSecondState, this));
  _firstState.setTransition(_state1to4Event, &_forthState, std::bind(&DerivedStateMachine::actionFirstToForthState, this));
  _fourthState.setTransition(_state4to5Event, &_fifthState, std::bind(&DerivedStateMachine::actionFourthToFifthState, this));

  _secondState.setTransition(TestStateMachine::userEvent2, &_firstState, std::bind(&DerivedStateMachine::actionExtern2, this));

  _thirdState.setTransition(_state3to1Event, &_firstState, std::bind(&DerivedStateMachine::actionThirdToFirstState, this));
}

DerivedStateMachine::~DerivedStateMachine(){}

void DerivedStateMachine::assertRequestedState(ChimeraTK::State* referenceState){

  if(_requestedState == nullptr){
    _isCorrectRequestedState.exchange(referenceState == nullptr);
  }
  else{
    _isCorrectRequestedState.exchange(referenceState != nullptr &&
                                      referenceState->getName() == _requestedState->getName());
  }
}

void DerivedStateMachine::actionIdleToFirstState(){

  if(!_asyncActionActive.valid()
     || (_asyncActionActive.valid()
         && _asyncActionActive.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){

    _asyncActionActive = std::async(std::launch::async, [this]{performTransition(_state1to2Event);});
  }
}

void DerivedStateMachine::actionFirstToSecondState(){
  //std::cout << "  ** Performing callback \"actionFristToSecondState\"" << std::endl;

  // Wait for main test thread, check current and requested state
  HG_ANNOTATE_HAPPENS_BEFORE(_transitionAllowed)

  if(_currentState->getName() == "firstState"){
    _isCorrectCurrentState.exchange(true);
  }
  else{
    _isCorrectCurrentState.exchange(false);
  }
  assertRequestedState(&_secondState);

  _transitionAllowed.exchange(false);
  while(!_transitionAllowed){}
  HG_ANNOTATE_HAPPENS_AFTER(_transitionAllowed)


  //std::cout << "    **** Registered _transitionAllowed flag in \"actionIdleToFirstState\"" << std::endl;
  moveToRequestedState();

  _transitionAllowed.exchange(false);

  // Requested state should now be reset
  assertRequestedState(nullptr);

}

void DerivedStateMachine::actionExtern1(){
  _counter.fetch_add(1);
}

void DerivedStateMachine::actionExtern2(){
  _counter.fetch_add(2);
}

void DerivedStateMachine::actionThirdToFirstState(){
  if(!_asyncActionActive.valid()
     || (_asyncActionActive.valid()
         && _asyncActionActive.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
    _asyncActionActive = std::async(std::launch::async, [this]{performTransition(_state1to4Event);});
  }
  _counter.fetch_add(31);
}

void DerivedStateMachine::actionFirstToFourthState(){
  _counter.fetch_add(14);
  moveToRequestedState();
  performTransition(_state4to5Event);
}

void DerivedStateMachine::actionFourthToFifthState(){
  _counter.fetch_add(45);
  moveToRequestedState();
}

//bool DerivedStateMachine::propagateEvent(){
//  if (_currentState->getName() == "firstState" && _currentState->isEventUnknown() == true){
//    return true;
//  }else{
//    return false;
//  }
//}

//StateMachineComplete::StateMachineComplete() : DerivedStateMachine(),
//    _firstUp("firstUp"),
//    _secondUp("secondUp"),
//    _fromSubStateMachineToFirstUp("fromSubStateMachineToFirstUp"),
//    _fromFirstUpToSecondUp("fromFirstUpToSecondUp"),
//    _fromSecondUpToSubStateMachine("fromSecondUpToSubStateMachine"){
//  _initState.setTransition(ChimeraTK::StateMachine::noEvent, &_initState, [](){});
//  _firstState.setTransition(_fromSubStateMachineToFirstUp, &_firstUp, [](){});
//  _firstUp.setTransition(_fromFirstUpToSecondUp, &_secondUp, [](){});
//  _secondUp.setTransition(_fromSecondUpToSubStateMachine, &_firstState, [](){});
//}
//
//StateMachineComplete::~StateMachineComplete(){}

TestStateMachine::TestStateMachine() :
    _baseStateMachine(new ChimeraTK::StateMachine()),
    _derivedStateMachine(){}
    //_stateMachineComplete(){}


// Test of the base state machine provided by the StateMachine class
void TestStateMachine::testBaseStateMachine(){
  BOOST_CHECK(_baseStateMachine->getCurrentState()->getName()=="initState") ;

  // Default-constructed event should be undefined
  ChimeraTK::Event undefinedEvent;
  BOOST_CHECK(undefinedEvent == ChimeraTK::StateMachine::undefinedEvent);
  BOOST_CHECK_NO_THROW(_baseStateMachine->setAndProcessUserEvent(undefinedEvent));
  BOOST_CHECK_EQUAL(_baseStateMachine->getCurrentState()->getName(), "initState") ;
  BOOST_CHECK(_baseStateMachine->_isEventUnknown);
}

void TestStateMachine::testDerivedStateMachine(){

  HG_ANNOTATE_HAPPENS_BEFORE(_derivedStateMachine._transitionAllowed)
  _derivedStateMachine._transitionAllowed.exchange(true);

  // noEvent triggers transition from idle to first state
  BOOST_CHECK_EQUAL(_derivedStateMachine.getCurrentState()->getName(), "initState");
  BOOST_CHECK_NO_THROW(_derivedStateMachine.setAndProcessUserEvent(ChimeraTK::StateMachine::noEvent));

  // Wait until the async task sets the flag to false
  while( _derivedStateMachine._transitionAllowed){}
  HG_ANNOTATE_HAPPENS_AFTER(_derivedStateMachine._transitionAllowed)

  // Now, we should be in first state, second state should be requested
  // (the flags are computed in actionFirstToSecondState, before the waiting)
  BOOST_CHECK(_derivedStateMachine._isCorrectCurrentState.load());
  BOOST_CHECK(_derivedStateMachine._isCorrectRequestedState.load());

  // Let async task transition to second state
  HG_ANNOTATE_HAPPENS_BEFORE(_derivedStateMachine._transitionAllowed)
  _derivedStateMachine._transitionAllowed.exchange(true);

  // Wait for the async task to finish, we should then be in second state and
  // the requested state pointer should be reset
  _derivedStateMachine._asyncActionActive.get();
  BOOST_CHECK(_derivedStateMachine._isCorrectRequestedState.load());
  BOOST_CHECK_EQUAL(_derivedStateMachine.getCurrentState()->getName(), "secondState");


  // Invoke event to move back to first state
  _derivedStateMachine._counter.store(0);
  BOOST_CHECK_NO_THROW(_derivedStateMachine.setAndProcessUserEvent(userEvent2));
  BOOST_CHECK_EQUAL(_derivedStateMachine.getCurrentState()->getName(), "firstState");


  BOOST_CHECK_NO_THROW(_derivedStateMachine.setAndProcessUserEvent(userEvent1));
  BOOST_CHECK_EQUAL(_derivedStateMachine.getCurrentState()->getName(), "thirdState");
  BOOST_CHECK_EQUAL(_derivedStateMachine._counter.load(), 3);

  //TODO
  _derivedStateMachine._counter.store(0);
  BOOST_CHECK_NO_THROW(_derivedStateMachine.setAndProcessUserEvent(__state3to1Event));

//
//  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.setAndProcessUserEvent(TestStateMachine::userEvent1));
//  BOOST_CHECK(_stateMachineTransitionTable._internEvent == ChimeraTK::StateMachine::noEvent);
//  BOOST_CHECK(_derivedStateMachine.getCurrentState()->getName() == "secondState");
//  BOOST_CHECK(_derivedStateMachine.getCurrentState()->isEventUnknown() == true);
//
//  BOOST_CHECK(_derivedStateMachine.getInt() == 2);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 1);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//  BOOST_CHECK(_derivedStateMachine._internEvent == ChimeraTK::StateMachine::noEvent);
//
//  BOOST_CHECK(_derivedStateMachine.getInt() == 2);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 2);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//  BOOST_CHECK(_derivedStateMachine._internEvent == ChimeraTK::StateMachine::noEvent);
//
//  BOOST_CHECK(_derivedStateMachine.getInt() == 2);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 3);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//  BOOST_CHECK(_derivedStateMachine._internEvent == _derivedStateMachine._state2to3Event);
//
//  BOOST_CHECK(_derivedStateMachine.getInt() == 3);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 4);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//  BOOST_CHECK(_derivedStateMachine._internEvent == ChimeraTK::StateMachine::noEvent);
//
//  BOOST_CHECK(_derivedStateMachine.getCurrentState()->getName() == "thirdState");
//  BOOST_CHECK(_derivedStateMachine.getInt() == 3);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 0);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//  BOOST_CHECK(_derivedStateMachine._internEvent == ChimeraTK::StateMachine::noEvent);
//
//  BOOST_CHECK(_derivedStateMachine.getInt() == 3);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 1);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//  BOOST_CHECK(_derivedStateMachine._internEvent == ChimeraTK::StateMachine::noEvent);
//
//  BOOST_CHECK(_derivedStateMachine.getInt() == 3);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 2);
////  _derivedStateMachine.setUserEvent(TestStateMachine::userEvent1);
////  BOOST_CHECK(_derivedStateMachine._userEvent == TestStateMachine::userEvent1);
//  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.setAndProcessUserEvent(TestStateMachine::userEvent1));
//  BOOST_CHECK(_stateMachineTransitionTable._userEvent == ChimeraTK::StateMachine::noEvent);
//  BOOST_CHECK(_derivedStateMachine.getCurrentState()->getName() == "thirdState");
//  BOOST_CHECK(_derivedStateMachine.getCurrentState()->isEventUnknown() == true);
//
//  BOOST_CHECK(_derivedStateMachine.getInt() == 3);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 2);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//  BOOST_CHECK(_derivedStateMachine._internEvent == _derivedStateMachine._state3to1Event);
//
//  BOOST_CHECK(_derivedStateMachine.getInt() == 1);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 3);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//  BOOST_CHECK(_derivedStateMachine._internEvent == ChimeraTK::StateMachine::noEvent);
//
//  BOOST_CHECK(_derivedStateMachine.getCurrentState()->getName() == "firstState");
//  BOOST_CHECK(_derivedStateMachine.getInt() == 1);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 0);
////  _derivedStateMachine.setUserEvent(TestStateMachine::userEvent1);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.setAndProcessUserEvent(TestStateMachine::userEvent1));
//  BOOST_CHECK(_stateMachineTransitionTable._userEvent == ChimeraTK::StateMachine::noEvent);
//
//  BOOST_CHECK(_derivedStateMachine.getCurrentState()->getName() == "thirdState");
//  BOOST_CHECK(_derivedStateMachine.getInt() == 100);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 0);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//  BOOST_CHECK(_derivedStateMachine._internEvent == ChimeraTK::StateMachine::noEvent);
//
//  BOOST_CHECK(_derivedStateMachine.getInt() == 100);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 1);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//  BOOST_CHECK(_derivedStateMachine._internEvent == ChimeraTK::StateMachine::noEvent);
//
//  BOOST_CHECK(_derivedStateMachine.getInt() == 100);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 2);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//  BOOST_CHECK(_derivedStateMachine._internEvent == _derivedStateMachine._state3to1Event);
//
//  BOOST_CHECK(_derivedStateMachine.getInt() == 1);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 3);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//
//  BOOST_CHECK(_derivedStateMachine.getCurrentState()->getName() == "firstState");
//  BOOST_CHECK(_derivedStateMachine.getInt() == 1);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 0);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//
//  BOOST_CHECK(_derivedStateMachine.getInt() == 1);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 1);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//
//  BOOST_CHECK(_derivedStateMachine.getInt() == 2);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 2);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.processEvent());
//
//  BOOST_CHECK(_derivedStateMachine.getCurrentState()->getName() == "secondState");
//  BOOST_CHECK(_derivedStateMachine.getInt() == 2);
//  BOOST_CHECK(_derivedStateMachine.getCount() == 0);
//  //_derivedStateMachine.setUserEvent(TestStateMachine::userEvent2);
//  BOOST_CHECK_NO_THROW(_derivedStateMachine.setAndProcessUserEvent(TestStateMachine::userEvent2));
//  BOOST_CHECK(_stateMachineTransitionTable._userEvent == ChimeraTK::StateMachine::noEvent);
//  BOOST_CHECK(_derivedStateMachine.getCurrentState()->getName() == "firstState");
}
//} // namespace ChimeraTK

//void TestStateMachine::testSubStateMachine(){
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "initState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "initState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "firstState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "firstState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "firstState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "secondState");
//  _stateMachineComplete.setUserEvent(TestStateMachine::userEvent2);
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "firstState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "firstState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "firstState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "secondState");
//  BOOST_CHECK(_stateMachineComplete.getInt() == 2);
//  BOOST_CHECK(_stateMachineComplete.getCount() == 0);
//  _stateMachineComplete.setUserEvent(_stateMachineComplete._fromSubStateMachineToFirstUp);
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->isEventUnknown() == true);
//  //BOOST_CHECK(_stateMachineComplete.isEventUnknown() == false);
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "secondState");
//  BOOST_CHECK(_stateMachineComplete.getInt() == 2);
//  BOOST_CHECK(_stateMachineComplete.getCount() == 0);
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "secondState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "secondState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "secondState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "secondState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "thirdState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "thirdState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "thirdState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "thirdState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "firstState");
//  _stateMachineComplete.setUserEvent(_stateMachineComplete._fromSubStateMachineToFirstUp);
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "firstUp");
//  _stateMachineComplete.setUserEvent(_stateMachineComplete._fromFirstUpToSecondUp);
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "secondUp");
//  _stateMachineComplete.setUserEvent(_stateMachineComplete._fromSecondUpToSubStateMachine);
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "firstState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "firstState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "firstState");
//  BOOST_CHECK_NO_THROW(_stateMachineComplete.processEvent());
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "StateMachineSubStateMachine");
//  BOOST_CHECK(_stateMachineComplete.getCurrentState()->getName() == "secondState");
//}


class StateMachineTestSuite : public test_suite{
public:
  StateMachineTestSuite() : test_suite("State Machine Suite"){
    boost::shared_ptr</*ChimeraTK::*/TestStateMachine> myTestStateMachine(new /*ChimeraTK::*/TestStateMachine());
    add(BOOST_CLASS_TEST_CASE(&/*ChimeraTK::*/TestStateMachine::testBaseStateMachine, myTestStateMachine));
    add(BOOST_CLASS_TEST_CASE(&/*ChimeraTK::*/TestStateMachine::testDerivedStateMachine, myTestStateMachine));
  }
};

test_suite*
init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
  framework::master_test_suite().p_name.value = "State Machine Test Suite";
  return new StateMachineTestSuite;
}


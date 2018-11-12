/*
 * testStateMachine.cc
 *
 *  Created on: Feb 22, 2017
 *      Author: vitimic
 */

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "StateMachine.h"
#include <functional>


class TestStateMachine;

class DerivedStateMachine : public ChimeraTK::StateMachine{
public:
  DerivedStateMachine();
  ~DerivedStateMachine();


  friend class TestStateMachine;
protected:
  ChimeraTK::State _firstState;
  ChimeraTK::State _secondState;
  ChimeraTK::State _thirdState;
  ChimeraTK::Event _state1to2Event;
  ChimeraTK::Event _state2to3Event;
  ChimeraTK::Event _state3to1Event;
  void actionIdleToFirstState();
  void actionFirstToSecondState();
  void action22();
  void actionSecondToThirdState();
  void action33();
  void action31();
  void actionExtern1();
  void actionExtern2();
  bool _transitionAllowed;
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
  ChimeraTK::StateMachine _baseStateMachine;
  DerivedStateMachine _derivedStateMachine;
  void assertRequestedState(ChimeraTK::StateMachine& stateMachine, ChimeraTK::State* state);
  //StateMachineComplete _stateMachineComplete;
};

class StateMachineTestSuite : public test_suite{
public:
  StateMachineTestSuite() : test_suite("State Machine Suite"){
    boost::shared_ptr<TestStateMachine> myTestStateMachine(new TestStateMachine());
    add(BOOST_CLASS_TEST_CASE(&TestStateMachine::testBaseStateMachine, myTestStateMachine));
    add(BOOST_CLASS_TEST_CASE(&TestStateMachine::testDerivedStateMachine, myTestStateMachine));
  }
};

ChimeraTK::Event TestStateMachine::userEvent1("userEvent1");
ChimeraTK::Event TestStateMachine::userEvent2("userEvent2");

DerivedStateMachine::DerivedStateMachine() :
    StateMachine(),
    _firstState("firstState"),
    _secondState("secondState"),
    _thirdState("thirdState"),
    _state1to2Event("changeState1"),
    _state2to3Event("changeState2"),
    _state3to1Event("changeState3"),
    _transitionAllowed(false)
{

  _initState.setTransition(ChimeraTK::StateMachine::noEvent, &_firstState, actionIdleToFirstState);

  _firstState.setTransition(TestStateMachine::userEvent1, &_thirdState, std::bind(&DerivedStateMachine::actionExtern1, this));
  _firstState.setTransition(_state1to2Event, &_secondState, std::bind(&DerivedStateMachine::actionFirstToSecondState, this));

  _secondState.setTransition(TestStateMachine::userEvent2, &_firstState, std::bind(&DerivedStateMachine::actionExtern2, this));
  _secondState.setTransition(_state2to3Event, &_thirdState, std::bind(&DerivedStateMachine::actionSecondToThirdState, this));

  _thirdState.setTransition(_state3to1Event, &_firstState, std::bind(&DerivedStateMachine::action31, this));
}

DerivedStateMachine::~DerivedStateMachine(){}

void DerivedStateMachine::actionIdleToFirstState(){

  if(!_asyncActionActive.valid()
     || (_asyncActionActive.valid()
         && _asyncActionActive.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
    std::async(std::launch::async, [this]{
                                            _transitionAllowed = false;
                                            // Trigger internal event
                                            // this should not perform a transition, as the async action is running
                                            performTransition(_state1to2Event);
                                         });
  }
}

void DerivedStateMachine::actionFirstToSecondState(){
  while(!_transitionAllowed){}
  moveToRequestedState();
  _transitionAllowed = false;
}

void DerivedStateMachine::actionExtern1(){
//  _int = 100;
//  _count = 0;
}

void DerivedStateMachine::action22(){
//  if (_count >= 3){
//    _int = 3;

    if(!_asyncActionActive.valid()
       || (_asyncActionActive.valid()
           && _asyncActionActive.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
      std::async(std::launch::async, [this]{performTransition(_state2to3Event);});
    }
  //}
  //_count++;
}

void DerivedStateMachine::actionSecondToThirdState(){
//  _count = 0;
}

void DerivedStateMachine::actionExtern2(){
//  _int = 200;
//  _count = 0;
}

void DerivedStateMachine::action33(){
//  if (_count >= 2){
//    _int = 1;

    if(!_asyncActionActive.valid()
       || (_asyncActionActive.valid()
           && _asyncActionActive.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
      std::async(std::launch::async, [this]{performTransition(_state3to1Event);});
    }
//  }
//  _count++;
}

void DerivedStateMachine::action31(){
//  _count = 0;
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

test_suite*
init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
  framework::master_test_suite().p_name.value = "State Machine Test Suite";
  return new StateMachineTestSuite;
}

TestStateMachine::TestStateMachine() :
    _baseStateMachine(),
    _derivedStateMachine(){}
    //_stateMachineComplete(){}

// FIXME Move to DerivedStatemachine class so that we can lock access
void TestStateMachine::assertRequestedState(ChimeraTK::StateMachine& stateMachine, ChimeraTK::State* state){

  ChimeraTK::State* reqState = nullptr;
  {
    std::lock_guard<std::mutex> lck(stateMachine._stateMachineMutex);
    reqState = stateMachine._requestedState;
  }

  if(reqState == nullptr){
    BOOST_CHECK(state == nullptr);
  }
  else{
    BOOST_CHECK(state != nullptr);
    BOOST_CHECK(reqState->getName() == state->getName());
  }
}

// Test of the base state machine provided by the StateMachine class
void TestStateMachine::testBaseStateMachine(){
  BOOST_CHECK(_baseStateMachine.getCurrentState()->getName()=="initState") ;

  // Default-constructed event should be undefined
  ChimeraTK::Event undefinedEvent;
  BOOST_CHECK(undefinedEvent == ChimeraTK::StateMachine::undefinedEvent);
  BOOST_CHECK_NO_THROW(_baseStateMachine.setAndProcessUserEvent(undefinedEvent));
  BOOST_CHECK(_baseStateMachine.getCurrentState()->getName()=="initState") ;
  BOOST_CHECK(_baseStateMachine._isEventUnknown == true) ;
}

void TestStateMachine::testDerivedStateMachine(){

  // noEvent triggers transition from idle to first state
  BOOST_CHECK(_derivedStateMachine.getCurrentState()->getName() == "initState");
  BOOST_CHECK_NO_THROW(_derivedStateMachine.setAndProcessUserEvent(ChimeraTK::StateMachine::noEvent));
  BOOST_CHECK(_derivedStateMachine.getCurrentState()->getName() == "firstState");

  // Transition to second state is triggered by an internal event
  // in async task when entering the first state, but waits for _transitionAllowed
  assertRequestedState(_derivedStateMachine, &_derivedStateMachine._secondState);
  _derivedStateMachine._transitionAllowed = true;

  while(_derivedStateMachine._asyncActionActive.valid()){}
  assertRequestedState(_derivedStateMachine, nullptr);
  BOOST_CHECK(_derivedStateMachine.getCurrentState()->getName() == "secondState");

;

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

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

class StateMachineSubStateMachine : public ChimeraTK::StateMachine{
public:
  StateMachineSubStateMachine();
  ~StateMachineSubStateMachine();
  int getInt(){return _int;}
  int getCount(){return _count;}

  friend class TestStateMachine;
protected:
  ChimeraTK::State _firstState;
  ChimeraTK::State _secondState;
  ChimeraTK::State _thirdState;
  ChimeraTK::Event _changeState1;
  ChimeraTK::Event _changeState2;
  ChimeraTK::Event _changeState3;
  void action11();
  void action12();
  void action22();
  void action23();
  void action33();
  void action31();
  void actionExtern1();
  void actionExtern2();
  int _int;
  uint32_t _count;
  bool propagateEvent();
};

class StateMachineComplete : public StateMachineSubStateMachine{
public:
  StateMachineComplete();
  ~StateMachineComplete();

  friend class TestStateMachine;
private:
  ChimeraTK::State _firstUp;
  ChimeraTK::State _secondUp;
  ChimeraTK::Event _fromSubStateMachineToFirstUp;
  ChimeraTK::Event _fromFirstUpToSecondUp;
  ChimeraTK::Event _fromSecondUpToSubStateMachine;
};

class TestStateMachine{
public:
  TestStateMachine();
  void testBaseStateMachine();
  void testTransitionTable();
  void testSubStateMachine();
  static ChimeraTK::Event userEvent1;
  static ChimeraTK::Event userEvent2;

private:
  ChimeraTK::StateMachine _baseStateMachine;
  StateMachineSubStateMachine _stateMachineTransitionTable;
  StateMachineComplete _stateMachineComplete;
};

class StateMachineTestSuite : public test_suite{
public:
  StateMachineTestSuite() : test_suite("State Machine Suite"){
    boost::shared_ptr<TestStateMachine> myTestStateMachine(new TestStateMachine());
    add(BOOST_CLASS_TEST_CASE(&TestStateMachine::testBaseStateMachine, myTestStateMachine));
    add(BOOST_CLASS_TEST_CASE(&TestStateMachine::testTransitionTable, myTestStateMachine));
  }
};

ChimeraTK::Event TestStateMachine::userEvent1("userEvent1");
ChimeraTK::Event TestStateMachine::userEvent2("userEvent2");

StateMachineSubStateMachine::StateMachineSubStateMachine() :
	    StateMachine(),
	    _firstState("firstState"),
	    _secondState("secondState"),
	    _thirdState("thirdState"),
	    _changeState1("changeState1"),
	    _changeState2("changeState2"),
	    _changeState3("changeState3"),
	    _int(1),
	    _count(0){
  _initState.setTransition(ChimeraTK::StateMachine::noEvent, &_firstState, [](){});

  _firstState.setTransition(ChimeraTK::StateMachine::noEvent, &_firstState, std::bind(&StateMachineSubStateMachine::action11, this));
  _firstState.setTransition(TestStateMachine::userEvent1, &_thirdState, std::bind(&StateMachineSubStateMachine::actionExtern1, this));
  _firstState.setTransition(_changeState1, &_secondState, std::bind(&StateMachineSubStateMachine::action12, this));

  _secondState.setTransition(ChimeraTK::StateMachine::noEvent, &_secondState, std::bind(&StateMachineSubStateMachine::action22, this));
  _secondState.setTransition(TestStateMachine::userEvent2, &_firstState, std::bind(&StateMachineSubStateMachine::actionExtern2, this));
  _secondState.setTransition(_changeState2, &_thirdState, std::bind(&StateMachineSubStateMachine::action23, this));

  _thirdState.setTransition(ChimeraTK::StateMachine::noEvent, &_thirdState, std::bind(&StateMachineSubStateMachine::action33, this));
  _thirdState.setTransition(_changeState3, &_firstState, std::bind(&StateMachineSubStateMachine::action31, this));
}

StateMachineSubStateMachine::~StateMachineSubStateMachine(){}

void StateMachineSubStateMachine::action11(){
  if (_count >= 1){
    _int = 2;
    _internEvent = _changeState1;
  }
  _count++;
}

void StateMachineSubStateMachine::action12(){
  _count = 0;
}

void StateMachineSubStateMachine::actionExtern1(){
  _int = 100;
  _count = 0;
}

void StateMachineSubStateMachine::action22(){
  if (_count >= 3){
    _int = 3;
    _internEvent = _changeState2;
  }
  _count++;
}

void StateMachineSubStateMachine::action23(){
  _count = 0;
}

void StateMachineSubStateMachine::actionExtern2(){
  _int = 200;
  _count = 0;
}

void StateMachineSubStateMachine::action33(){
  if (_count >= 2){
    _int = 1;
    _internEvent = _changeState3;
  }
  _count++;
}

void StateMachineSubStateMachine::action31(){
  _count = 0;
}

bool StateMachineSubStateMachine::propagateEvent(){
  if (_currentState->getName() == "firstState" && _currentState->isEventUnknown() == true){
    return true;
  }else{
    return false;
  }
}

StateMachineComplete::StateMachineComplete() : StateMachineSubStateMachine(),
    _firstUp("firstUp"),
    _secondUp("secondUp"),
    _fromSubStateMachineToFirstUp("fromSubStateMachineToFirstUp"),
    _fromFirstUpToSecondUp("fromFirstUpToSecondUp"),
    _fromSecondUpToSubStateMachine("fromSecondUpToSubStateMachine"){
  _initState.setTransition(ChimeraTK::StateMachine::noEvent, &_initState, [](){});
  _firstState.setTransition(_fromSubStateMachineToFirstUp, &_firstUp, [](){});
  _firstUp.setTransition(_fromFirstUpToSecondUp, &_secondUp, [](){});
  _secondUp.setTransition(_fromSecondUpToSubStateMachine, &_firstState, [](){});
}

StateMachineComplete::~StateMachineComplete(){}

test_suite*
init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
  framework::master_test_suite().p_name.value = "State Machine Test Suite";
  return new StateMachineTestSuite;
}

TestStateMachine::TestStateMachine() :
    _baseStateMachine(),
    _stateMachineTransitionTable(),
    _stateMachineComplete(){}

// Test of the base state machine provided by the StateMachine class
void TestStateMachine::testBaseStateMachine(){
  BOOST_CHECK(_baseStateMachine.getCurrentState()->getName()=="initState") ;
  BOOST_CHECK_NO_THROW(_baseStateMachine.processEvent());
  BOOST_CHECK(_baseStateMachine.getCurrentState()->getName()=="initState") ;
  BOOST_CHECK(_baseStateMachine.getCurrentState()->isEventUnknown()==true) ;
}

void TestStateMachine::testTransitionTable(){
  BOOST_CHECK(_stateMachineTransitionTable.getCurrentState()->getName() == "initState");
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());

  BOOST_CHECK(_stateMachineTransitionTable.getCurrentState()->getName() == "firstState");
  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 1);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 0);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 1);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 1);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 2);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 2);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());

  BOOST_CHECK(_stateMachineTransitionTable.getCurrentState()->getName() == "secondState");
  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 2);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 0);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 2);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 1);
  //_stateMachineTransitionTable.setUserEvent(TestStateMachine::userEvent1);
  //BOOST_CHECK(_stateMachineTransitionTable._userEvent == TestStateMachine::userEvent1);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.setAndProcessUserEvent(TestStateMachine::userEvent1));
  BOOST_CHECK(_stateMachineTransitionTable._internEvent == ChimeraTK::StateMachine::noEvent);
  BOOST_CHECK(_stateMachineTransitionTable.getCurrentState()->getName() == "secondState");
  BOOST_CHECK(_stateMachineTransitionTable.getCurrentState()->isEventUnknown() == true);

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 2);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 1);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());
  BOOST_CHECK(_stateMachineTransitionTable._internEvent == ChimeraTK::StateMachine::noEvent);

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 2);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 2);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());
  BOOST_CHECK(_stateMachineTransitionTable._internEvent == ChimeraTK::StateMachine::noEvent);

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 2);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 3);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());
  BOOST_CHECK(_stateMachineTransitionTable._internEvent == _stateMachineTransitionTable._changeState2);

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 3);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 4);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());
  BOOST_CHECK(_stateMachineTransitionTable._internEvent == ChimeraTK::StateMachine::noEvent);

  BOOST_CHECK(_stateMachineTransitionTable.getCurrentState()->getName() == "thirdState");
  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 3);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 0);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());
  BOOST_CHECK(_stateMachineTransitionTable._internEvent == ChimeraTK::StateMachine::noEvent);

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 3);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 1);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());
  BOOST_CHECK(_stateMachineTransitionTable._internEvent == ChimeraTK::StateMachine::noEvent);

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 3);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 2);
//  _stateMachineTransitionTable.setUserEvent(TestStateMachine::userEvent1);
//  BOOST_CHECK(_stateMachineTransitionTable._userEvent == TestStateMachine::userEvent1);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.setAndProcessUserEvent(TestStateMachine::userEvent1));
  BOOST_CHECK(_stateMachineTransitionTable._userEvent == ChimeraTK::StateMachine::noEvent);
  BOOST_CHECK(_stateMachineTransitionTable.getCurrentState()->getName() == "thirdState");
  BOOST_CHECK(_stateMachineTransitionTable.getCurrentState()->isEventUnknown() == true);

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 3);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 2);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());
  BOOST_CHECK(_stateMachineTransitionTable._internEvent == _stateMachineTransitionTable._changeState3);

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 1);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 3);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());
  BOOST_CHECK(_stateMachineTransitionTable._internEvent == ChimeraTK::StateMachine::noEvent);

  BOOST_CHECK(_stateMachineTransitionTable.getCurrentState()->getName() == "firstState");
  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 1);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 0);
//  _stateMachineTransitionTable.setUserEvent(TestStateMachine::userEvent1);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.setAndProcessUserEvent(TestStateMachine::userEvent1));
  BOOST_CHECK(_stateMachineTransitionTable._userEvent == ChimeraTK::StateMachine::noEvent);

  BOOST_CHECK(_stateMachineTransitionTable.getCurrentState()->getName() == "thirdState");
  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 100);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 0);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());
  BOOST_CHECK(_stateMachineTransitionTable._internEvent == ChimeraTK::StateMachine::noEvent);

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 100);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 1);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());
  BOOST_CHECK(_stateMachineTransitionTable._internEvent == ChimeraTK::StateMachine::noEvent);

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 100);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 2);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());
  BOOST_CHECK(_stateMachineTransitionTable._internEvent == _stateMachineTransitionTable._changeState3);

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 1);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 3);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());

  BOOST_CHECK(_stateMachineTransitionTable.getCurrentState()->getName() == "firstState");
  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 1);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 0);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 1);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 1);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());

  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 2);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 2);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.processEvent());

  BOOST_CHECK(_stateMachineTransitionTable.getCurrentState()->getName() == "secondState");
  BOOST_CHECK(_stateMachineTransitionTable.getInt() == 2);
  BOOST_CHECK(_stateMachineTransitionTable.getCount() == 0);
  //_stateMachineTransitionTable.setUserEvent(TestStateMachine::userEvent2);
  BOOST_CHECK_NO_THROW(_stateMachineTransitionTable.setAndProcessUserEvent(TestStateMachine::userEvent2));
  BOOST_CHECK(_stateMachineTransitionTable._userEvent == ChimeraTK::StateMachine::noEvent);
  BOOST_CHECK(_stateMachineTransitionTable.getCurrentState()->getName() == "firstState");
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

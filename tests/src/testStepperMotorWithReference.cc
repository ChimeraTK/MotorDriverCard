/*
 * testStepperMotorWithReference.cc
 *
 *  Created on: Mar 3, 2017
 *      Author: vitimic
 */

#include <boost/test/included/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/pointer_cast.hpp>
using namespace boost::unit_test_framework;

#include "StepperMotorWithReference.h"
#include "StepperMotorWithReferenceStateMachine.h"
#include "TMC429Constants.h"
#include "DFMC_MD22Dummy.h"
#include "MotorControlerDummy.h"
#include "MotorDriverCardFactory.h"
#include "MotorDriverException.h"
#include "StepperMotorException.h"
#include <ChimeraTK/DMapFilesParser.h>
#include <ChimeraTK/Utilities.h>


static const std::string stepperMotorDeviceName("STEPPER-MOTOR-DUMMY");
static const std::string stepperMotorDeviceConfigFile("VT21-MotorDriverCardConfig.xml");
static const std::string dmapPath(".");
static const std::string moduleName("");

namespace ChimeraTK{
  class StepperMotorWithReferenceTest{
  public:
    StepperMotorWithReferenceTest();
    void testCalibrate();
    void testCalibrateError();
    void testCalibrateStop();
    void testTranslation();
    void testDetermineTolerance();
    void testDetermineToleranceError();
    void testDetermineToleranceStop();
    void waitForCalibState();
    void waitForDetTolerance();
    void waitToSetPositiveTargetPos(int targetPos = 50000);
    void waitToSetNegativeTargetPos(int targetPos = -40000);
    void waitForMoveState();
    void waitForPositiveEndSwitchActive();
    void waitForNegativeEndSwitchActive();
    void waitForStop();
    bool waitForState(std::string stateName, unsigned timeoutInSeconds);
  private:
    boost::shared_ptr<ChimeraTK::StepperMotorWithReference> _stepperMotor;
    boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;
  };
}

using namespace ChimeraTK;

class StepperMotorWithReferenceTestSuite : public test_suite {
public:
  StepperMotorWithReferenceTestSuite() : test_suite("Stepper Motor With Reference Test Suite"){
    boost::shared_ptr<StepperMotorWithReferenceTest> myTest(new StepperMotorWithReferenceTest);
    add(BOOST_CLASS_TEST_CASE(&StepperMotorWithReferenceTest::testCalibrate, myTest));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorWithReferenceTest::testCalibrateError, myTest));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorWithReferenceTest::testCalibrateStop, myTest));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorWithReferenceTest::testTranslation, myTest));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorWithReferenceTest::testDetermineTolerance, myTest));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorWithReferenceTest::testDetermineToleranceError, myTest));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorWithReferenceTest::testDetermineToleranceStop, myTest));
  }
};

test_suite* init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
  framework::master_test_suite().p_name.value = "Stepper Motor with Reference test suite";
  return new StepperMotorWithReferenceTestSuite;
}

StepperMotorWithReferenceTest::StepperMotorWithReferenceTest() :
                    _stepperMotor(),
                    _motorControlerDummy()
{
  std::string deviceFileName(mtca4u::DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).deviceName);
  std::string mapFileName(mtca4u::DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).mapFileName);
  mtca4u::MotorDriverCardFactory::instance().setDummyMode();
  _motorControlerDummy = boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(deviceFileName,  moduleName, stepperMotorDeviceConfigFile)->getMotorControler(0));
}

bool StepperMotorWithReferenceTest::waitForState(std::string stateName, unsigned timeoutInSeconds = 10){

  unsigned sleepPeriodInUsec = 100000;
  unsigned timeoutInSteps    = timeoutInSeconds*1000000/sleepPeriodInUsec;
  unsigned cnt = 0;
  bool isCorrectState = false;
  do{

    usleep(sleepPeriodInUsec);

    if(_stepperMotor->getState() == stateName){
      isCorrectState = true;
      break;
    }
    else{
      cnt++;
    }
  }while(cnt <= timeoutInSteps);

  return isCorrectState;
}

void StepperMotorWithReferenceTest::waitForCalibState(){
  while (1){
    usleep(10);
    _stepperMotor->_mutex.lock();
    if ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "calibrating"){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitForDetTolerance(){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "calculatingTolerance"){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitToSetPositiveTargetPos(int targetPos){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if (_stepperMotor->_motorControler->getTargetPosition() != targetPos){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitToSetNegativeTargetPos(int targetPos){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if (_stepperMotor->_motorControler->getTargetPosition() != targetPos){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitForMoveState(){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if (_stepperMotor->_stateMachine->getCurrentState()->getName() == "movingState"){
      break;
    }
    _stepperMotor->_mutex.unlock();
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitForPositiveEndSwitchActive(){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if (!_stepperMotor->_motorControler->getReferenceSwitchData().getPositiveSwitchActive()){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitForNegativeEndSwitchActive(){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if (!_stepperMotor->_motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitForStop(){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if (std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotor->_stateMachine)->_stopAction != true){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTest::testCalibrate(){

  // Test calibration w/ disabled end switches
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);

  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));

  _stepperMotor->setEnabled(true);
  BOOST_CHECK(waitForState("idleState"));

  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);

  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeEndSwitchEnabled(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveEndSwitchEnabled(), false);

  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());

  BOOST_CHECK(waitForState("idleState"));
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->_calibrationFailed.load(), true);


  // Test calibration w/ disabled positive end switch
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(true);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotor->setEnabled(true);
  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeEndSwitchEnabled(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveEndSwitchEnabled(), false);
  BOOST_CHECK(waitForState("idleState"));

  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("idleState"));

  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->_calibrationFailed.load(), true);


  // Test calibration w/ disabled negative end switch
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotor->setEnabled(true);
  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeEndSwitchEnabled(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveEndSwitchEnabled(), true);
  BOOST_CHECK(waitForState("idleState"));
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("idleState"));
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->_calibrationFailed.load(), true);


  // Test calibration w/ end switches enabled
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(true);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotor->setEnabled(true);
  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeEndSwitchEnabled(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveEndSwitchEnabled(), true);
  BOOST_CHECK(waitForState("idleState"));
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("calibrating"));
  waitToSetPositiveTargetPos();
  _motorControlerDummy->moveTowardsTarget(1);
  waitForPositiveEndSwitchActive();
  waitToSetNegativeTargetPos();
  BOOST_CHECK(_stepperMotor->_calibPositiveEndSwitchInSteps == 10000);
  BOOST_CHECK(_stepperMotor->_motorControler->getActualPosition() == 10000);
  BOOST_CHECK(_stepperMotor->isPositiveReferenceActive() == true);
  BOOST_CHECK_THROW(_stepperMotor->setTargetPositionInSteps(0), ChimeraTK::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->moveRelativeInSteps(-100), ChimeraTK::MotorDriverException);
  BOOST_CHECK(_stepperMotor->_motorControler->getActualPosition() == 10000);
  _motorControlerDummy->moveTowardsTarget(1);

  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(_stepperMotor->_calibPositiveEndSwitchInSteps.load(), 20000);
  BOOST_CHECK_EQUAL(_stepperMotor->_calibNegativeEndSwitchInSteps.load(), 0);
  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeReferenceActive(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->_calibrationFailed.load(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), NO_ERROR);
  BOOST_CHECK_EQUAL(_stepperMotor->_motorControler->getActualPosition(), 0);
  while(!_stepperMotor->isSystemIdle()){}
  _stepperMotor->setTargetPositionInSteps(100);
  _stepperMotor->start();
  waitForMoveState();
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(_stepperMotor->_motorControler->getActualPosition(), 100);
}

void StepperMotorWithReferenceTest::testCalibrateError(){
  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotor->setEnabled(true);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  waitForCalibState();
  waitToSetPositiveTargetPos();
  _motorControlerDummy->moveTowardsTarget(1);
  waitForPositiveEndSwitchActive();
  waitToSetNegativeTargetPos();
  _motorControlerDummy->moveTowardsTarget(0.01);
  _motorControlerDummy->simulateBlockedMotor(true);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(_stepperMotor->_calibPositiveEndSwitchInSteps, 10000);
  BOOST_CHECK_EQUAL((std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotor->_stateMachine))->_moveInterrupted, true);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->_calibrationFailed.load(), true);
  _motorControlerDummy->simulateBlockedMotor(false);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), ACTION_ERROR);
  _motorControlerDummy->resetInternalStateToDefaults();
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  waitForCalibState();
  waitToSetPositiveTargetPos();
  _motorControlerDummy->moveTowardsTarget(0.01);
  _motorControlerDummy->simulateBlockedMotor(true);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL((std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotor->_stateMachine))->_moveInterrupted, true);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->_calibrationFailed.load(), true);
  _motorControlerDummy->simulateBlockedMotor(false);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), ACTION_ERROR);
}

void StepperMotorWithReferenceTest::testCalibrateStop(){
  _motorControlerDummy->resetInternalStateToDefaults();
  _motorControlerDummy->setPositiveEndSwitch(60000);
  _motorControlerDummy->setNegativeEndSwitch(-60000);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  waitForCalibState();
  waitToSetPositiveTargetPos();
  _stepperMotor->stop();
  waitForStop();
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL((std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotor->_stateMachine))->_moveInterrupted, false);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->_calibrationFailed.load(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), ACTION_ERROR);

  _motorControlerDummy->resetInternalStateToDefaults();
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  waitForCalibState();
  waitToSetPositiveTargetPos();
  _motorControlerDummy->moveTowardsTarget(1);
  waitToSetPositiveTargetPos(100000);
  _stepperMotor->stop();
  waitForStop();
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL((std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotor->_stateMachine))->_moveInterrupted, false);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->_calibrationFailed.load(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), ACTION_ERROR);
}

void StepperMotorWithReferenceTest::testTranslation(){
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor->_mutex.lock();
  _stepperMotor->_motorControler->setCalibrationTime(0);
  _stepperMotor->_mutex.unlock();
  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);
  _stepperMotor->_calibNegativeEndSwitchInSteps.exchange(-10000);
  _stepperMotor->_calibPositiveEndSwitchInSteps.exchange(10000);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(_stepperMotor->_motorControler->getCalibrationTime(), 0);

  BOOST_CHECK_NO_THROW(_stepperMotor->translateAxisInSteps(100));
  BOOST_CHECK_EQUAL(_stepperMotor->_maxPositionLimitInSteps, std::numeric_limits<int>::max());
  BOOST_CHECK_EQUAL(_stepperMotor->_minPositionLimitInSteps, std::numeric_limits<int>::min() + 100);
  BOOST_CHECK_EQUAL(_stepperMotor->_calibPositiveEndSwitchInSteps, 10000);
  BOOST_CHECK_EQUAL(_stepperMotor->_calibNegativeEndSwitchInSteps, -10000);

  _stepperMotor->_mutex.lock();
  _stepperMotor->_motorControler->setCalibrationTime(time(NULL));
  _stepperMotor->_calibrationFailed.exchange(false);
  _stepperMotor->_mutex.unlock();

  BOOST_CHECK_NO_THROW(_stepperMotor->translateAxisInSteps(100));
  BOOST_CHECK_EQUAL(_stepperMotor->_maxPositionLimitInSteps, std::numeric_limits<int>::max());
  BOOST_CHECK_EQUAL(_stepperMotor->_minPositionLimitInSteps, std::numeric_limits<int>::min() + 200);
  BOOST_CHECK_EQUAL(_stepperMotor->_calibPositiveEndSwitchInSteps, 10100);
  BOOST_CHECK_EQUAL(_stepperMotor->_calibNegativeEndSwitchInSteps, -9900);

  BOOST_CHECK_THROW(_stepperMotor->translateAxisInSteps(std::numeric_limits<int>::max()), MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->translateAxisInSteps(std::numeric_limits<int>::min()), MotorDriverException);
  _stepperMotor->_mutex.lock();
  _stepperMotor->_motorControler->setCalibrationTime(0);
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTest::testDetermineTolerance(){
  _motorControlerDummy->resetInternalStateToDefaults();
  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(_stepperMotor->_motorControler->getCalibrationTime(), 0);
  _stepperMotor->determineTolerance();
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(_stepperMotor->_toleranceCalculated.load(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->_toleranceCalcFailed.load(),  true);

  _stepperMotor->_mutex.lock();
  _stepperMotor->_motorControler->setCalibrationTime(time(NULL));
  _stepperMotor->_mutex.unlock();
  _stepperMotor->_calibrationFailed.exchange(false);
  _stepperMotor->_calibNegativeEndSwitchInSteps.exchange(-10000);
  _stepperMotor->_calibPositiveEndSwitchInSteps.exchange(10000);

  _stepperMotor->determineTolerance();
  //FIXME This test is the only place where member _index is used. It syncs the test to every step of the tolerance calculation.
  //      -> Modify test and remove _index. Based on implementation of the dummy, can this even fail?
  waitForDetTolerance();
  for (unsigned int i=0; i<10; i++){
    _motorControlerDummy->setPositiveEndSwitch(10000 + i);
    waitToSetPositiveTargetPos(_stepperMotor->_calibPositiveEndSwitchInSteps.load() - 1000);
    _motorControlerDummy->moveTowardsTarget(1);
    BOOST_CHECK_EQUAL(_stepperMotor->_motorControler->getActualPosition(), _stepperMotor->_calibPositiveEndSwitchInSteps.load() - 1000);
    BOOST_CHECK_EQUAL(_stepperMotor->isPositiveReferenceActive(), false);
    waitToSetPositiveTargetPos(_stepperMotor->_calibPositiveEndSwitchInSteps.load() + 1000);
    _motorControlerDummy->moveTowardsTarget(1);
    waitForPositiveEndSwitchActive();
    BOOST_CHECK(_stepperMotor->_motorControler->getActualPosition() == _motorControlerDummy->getPositiveEndSwitch());
    BOOST_CHECK(_stepperMotor->isPositiveReferenceActive() == true);
    while (_stepperMotor->_index != i+1 && i != 9){}
  }
  while (_stepperMotor->_index != 0){}
  for (unsigned int i=0; i<10; i++){
    _motorControlerDummy->setNegativeEndSwitch(-10000 - i*10);
    waitToSetNegativeTargetPos(_stepperMotor->_calibNegativeEndSwitchInSteps.load() + 1000);
    _motorControlerDummy->moveTowardsTarget(1);
    BOOST_CHECK(_stepperMotor->_motorControler->getActualPosition() == _stepperMotor->_calibNegativeEndSwitchInSteps + 1000);
    BOOST_CHECK(_stepperMotor->isNegativeReferenceActive() == false);
    waitToSetNegativeTargetPos(_stepperMotor->_calibNegativeEndSwitchInSteps.load() - 1000);
    _motorControlerDummy->moveTowardsTarget(1);
    waitForNegativeEndSwitchActive();
    BOOST_CHECK(_stepperMotor->_motorControler->getActualPosition() == _motorControlerDummy->getNEgativeEndSwitch());
    BOOST_CHECK(_stepperMotor->isNegativeReferenceActive() == true);
    while (_stepperMotor->_index != i+1 && i != 9){}
  }
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_CLOSE(_stepperMotor->getTolerancePositiveEndSwitch(), 3.02765, 0.001);
  BOOST_CHECK_CLOSE(_stepperMotor->getToleranceNegativeEndSwitch(), 30.2765, 0.001);
  _stepperMotor->setTargetPositionInSteps(0);
  _stepperMotor->start();
  waitToSetPositiveTargetPos(0);
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotor->_stateMachine)->_moveInterrupted, false);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), NO_ERROR);
  BOOST_CHECK_EQUAL(_stepperMotor->_toleranceCalculated, true);
  BOOST_CHECK_EQUAL(_stepperMotor->_toleranceCalcFailed, false);
}

void StepperMotorWithReferenceTest::testDetermineToleranceError(){
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotor->setEnabled(true);
  _stepperMotor->_mutex.lock();
  _stepperMotor->_motorControler->setCalibrationTime(15);
  _stepperMotor->_mutex.unlock();
  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);
  _stepperMotor->_calibNegativeEndSwitchInSteps.exchange(-10000);
  _stepperMotor->_calibPositiveEndSwitchInSteps.exchange(10000);
  while(!_stepperMotor->isSystemIdle()){}
  _stepperMotor->determineTolerance();
  waitForDetTolerance();
  waitToSetPositiveTargetPos(_stepperMotor->_calibPositiveEndSwitchInSteps.load() - 1000);
  _motorControlerDummy->moveTowardsTarget(0.1);
  _motorControlerDummy->simulateBlockedMotor(true);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), ACTION_ERROR);
  BOOST_CHECK_EQUAL(_stepperMotor->_toleranceCalculated.load(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->_toleranceCalcFailed.load(), true);
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor->_tolerancePositiveEndSwitch.exchange(100);
  _stepperMotor->_toleranceNegativeEndSwitch.exchange(100);
  _stepperMotor->_calibNegativeEndSwitchInSteps.exchange(0);
  _stepperMotor->_calibPositiveEndSwitchInSteps.exchange(20000);
  _stepperMotor->_toleranceCalculated.exchange(true);
  _stepperMotor->_toleranceCalcFailed.exchange(false);
  _stepperMotor->_mutex.lock();
  _stepperMotor->_motorControler->setCalibrationTime(time(NULL));
  _stepperMotor->_mutex.unlock();
  _stepperMotor->_calibrationFailed.exchange(false);
  _stepperMotor->setTargetPositionInSteps(10000);
  _stepperMotor->start();
  waitToSetPositiveTargetPos(10000);
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), CALIBRATION_LOST);
}

void StepperMotorWithReferenceTest::testDetermineToleranceStop(){
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotor->setEnabled(true);
  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);
  _stepperMotor->_calibNegativeEndSwitchInSteps = -10000;
  _stepperMotor->_calibPositiveEndSwitchInSteps =  10000;
  while(!_stepperMotor->isSystemIdle()){}
  _stepperMotor->_mutex.lock();
  _stepperMotor->_motorControler->setCalibrationTime(15);
  _stepperMotor->_mutex.unlock();
  _motorControlerDummy->simulateBlockedMotor(false);
  _stepperMotor->determineTolerance();
  waitToSetPositiveTargetPos(_stepperMotor->_calibPositiveEndSwitchInSteps - 1000);
  _stepperMotor->stop();
  waitForStop();
  _motorControlerDummy->moveTowardsTarget(1);
  waitToSetPositiveTargetPos(_stepperMotor->_calibPositiveEndSwitchInSteps + 1000);
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(_stepperMotor->_toleranceCalculated, false);
  BOOST_CHECK_EQUAL(_stepperMotor->_toleranceCalcFailed, true);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), ACTION_ERROR);
}

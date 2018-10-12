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
  private:
    boost::shared_ptr<ChimeraTK::StepperMotorWithReference> _stepperMotorWithReference;
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
				    _stepperMotorWithReference(),
				    _motorControlerDummy(){
  std::string deviceFileName(mtca4u::DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).deviceName);
  std::string mapFileName(mtca4u::DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).mapFileName);
  mtca4u::MotorDriverCardFactory::instance().setDummyMode();
  _motorControlerDummy = boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(deviceFileName,  moduleName, stepperMotorDeviceConfigFile)->getMotorControler(0));
}

void StepperMotorWithReferenceTest::waitForCalibState(){
  while (1){
    usleep(10);
    _stepperMotorWithReference->_mutex.lock();
    if ((_stepperMotorWithReference->_stateMachine->getCurrentState())->getName() != "calibrating"){
      _stepperMotorWithReference->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotorWithReference->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitForDetTolerance(){
  while (1){
    usleep(1000);
    _stepperMotorWithReference->_mutex.lock();
    if ((_stepperMotorWithReference->_stateMachine->getCurrentState())->getName() != "calculatingTolerance"){
      _stepperMotorWithReference->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotorWithReference->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitToSetPositiveTargetPos(int targetPos){
  while (1){
    usleep(1000);
    _stepperMotorWithReference->_mutex.lock();
    if (_stepperMotorWithReference->_motorControler->getTargetPosition() != targetPos){
      _stepperMotorWithReference->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotorWithReference->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitToSetNegativeTargetPos(int targetPos){
  while (1){
    usleep(1000);
    _stepperMotorWithReference->_mutex.lock();
    if (_stepperMotorWithReference->_motorControler->getTargetPosition() != targetPos){
      _stepperMotorWithReference->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotorWithReference->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitForMoveState(){
  while (1){
    usleep(1000);
    _stepperMotorWithReference->_mutex.lock();
    if (_stepperMotorWithReference->_stateMachine->getCurrentState()->getName() == "StepperMotorStateMachine"){
      if ((dynamic_cast<StepperMotorStateMachine*>(_stepperMotorWithReference->_stateMachine->getCurrentState()))->getCurrentState()->getName() == "movingState"){
	break;
      }
    }
    _stepperMotorWithReference->_mutex.unlock();
  }
  _stepperMotorWithReference->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitForPositiveEndSwitchActive(){
  while (1){
    usleep(1000);
    _stepperMotorWithReference->_mutex.lock();
    if (!_stepperMotorWithReference->_motorControler->getReferenceSwitchData().getPositiveSwitchActive()){
      _stepperMotorWithReference->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotorWithReference->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitForNegativeEndSwitchActive(){
  while (1){
    usleep(1000);
    _stepperMotorWithReference->_mutex.lock();
    if (!_stepperMotorWithReference->_motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
      _stepperMotorWithReference->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotorWithReference->_mutex.unlock();
}

void StepperMotorWithReferenceTest::waitForStop(){
  while (1){
    usleep(1000);
    _stepperMotorWithReference->_mutex.lock();
    if (std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotorWithReference->_stateMachine)->_stopAction != true){
      _stepperMotorWithReference->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotorWithReference->_mutex.unlock();
}

void StepperMotorWithReferenceTest::testCalibrate(){
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);

  _stepperMotorWithReference.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotorWithReference->setEnabled(true);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == false);

  BOOST_CHECK(_stepperMotorWithReference->isNegativeEndSwitchEnabled() == false);
  BOOST_CHECK(_stepperMotorWithReference->isPositiveEndSwitchEnabled() == false);
  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->calibrate());
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == false);
  BOOST_CHECK(_stepperMotorWithReference->_calibrationFailed == true);

  _motorControlerDummy->setNegativeReferenceSwitchEnabled(true);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _stepperMotorWithReference.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotorWithReference->setEnabled(true);
  BOOST_CHECK(_stepperMotorWithReference->isNegativeEndSwitchEnabled() == true);
  BOOST_CHECK(_stepperMotorWithReference->isPositiveEndSwitchEnabled() == false);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->calibrate());
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == false);
  BOOST_CHECK(_stepperMotorWithReference->_calibrationFailed == true);

  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
  _stepperMotorWithReference.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotorWithReference->setEnabled(true);
  BOOST_CHECK(_stepperMotorWithReference->isNegativeEndSwitchEnabled() == false);
  BOOST_CHECK(_stepperMotorWithReference->isPositiveEndSwitchEnabled() == true);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->calibrate());
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == false);
  BOOST_CHECK(_stepperMotorWithReference->_calibrationFailed == true);

  _motorControlerDummy->setNegativeReferenceSwitchEnabled(true);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
  _stepperMotorWithReference.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotorWithReference->setEnabled(true);
  BOOST_CHECK(_stepperMotorWithReference->isNegativeEndSwitchEnabled() == true);
  BOOST_CHECK(_stepperMotorWithReference->isPositiveEndSwitchEnabled() == true);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->calibrate());
  waitForCalibState();
  waitToSetPositiveTargetPos();
  _motorControlerDummy->moveTowardsTarget(1);
  waitForPositiveEndSwitchActive();
  waitToSetNegativeTargetPos();
  BOOST_CHECK(_stepperMotorWithReference->_calibPositiveEndSwitchInSteps == 10000);
  BOOST_CHECK(_stepperMotorWithReference->_motorControler->getActualPosition() == 10000);
  BOOST_CHECK(_stepperMotorWithReference->isPositiveReferenceActive() == true);
  BOOST_CHECK_THROW(_stepperMotorWithReference->setTargetPositionInSteps(0), ChimeraTK::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotorWithReference->moveRelativeInSteps(-100), ChimeraTK::MotorDriverException);
  BOOST_CHECK(_stepperMotorWithReference->_motorControler->getActualPosition() == 10000);
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->_calibPositiveEndSwitchInSteps == 20000);
  BOOST_CHECK(_stepperMotorWithReference->_calibNegativeEndSwitchInSteps == 0);
  BOOST_CHECK(_stepperMotorWithReference->isNegativeReferenceActive() == true);
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == true);
  BOOST_CHECK(_stepperMotorWithReference->_calibrationFailed == false);
  BOOST_CHECK(_stepperMotorWithReference->getError() == NO_ERROR);
  BOOST_CHECK(_stepperMotorWithReference->_motorControler->getActualPosition() == 0);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  _stepperMotorWithReference->setTargetPositionInSteps(100);
  _stepperMotorWithReference->start();
  waitForMoveState();
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->_motorControler->getActualPosition() == 100);
}

void StepperMotorWithReferenceTest::testCalibrateError(){
  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotorWithReference.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotorWithReference->setEnabled(true);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->calibrate());
  waitForCalibState();
  waitToSetPositiveTargetPos();
  _motorControlerDummy->moveTowardsTarget(1);
  waitForPositiveEndSwitchActive();
  waitToSetNegativeTargetPos();
  _motorControlerDummy->moveTowardsTarget(0.01);
  _motorControlerDummy->simulateBlockedMotor(true);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->_calibPositiveEndSwitchInSteps == 10000);
  BOOST_CHECK((std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotorWithReference->_stateMachine))->_moveInterrupted == true);
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == false);
  BOOST_CHECK(_stepperMotorWithReference->_calibrationFailed == true);
  _motorControlerDummy->simulateBlockedMotor(false);
  BOOST_CHECK(_stepperMotorWithReference->getError() == ACTION_ERROR);
  _motorControlerDummy->resetInternalStateToDefaults();
  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->calibrate());
  waitForCalibState();
  waitToSetPositiveTargetPos();
  _motorControlerDummy->moveTowardsTarget(0.01);
  _motorControlerDummy->simulateBlockedMotor(true);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK((std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotorWithReference->_stateMachine))->_moveInterrupted == true);
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == false);
  BOOST_CHECK(_stepperMotorWithReference->_calibrationFailed == true);
  _motorControlerDummy->simulateBlockedMotor(false);
  BOOST_CHECK(_stepperMotorWithReference->getError() == ACTION_ERROR);
}

void StepperMotorWithReferenceTest::testCalibrateStop(){
  _motorControlerDummy->resetInternalStateToDefaults();
  _motorControlerDummy->setPositiveEndSwitch(60000);
  _motorControlerDummy->setNegativeEndSwitch(-60000);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->calibrate());
  waitForCalibState();
  waitToSetPositiveTargetPos();
  _stepperMotorWithReference->stop();
  waitForStop();
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK((std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotorWithReference->_stateMachine))->_moveInterrupted == false);
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == false);
  BOOST_CHECK(_stepperMotorWithReference->_calibrationFailed == true);
  BOOST_CHECK(_stepperMotorWithReference->getError() == ACTION_ERROR);

  _motorControlerDummy->resetInternalStateToDefaults();
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->calibrate());
  waitForCalibState();
  waitToSetPositiveTargetPos();
  _motorControlerDummy->moveTowardsTarget(1);
  waitToSetPositiveTargetPos(100000);
  _stepperMotorWithReference->stop();
  waitForStop();
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK((std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotorWithReference->_stateMachine))->_moveInterrupted == false);
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == false);
  BOOST_CHECK(_stepperMotorWithReference->_calibrationFailed == true);
  BOOST_CHECK(_stepperMotorWithReference->getError() == ACTION_ERROR);
}

void StepperMotorWithReferenceTest::testTranslation(){
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotorWithReference->_mutex.lock();
  _stepperMotorWithReference->_motorControler->setCalibrationTime(0);
  _stepperMotorWithReference->_mutex.unlock();
  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);
  _stepperMotorWithReference->_calibNegativeEndSwitchInSteps = -10000;
  _stepperMotorWithReference->_calibPositiveEndSwitchInSteps =  10000;
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->_motorControler->getCalibrationTime() == 0);

  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->translateAxisInSteps(100));
  BOOST_CHECK(_stepperMotorWithReference->_maxPositionLimitInSteps == std::numeric_limits<int>::max());
  BOOST_CHECK(_stepperMotorWithReference->_minPositionLimitInSteps == std::numeric_limits<int>::min() + 100);
  BOOST_CHECK(_stepperMotorWithReference->_calibPositiveEndSwitchInSteps == 10000);
  BOOST_CHECK(_stepperMotorWithReference->_calibNegativeEndSwitchInSteps == -10000);

  _stepperMotorWithReference->_mutex.lock();
  _stepperMotorWithReference->_motorControler->setCalibrationTime(time(NULL));
  _stepperMotorWithReference->_calibrationFailed = false;
  _stepperMotorWithReference->_mutex.unlock();

  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->translateAxisInSteps(100));
  BOOST_CHECK(_stepperMotorWithReference->_maxPositionLimitInSteps == std::numeric_limits<int>::max());
  BOOST_CHECK(_stepperMotorWithReference->_minPositionLimitInSteps == std::numeric_limits<int>::min() + 200);
  BOOST_CHECK(_stepperMotorWithReference->_calibPositiveEndSwitchInSteps == 10100);
  BOOST_CHECK(_stepperMotorWithReference->_calibNegativeEndSwitchInSteps == -9900);

  BOOST_CHECK_THROW(_stepperMotorWithReference->translateAxisInSteps(std::numeric_limits<int>::max()), MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotorWithReference->translateAxisInSteps(std::numeric_limits<int>::min()), MotorDriverException);
  _stepperMotorWithReference->_mutex.lock();
  _stepperMotorWithReference->_motorControler->setCalibrationTime(0);
  _stepperMotorWithReference->_mutex.unlock();
}

void StepperMotorWithReferenceTest::testDetermineTolerance(){
  _motorControlerDummy->resetInternalStateToDefaults();
  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->_motorControler->getCalibrationTime() == 0);
  _stepperMotorWithReference->determineTolerance();
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->_toleranceCalculated == false);
  BOOST_CHECK(_stepperMotorWithReference->_toleranceCalcFailed == true);

  _stepperMotorWithReference->_mutex.lock();
  _stepperMotorWithReference->_motorControler->setCalibrationTime(time(NULL));
  _stepperMotorWithReference->_mutex.unlock();
  _stepperMotorWithReference->_calibrationFailed = false;
  _stepperMotorWithReference->_calibNegativeEndSwitchInSteps = -10000;
  _stepperMotorWithReference->_calibPositiveEndSwitchInSteps =  10000;

  _stepperMotorWithReference->determineTolerance();
  //FIXME This test is the only place where member _index is used. It syncs the test to every step of the tolerance calculation.
  //      -> Modify test and remove _index. Based on implementation of the dummy, can this even fail?
  waitForDetTolerance();
  for (unsigned int i=0; i<10; i++){
    _motorControlerDummy->setPositiveEndSwitch(10000 + i);
    waitToSetPositiveTargetPos(_stepperMotorWithReference->_calibPositiveEndSwitchInSteps - 1000);
    _motorControlerDummy->moveTowardsTarget(1);
    BOOST_CHECK(_stepperMotorWithReference->_motorControler->getActualPosition() == _stepperMotorWithReference->_calibPositiveEndSwitchInSteps - 1000);
    BOOST_CHECK(_stepperMotorWithReference->isPositiveReferenceActive() == false);
    waitToSetPositiveTargetPos(_stepperMotorWithReference->_calibPositiveEndSwitchInSteps + 1000);
    _motorControlerDummy->moveTowardsTarget(1);
    waitForPositiveEndSwitchActive();
    BOOST_CHECK(_stepperMotorWithReference->_motorControler->getActualPosition() == _motorControlerDummy->getPositiveEndSwitch());
    BOOST_CHECK(_stepperMotorWithReference->isPositiveReferenceActive() == true);
    while (_stepperMotorWithReference->_index != i+1 && i != 9){}
  }
  while (_stepperMotorWithReference->_index != 0){}
  for (unsigned int i=0; i<10; i++){
    _motorControlerDummy->setNegativeEndSwitch(-10000 - i*10);
    waitToSetNegativeTargetPos(_stepperMotorWithReference->_calibNegativeEndSwitchInSteps + 1000);
    _motorControlerDummy->moveTowardsTarget(1);
    BOOST_CHECK(_stepperMotorWithReference->_motorControler->getActualPosition() == _stepperMotorWithReference->_calibNegativeEndSwitchInSteps + 1000);
    BOOST_CHECK(_stepperMotorWithReference->isNegativeReferenceActive() == false);
    waitToSetNegativeTargetPos(_stepperMotorWithReference->_calibNegativeEndSwitchInSteps - 1000);
    _motorControlerDummy->moveTowardsTarget(1);
    waitForNegativeEndSwitchActive();
    BOOST_CHECK(_stepperMotorWithReference->_motorControler->getActualPosition() == _motorControlerDummy->getNEgativeEndSwitch());
    BOOST_CHECK(_stepperMotorWithReference->isNegativeReferenceActive() == true);
    while (_stepperMotorWithReference->_index != i+1 && i != 9){}
  }
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK_CLOSE(_stepperMotorWithReference->getTolerancePositiveEndSwitch(), 3.02765, 0.001);
  BOOST_CHECK_CLOSE(_stepperMotorWithReference->getToleranceNegativeEndSwitch(), 30.2765, 0.001);
  _stepperMotorWithReference->setTargetPositionInSteps(0);
  _stepperMotorWithReference->start();
  waitToSetPositiveTargetPos(0);
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotorWithReference->_stateMachine)->_moveInterrupted == false);
  BOOST_CHECK(_stepperMotorWithReference->getError() == NO_ERROR);
  BOOST_CHECK(_stepperMotorWithReference->_toleranceCalculated == true);
  BOOST_CHECK(_stepperMotorWithReference->_toleranceCalcFailed == false);
}

void StepperMotorWithReferenceTest::testDetermineToleranceError(){
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotorWithReference.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotorWithReference->setEnabled(true);
  _stepperMotorWithReference->_mutex.lock();
  _stepperMotorWithReference->_motorControler->setCalibrationTime(15);
  _stepperMotorWithReference->_mutex.unlock();
  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);
  _stepperMotorWithReference->_calibNegativeEndSwitchInSteps = -10000;
  _stepperMotorWithReference->_calibPositiveEndSwitchInSteps =  10000;
  while(!_stepperMotorWithReference->isSystemIdle()){}
  _stepperMotorWithReference->determineTolerance();
  waitForDetTolerance();
  waitToSetPositiveTargetPos(_stepperMotorWithReference->_calibPositiveEndSwitchInSteps - 1000);
  _motorControlerDummy->moveTowardsTarget(0.1);
  _motorControlerDummy->simulateBlockedMotor(true);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->getError() == ACTION_ERROR);
  BOOST_CHECK(_stepperMotorWithReference->_toleranceCalculated == false);
  BOOST_CHECK(_stepperMotorWithReference->_toleranceCalcFailed == true);
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotorWithReference->_tolerancePositiveEndSwitch = 100;
  _stepperMotorWithReference->_toleranceNegativeEndSwitch = 100;
  _stepperMotorWithReference->_calibNegativeEndSwitchInSteps = 0;
  _stepperMotorWithReference->_calibPositiveEndSwitchInSteps =  20000;
  _stepperMotorWithReference->_toleranceCalculated = true;
  _stepperMotorWithReference->_toleranceCalcFailed = false;
  _stepperMotorWithReference->_mutex.lock();
  _stepperMotorWithReference->_motorControler->setCalibrationTime(time(NULL));
  _stepperMotorWithReference->_mutex.unlock();
  _stepperMotorWithReference->_calibrationFailed = false;
  _stepperMotorWithReference->setTargetPositionInSteps(10000);
  _stepperMotorWithReference->start();
  waitToSetPositiveTargetPos(10000);
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->getError() == CALIBRATION_LOST);
}

void StepperMotorWithReferenceTest::testDetermineToleranceStop(){
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotorWithReference.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotorWithReference->setEnabled(true);
  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);
  _stepperMotorWithReference->_calibNegativeEndSwitchInSteps = -10000;
  _stepperMotorWithReference->_calibPositiveEndSwitchInSteps =  10000;
  while(!_stepperMotorWithReference->isSystemIdle()){}
  _stepperMotorWithReference->_mutex.lock();
  _stepperMotorWithReference->_motorControler->setCalibrationTime(15);
  _stepperMotorWithReference->_mutex.unlock();
  _motorControlerDummy->simulateBlockedMotor(false);
  _stepperMotorWithReference->determineTolerance();
  waitToSetPositiveTargetPos(_stepperMotorWithReference->_calibPositiveEndSwitchInSteps - 1000);
  _stepperMotorWithReference->stop();
  waitForStop();
  _motorControlerDummy->moveTowardsTarget(1);
  waitToSetPositiveTargetPos(_stepperMotorWithReference->_calibPositiveEndSwitchInSteps + 1000);
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->_toleranceCalculated == false);
  BOOST_CHECK(_stepperMotorWithReference->_toleranceCalcFailed == true);
  BOOST_CHECK(_stepperMotorWithReference->getError() == ACTION_ERROR);
}

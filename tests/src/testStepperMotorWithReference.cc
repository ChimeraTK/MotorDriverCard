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
#include <mtca4u/DMapFilesParser.h>
#include <mtca4u/Utilities.h>


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
    void testDetermineTolerance();
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
    add(BOOST_CLASS_TEST_CASE(&StepperMotorWithReferenceTest::testDetermineTolerance, myTest));
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
  while(_stepperMotorWithReference->_stateMachine->getCurrentState()->getName() != "calibrating"){}
  usleep(10000);
  _motorControlerDummy->moveTowardsTarget(1);
  usleep(10000);
  BOOST_CHECK(_stepperMotorWithReference->_calibPositiveEndSwitchInSteps == 10000);
  BOOST_CHECK(_stepperMotorWithReference->_motorControler->getActualPosition() == 10000);
  BOOST_CHECK_THROW(_stepperMotorWithReference->moveToPositionInSteps(0), ChimeraTK::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotorWithReference->moveRelativeInSteps(-100), ChimeraTK::MotorDriverException);
  BOOST_CHECK(_stepperMotorWithReference->_motorControler->getActualPosition() == 10000);
  usleep(10000);
  _motorControlerDummy->moveTowardsTarget(1);
  usleep(10000);
  BOOST_CHECK(_stepperMotorWithReference->_calibPositiveEndSwitchInSteps == 20000);
  BOOST_CHECK(_stepperMotorWithReference->_calibNegativeEndSwitchInSteps == 0);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == true);
  BOOST_CHECK(_stepperMotorWithReference->_calibrationFailed == false);
  BOOST_CHECK(_stepperMotorWithReference->_stateMachine->getCurrentState()->getName() == "StepperMotorStateMachine");

  BOOST_CHECK(_stepperMotorWithReference->getError() == NO_ERROR);
  BOOST_CHECK(_stepperMotorWithReference->_motorControler->getActualPosition() == 0);
  _stepperMotorWithReference->moveToPositionInSteps(100);
  usleep(10000);
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->_motorControler->getActualPosition() == 100);
}

void StepperMotorWithReferenceTest::testCalibrateError(){
  _motorControlerDummy->resetInternalStateToDefaults();
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->calibrate());
  usleep(10000);
  _motorControlerDummy->moveTowardsTarget(1);
  usleep(10000);
  _motorControlerDummy->moveTowardsTarget(0.01);
  _motorControlerDummy->simulateBlockedMotor(true);
  usleep(10000);
  BOOST_CHECK(_stepperMotorWithReference->_calibPositiveEndSwitchInSteps == 10000);
  BOOST_CHECK((std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotorWithReference->_stateMachine))->_moveInterrupted == true);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == false);
  BOOST_CHECK(_stepperMotorWithReference->_calibrationFailed == true);
  _motorControlerDummy->simulateBlockedMotor(false);
  BOOST_CHECK(_stepperMotorWithReference->getError() == ACTION_ERROR);
  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->calibrate());
  usleep(10000);
  _motorControlerDummy->moveTowardsTarget(0.01);
  usleep(10000);
  _motorControlerDummy->simulateBlockedMotor(true);
  BOOST_CHECK((std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotorWithReference->_stateMachine))->_moveInterrupted == true);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == false);
  BOOST_CHECK(_stepperMotorWithReference->_calibrationFailed == true);
  _motorControlerDummy->simulateBlockedMotor(false);
  BOOST_CHECK(_stepperMotorWithReference->getError() == ACTION_ERROR);
}

void StepperMotorWithReferenceTest::testCalibrateStop(){
  _motorControlerDummy->resetInternalStateToDefaults();
  MotorControlerDummy::_positiveEndSwitchPosition = 60000;
  MotorControlerDummy::_negativeEndSwitchPosition = -60000;
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->calibrate());
  usleep(10000);
  _stepperMotorWithReference->stop();
  usleep(10000);
  _motorControlerDummy->moveTowardsTarget(1);
  usleep(10000);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK((std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotorWithReference->_stateMachine))->_moveInterrupted == false);
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == false);
  BOOST_CHECK(_stepperMotorWithReference->_calibrationFailed == true);

  _motorControlerDummy->resetInternalStateToDefaults();
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotorWithReference->calibrate());
  usleep(10000);
  _motorControlerDummy->moveTowardsTarget(1);
  _motorControlerDummy->moveTowardsTarget(1);
  usleep(10000);
  _stepperMotorWithReference->stop();
  usleep(10000);
  _motorControlerDummy->moveTowardsTarget(1);
  usleep(10000);
  while(!_stepperMotorWithReference->isSystemIdle()){}
  BOOST_CHECK((std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotorWithReference->_stateMachine))->_moveInterrupted == false);
  BOOST_CHECK(_stepperMotorWithReference->isCalibrated() == false);
  BOOST_CHECK(_stepperMotorWithReference->_calibrationFailed == true);
}

void StepperMotorWithReferenceTest::testDetermineTolerance(){
  _motorControlerDummy->resetInternalStateToDefaults();
  //BOOST_CHECK(_stepperMotorWithReference->isSystemIdle() == true);

}
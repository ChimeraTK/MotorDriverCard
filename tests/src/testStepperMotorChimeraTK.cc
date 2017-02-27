/*
 * testStepperMotorChimeraTK.cc
 *
 *  Created on: Feb 24, 2017
 *      Author: vitimic
 */

#include <boost/test/included/unit_test.hpp>
#include <boost/shared_ptr.hpp>
using namespace boost::unit_test_framework;

#include "StepperMotor.h"
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
//static mtca4u::TMC429OutputWord readDFMCDummyMotor0VMaxRegister(boost::shared_ptr<mtca4u::DFMC_MD22Dummy>& dfmc_md22);
//static mtca4u::StallGuardControlData readDFMCDummyMotor0CurrentScale(boost::shared_ptr<mtca4u::DFMC_MD22Dummy>& dfmc_md2);

namespace ChimeraTK{

  class TestUnitConveter : public StepperMotorUnitsConverter {
  public:
    TestUnitConveter() {
    };

    virtual float stepsToUnits(int steps) {
      return float(steps/10.0);
    }
    virtual int unitsToSteps(float units) {
      return int(10.0*units);
    };
  };

  class StepperMotorChimeraTKTest{
  public:
    StepperMotorChimeraTKTest();
    void testSoftLimits();
    void testEnable();
    void testSetActualPosition();
    void testMove();
    void testMoveRelative();
    void testStop();
    void testEmergencyStop();
    void testConverter();
  private:

    boost::shared_ptr<ChimeraTK::StepperMotor> _stepperMotor;
    boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;
    boost::shared_ptr<TestUnitConveter> _testUnitConveter;
  };
}

using namespace ChimeraTK;

class StepperMotorChimeraTKTestSuite : public test_suite {
public:

  StepperMotorChimeraTKTestSuite() : test_suite("Stepper Motor ChimeraTK suite") {
    // create an instance of the test class
    boost::shared_ptr<StepperMotorChimeraTKTest> stepperMotorChimeraTK(new StepperMotorChimeraTKTest);
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testSoftLimits, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testEnable, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testSetActualPosition, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testMove, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testMoveRelative, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testStop, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testEmergencyStop, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testConverter, stepperMotorChimeraTK));
  }
};

test_suite*
init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
  framework::master_test_suite().p_name.value = "Stepper Motor ChimeraTK test suite";
  return new StepperMotorChimeraTKTestSuite;
}

StepperMotorChimeraTKTest::StepperMotorChimeraTKTest() :
		_stepperMotor(),
		_motorControlerDummy(),
		_testUnitConveter(){
  std::string deviceFileName(mtca4u::DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).deviceName);
  std::string mapFileName(mtca4u::DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).mapFileName);

  //_testUnitConveter.reset(new TestUnitConveter);

  mtca4u::MotorDriverCardFactory::instance().setDummyMode();
  _motorControlerDummy = boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(deviceFileName,  moduleName, stepperMotorDeviceConfigFile)->getMotorControler(0));

  _stepperMotor.reset(new ChimeraTK::StepperMotor(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));

  //!!!! CHANGE THIS FOR LINEAR STEPER MOTOR TESTS
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);
}

void StepperMotorChimeraTKTest::testSoftLimits(){
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getSoftwareLimitsEnabled() == false);
  BOOST_CHECK_THROW(_stepperMotor->setSoftwareLimitsInSteps(1000, -1000), mtca4u::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->setSoftwareLimitsInSteps(1000, 1000), mtca4u::MotorDriverException);
  BOOST_CHECK_NO_THROW(_stepperMotor->setSoftwareLimitsInSteps(-1000, 1000));
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() == 1000);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -1000);
}

void StepperMotorChimeraTKTest::testEnable(){
  BOOST_CHECK(_stepperMotor->getEnabled() == false);
  _stepperMotor->setEnabled(true);
  BOOST_CHECK(_stepperMotor->getEnabled() == true);
  _stepperMotor->setEnabled(false);
  BOOST_CHECK(_stepperMotor->getEnabled() == false);
  _stepperMotor->setEnabled(true);
}

void StepperMotorChimeraTKTest::testSetActualPosition(){
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
  BOOST_CHECK(_stepperMotor->isCalibrated() == false);
  BOOST_CHECK_THROW(_stepperMotor->moveToPositionInSteps(10), mtca4u::MotorDriverException);
  BOOST_CHECK_NO_THROW(_stepperMotor->setActualPositionInSteps(0));
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 0);
  BOOST_CHECK(_stepperMotor->isCalibrated() == true);
}

void StepperMotorChimeraTKTest::testMove(){
  _stepperMotor->setSoftwareLimitsEnabled(true);

  BOOST_CHECK(_stepperMotor->getSoftwareLimitsEnabled() == true);
  BOOST_CHECK_THROW(_stepperMotor->moveToPositionInSteps(100000), mtca4u::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->moveToPositionInSteps(-100000), mtca4u::MotorDriverException);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(10));
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  BOOST_CHECK_THROW(_stepperMotor->moveToPositionInSteps(20), mtca4u::MotorDriverException);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  BOOST_CHECK_THROW(_stepperMotor->moveRelativeInSteps(10), mtca4u::MotorDriverException);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 10);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(20));
  while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  BOOST_CHECK_THROW(_stepperMotor->moveToPositionInSteps(20), mtca4u::MotorDriverException);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  BOOST_CHECK_THROW(_stepperMotor->moveRelativeInSteps(10), mtca4u::MotorDriverException);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  _motorControlerDummy->moveTowardsTarget(0.5);
  _stepperMotor->setEnabled(false);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 15);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::ACTION_ERROR);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);

  _stepperMotor->setEnabled(true);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(30));
  while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 30);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
}

void StepperMotorChimeraTKTest::testMoveRelative(){
  BOOST_CHECK_NO_THROW(_stepperMotor->moveRelativeInSteps(5));
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 35);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
}

void StepperMotorChimeraTKTest::testStop(){
  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(50));
  while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  _motorControlerDummy->moveTowardsTarget(0.4);
  _stepperMotor->stop();
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 41);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
}

void StepperMotorChimeraTKTest::testEmergencyStop(){
  BOOST_CHECK(_stepperMotor->getEnabled() == true);
  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(56));
  while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  _motorControlerDummy->moveTowardsTarget(0.8);
  _stepperMotor->emergencyStop();
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 53);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
  BOOST_CHECK(_stepperMotor->getEnabled() == false);
  _stepperMotor->setEnabled(true);
}

void StepperMotorChimeraTKTest::testConverter(){
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 53);
  BOOST_CHECK(_stepperMotor->getMinPositionLimit() == -1000);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimit() ==  1000);
  BOOST_CHECK_THROW(_stepperMotor->setStepperMotorUnitsConverter(_testUnitConveter), mtca4u::MotorDriverException);
  _testUnitConveter.reset(new TestUnitConveter);
  BOOST_CHECK_NO_THROW(_stepperMotor->setStepperMotorUnitsConverter(_testUnitConveter));
  BOOST_CHECK_CLOSE(_stepperMotor->getCurrentPosition(), 5.3, 1e-4);
  BOOST_CHECK(_stepperMotor->getMinPositionLimit() == -100);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimit() ==  100);

  _stepperMotor->setActualPosition(10.3);
  BOOST_CHECK_CLOSE(_stepperMotor->getCurrentPosition(), 10.3, 1e-4);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 103);
  BOOST_CHECK_THROW(_stepperMotor->setSoftwareLimits(90, -90), mtca4u::MotorDriverException);
  BOOST_CHECK_NO_THROW(_stepperMotor->setSoftwareLimits(-90, 90));
  BOOST_CHECK(_stepperMotor->getMinPositionLimit() == -90);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimit() ==  90);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -900);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() ==  900);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPosition(10));
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 10);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 100);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveRelative(5));
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 15);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 150);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);

  _stepperMotor->setStepperMotorUnitsConverterToDefault();

  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 150);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 150);
  BOOST_CHECK(_stepperMotor->getMinPositionLimit() == -900);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimit() ==  900);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -900);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() ==  900);
}

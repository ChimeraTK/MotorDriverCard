/*
 * testStepperMotorChimeraTK.cc
 *
 *  Created on: Feb 24, 2017
 *      Author: vitimic
 */

#include <boost/test/included/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <utility>
using namespace boost::unit_test_framework;

#include "StepperMotor.h"
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
static mtca4u::TMC429OutputWord readDFMCDummyMotor0VMaxRegister(boost::shared_ptr<mtca4u::DFMC_MD22Dummy>& dfmc_md22);
static mtca4u::StallGuardControlData readDFMCDummyMotor0CurrentScale(boost::shared_ptr<mtca4u::DFMC_MD22Dummy>& dfmc_md2);

namespace ChimeraTK{

  class TestUnitConverter : public StepperMotorUnitsConverter {
  public:
    TestUnitConverter() {
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
    void testUnitsConverterInitialization();
    void testSoftLimits();
    void testEnable();
    void testSetActualPosition();
    void testTranslateAxis();
    void testMove();
    void testMoveRelative();
    void testTargetPositionAndStart();
    void testStop();
    void testEmergencyStop();
    void testFullStepping();
    void testDisable();
    void testConverter();
    void testLogSettings();
    void testGetSetSpeedLimit();
    void testSetGetCurrent();
    void waitForMoveState();
    void waitForDisable();
  private:

    boost::shared_ptr<ChimeraTK::StepperMotor> _stepperMotor;
    boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;
    std::shared_ptr<TestUnitConverter> _testUnitConverter;
  };
}

using namespace ChimeraTK;

class StepperMotorChimeraTKTestSuite : public test_suite {
public:

  StepperMotorChimeraTKTestSuite() : test_suite("Stepper Motor ChimeraTK suite") {
    // create an instance of the test class
    boost::shared_ptr<StepperMotorChimeraTKTest> stepperMotorChimeraTK(new StepperMotorChimeraTKTest);
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testUnitsConverterInitialization, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testSoftLimits, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testEnable, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testSetActualPosition, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testTranslateAxis, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testMove, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testMoveRelative, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testTargetPositionAndStart, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testStop, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testEmergencyStop, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testFullStepping, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testDisable, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testConverter, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testLogSettings, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testGetSetSpeedLimit, stepperMotorChimeraTK));
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testSetGetCurrent, stepperMotorChimeraTK));
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
        _testUnitConverter(){
  std::string deviceFileName(ChimeraTK::DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).deviceName);
  std::string mapFileName(ChimeraTK::DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).mapFileName);

  _testUnitConverter.reset(new TestUnitConverter);

  mtca4u::MotorDriverCardFactory::instance().setDummyMode();
  _motorControlerDummy = boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(deviceFileName,  moduleName, stepperMotorDeviceConfigFile)->getMotorControler(0));

  // Omit the optional 5th argument for the units converter here so we get the default 1:1 converter
  _stepperMotor.reset(new ChimeraTK::StepperMotor(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));

  //!!!! CHANGE THIS FOR LINEAR STEPER MOTOR TESTS
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);
}

void StepperMotorChimeraTKTest::waitForMoveState(){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){
      //std::cout << _stepperMotor->_stateMachine->getCurrentState()->getName() << std::endl;
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorChimeraTKTest::waitForDisable(){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "disable"){
      //std::cout << _stepperMotor->_stateMachine->getCurrentState()->getName() << std::endl;
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}


void StepperMotorChimeraTKTest::testUnitsConverterInitialization(){

  // _stepperMotor was constructed with default 1:1 converter
  int steps = 1000;
  BOOST_CHECK(_stepperMotor->recalculateStepsInUnits(steps) == steps);

 std::shared_ptr<ChimeraTK::StepperMotor>  motorWithCustomConverter
   = std::make_shared<ChimeraTK::StepperMotor>(stepperMotorDeviceName, moduleName, 1, stepperMotorDeviceConfigFile, _testUnitConverter);

  // Should convert according to the TestUnitConverter
  BOOST_CHECK(motorWithCustomConverter->recalculateStepsInUnits(steps) == 100.f);
}


void StepperMotorChimeraTKTest::testSoftLimits(){
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getSoftwareLimitsEnabled() == false);
  BOOST_CHECK_NO_THROW(_stepperMotor->setMaxPositionLimitInSteps(-1000));
  BOOST_CHECK_THROW(_stepperMotor->setMinPositionLimitInSteps(1000), mtca4u::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->setMinPositionLimitInSteps(-1000), mtca4u::MotorDriverException);
  BOOST_CHECK_NO_THROW(_stepperMotor->setMaxPositionLimitInSteps(1000));
  BOOST_CHECK_NO_THROW(_stepperMotor->setMinPositionLimitInSteps(-1000));
  BOOST_CHECK_THROW(_stepperMotor->setMaxPositionLimitInSteps(-1000), mtca4u::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->setMaxPositionLimitInSteps(-2000), mtca4u::MotorDriverException);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() == 1000);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -1000);
}

void StepperMotorChimeraTKTest::testEnable(){
  BOOST_CHECK(_stepperMotor->getEnabled() == false);
  _stepperMotor->setEnabled(true);
  BOOST_CHECK(_stepperMotor->getEnabled() == true);
  _stepperMotor->setEnabled(false);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getEnabled() == false);
  _stepperMotor->setEnabled(true);
}

void StepperMotorChimeraTKTest::testSetActualPosition(){
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
  BOOST_CHECK(_stepperMotor->isCalibrated() == false);
  BOOST_CHECK(_stepperMotor->getCalibrationTime() == 0);
  BOOST_CHECK_THROW(_stepperMotor->moveToPositionInSteps(10), mtca4u::MotorDriverException);
  BOOST_CHECK_NO_THROW(_stepperMotor->setActualPositionInSteps(0));
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 0);
  BOOST_CHECK(_stepperMotor->isCalibrated() == true);
  BOOST_CHECK(_stepperMotor->getCalibrationTime() != 0);
  uint32_t timeEG = static_cast<uint32_t>(time(NULL));
  _stepperMotor->_mutex.lock();
  _stepperMotor->_motorControler->setCalibrationTime(timeEG);
  _stepperMotor->_mutex.unlock();
  BOOST_CHECK(_stepperMotor->getCalibrationTime() == timeEG);
}

void StepperMotorChimeraTKTest::testTranslateAxis(){
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() == 1000);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -1000);
  BOOST_CHECK_NO_THROW(_stepperMotor->translateAxisInSteps(100));
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() == 1100);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -900);
  BOOST_CHECK_NO_THROW(_stepperMotor->translateAxisInSteps(-300));
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() == 800);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -1200);
  BOOST_CHECK_NO_THROW(_stepperMotor->translateAxisInSteps(std::numeric_limits<int>::max()));
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() == std::numeric_limits<int>::max());
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -1200 + std::numeric_limits<int>::max());
  BOOST_CHECK_NO_THROW(_stepperMotor->setMinPositionLimitInSteps(-1000));
  BOOST_CHECK_NO_THROW(_stepperMotor->setMaxPositionLimitInSteps(1000));
  BOOST_CHECK_NO_THROW(_stepperMotor->translateAxisInSteps(std::numeric_limits<int>::min()));
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() == std::numeric_limits<int>::min() + 1000);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == std::numeric_limits<int>::min());
  BOOST_CHECK_NO_THROW(_stepperMotor->setMaxPositionLimitInSteps(1000));
  BOOST_CHECK_NO_THROW(_stepperMotor->setMinPositionLimitInSteps(-1000));
  BOOST_CHECK_NO_THROW(_stepperMotor->setActualPositionInSteps(0));
}

void StepperMotorChimeraTKTest::testMove(){
  _stepperMotor->setSoftwareLimitsEnabled(true);

  BOOST_CHECK(_stepperMotor->getSoftwareLimitsEnabled() == true);
  BOOST_CHECK_THROW(_stepperMotor->moveToPositionInSteps(100000), mtca4u::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->moveToPositionInSteps(-100000), mtca4u::MotorDriverException);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(10));
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  //usleep(10000);
  waitForMoveState();
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
  //while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  //usleep(10000);
  waitForMoveState();
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  BOOST_CHECK_THROW(_stepperMotor->moveToPositionInSteps(20), mtca4u::MotorDriverException);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  BOOST_CHECK_THROW(_stepperMotor->moveRelativeInSteps(10), mtca4u::MotorDriverException);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  BOOST_CHECK_THROW(_stepperMotor->setSoftwareLimitsEnabled(false), mtca4u::MotorDriverException);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  BOOST_CHECK(_stepperMotor->getSoftwareLimitsEnabled() == true);
  BOOST_CHECK_THROW(_stepperMotor->setMinPositionLimitInSteps(20000), mtca4u::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->setMaxPositionLimitInSteps(40000), mtca4u::MotorDriverException);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() == 1000);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -1000);
  BOOST_CHECK_THROW(_stepperMotor->setActualPositionInSteps(10000), mtca4u::MotorDriverException);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 10);
  BOOST_CHECK_THROW(_stepperMotor->setUserCurrentLimit(100), mtca4u::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->setUserSpeedLimit(100), mtca4u::MotorDriverException);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 20);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(30));
  //while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  //usleep(10000);
  waitForMoveState();
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 30);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
}

void StepperMotorChimeraTKTest::testMoveRelative(){
  BOOST_CHECK_NO_THROW(_stepperMotor->moveRelativeInSteps(5));
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  //while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  //usleep(10000);
  waitForMoveState();
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 35);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
}

void StepperMotorChimeraTKTest::testTargetPositionAndStart(){
  float targetPos = 432.f;
  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPosition(targetPos));
  BOOST_CHECK(_stepperMotor->getAutostart() == false); /* should be false from member init*/
  BOOST_CHECK_NO_THROW(_stepperMotor->start());

  waitForMoveState();
  _motorControlerDummy->moveTowardsTarget(1.f);
  _stepperMotor->waitForIdle();

  BOOST_CHECK(_stepperMotor->getTargetPosition() == targetPos);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == _stepperMotor->getTargetPosition());

  _stepperMotor->setAutostart(true);
  int targetPosSteps = 35;
  _stepperMotor->setTargetPositionInSteps(targetPosSteps);
  waitForMoveState();
  _motorControlerDummy->moveTowardsTarget(1.f);
  BOOST_CHECK(_stepperMotor->getTargetPositionInSteps() == targetPosSteps);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == targetPosSteps);
  _stepperMotor->waitForIdle();

  _stepperMotor->setAutostart(false);
  BOOST_CHECK(_stepperMotor->getAutostart() == false);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
}

void StepperMotorChimeraTKTest::testStop(){
  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(50));

  waitForMoveState();
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
  //while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  //usleep(10000);
  waitForMoveState();
  _motorControlerDummy->moveTowardsTarget(0.8);
  _stepperMotor->emergencyStop();
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
  BOOST_CHECK(_stepperMotor->getEnabled() == false);
  BOOST_CHECK(_stepperMotor->isCalibrated() == false);
  BOOST_CHECK_NO_THROW(_stepperMotor->setActualPositionInSteps(53));
  BOOST_CHECK(_stepperMotor->getEnabled() == false);
  _stepperMotor->setEnabled(true);
  BOOST_CHECK(_stepperMotor->getEnabled() == true);
  BOOST_CHECK(_stepperMotor->isCalibrated() == true);
}

void StepperMotorChimeraTKTest::testFullStepping(){
  _motorControlerDummy->resetInternalStateToDefaults();
  _motorControlerDummy->setCalibrationTime(32);
  _stepperMotor->setEnabled(true);
  BOOST_CHECK(_stepperMotor->getEnabled() == true);
  BOOST_CHECK(_stepperMotor->isCalibrated() == true);
  BOOST_CHECK(_stepperMotor->isFullStepping() == false);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 0);
  BOOST_CHECK_NO_THROW(_stepperMotor->enableFullStepping());
  BOOST_CHECK(_stepperMotor->isFullStepping() == true);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(23));
  //usleep(10000);
  _stepperMotor->waitForIdle();
  //BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  //waitForMoveState();
  //usleep(1000);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 0);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 1023);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(-12));
  _stepperMotor->waitForIdle();
  //waitForMoveState();
  //usleep(1000);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 0);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 1023);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(-32));
  _stepperMotor->waitForIdle();
  //waitForMoveState();
  //usleep(1000);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 0);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 1023);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(-33));
  waitForMoveState();
  //usleep(1000);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == -64);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 767);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(-48));
  _stepperMotor->waitForIdle();
  //waitForMoveState();
  //usleep(1000);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == -64);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 767);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(48));
  waitForMoveState();
  //usleep(1000);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 64);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 255);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(67));
  _stepperMotor->waitForIdle();
  //waitForMoveState();
  //usleep(1000);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 64);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 255);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(95));
  _stepperMotor->waitForIdle();
  //waitForMoveState();
  //usleep(1000);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 64);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 255);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(96));
  waitForMoveState();
  //usleep(1000);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 128);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 511);

  BOOST_CHECK_NO_THROW(_stepperMotor->enableFullStepping(false));
}

void StepperMotorChimeraTKTest::testDisable(){
  _motorControlerDummy->resetInternalStateToDefaults();
  _motorControlerDummy->setCalibrationTime(32);
  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPositionInSteps(96));
  //while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  waitForMoveState();
  //usleep(10000);
  _motorControlerDummy->moveTowardsTarget(0.5);
  _stepperMotor->setEnabled(false);
  waitForDisable();
  //usleep(1000);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 64);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 255);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
  BOOST_CHECK(_stepperMotor->getEnabled() == false);
  BOOST_CHECK(_stepperMotor->isCalibrated() == true);
  BOOST_CHECK_NO_THROW(_stepperMotor->setActualPositionInSteps(68));
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  _stepperMotor->setEnabled(true);
  BOOST_CHECK(_stepperMotor->getEnabled() == true);
}

void StepperMotorChimeraTKTest::testConverter(){
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 68);
  BOOST_CHECK(_stepperMotor->getMinPositionLimit() == -1000);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimit() ==  1000);
  _testUnitConverter.reset();
  BOOST_CHECK_THROW(_stepperMotor->setStepperMotorUnitsConverter(_testUnitConverter), mtca4u::MotorDriverException);
  _testUnitConverter.reset(new TestUnitConverter);
  BOOST_CHECK_NO_THROW(_stepperMotor->setStepperMotorUnitsConverter(_testUnitConverter));
  BOOST_CHECK_CLOSE(_stepperMotor->getCurrentPosition(), 6.8, 1e-4);
  BOOST_CHECK(_stepperMotor->getMinPositionLimit() == -100);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimit() ==  100);

  _stepperMotor->setActualPosition(10.3);
  BOOST_CHECK_CLOSE(_stepperMotor->getCurrentPosition(), 10.3, 1e-4);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 103);
  BOOST_CHECK_THROW(_stepperMotor->setMaxPositionLimit(-900), mtca4u::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->setMinPositionLimit(900), mtca4u::MotorDriverException);
  BOOST_CHECK_NO_THROW(_stepperMotor->setMaxPositionLimit(90));
  BOOST_CHECK_NO_THROW(_stepperMotor->setMinPositionLimit(-90));
  BOOST_CHECK(_stepperMotor->getMinPositionLimit() == -90);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimit() ==  90);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -900);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() ==  900);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPosition(10));
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  //while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  //usleep(10000);
  waitForMoveState();
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 10);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 100);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveRelative(5));
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  //while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  //usleep(10000);
  waitForMoveState();
  BOOST_CHECK_THROW(_stepperMotor->setStepperMotorUnitsConverterToDefault(), mtca4u::MotorDriverException);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 15);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 150);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);

  BOOST_CHECK_NO_THROW(_stepperMotor->setStepperMotorUnitsConverter(_testUnitConverter));
  _stepperMotor->waitForIdle();
  _stepperMotor->moveToPositionInSteps(100);
  //usleep(1000);
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  //BOOST_CHECK(_stepperMotor->_stateMachine->getCurrentState()->getName() == "movingState");
  //usleep(10000);
  waitForMoveState();
  _motorControlerDummy->moveTowardsTarget(0.5);
  _stepperMotor->stop();
  //usleep(1000);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->isSystemIdle() == true);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 12.5);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 125);
  _stepperMotor->setStepperMotorUnitsConverterToDefault();

  BOOST_CHECK_NO_THROW(_stepperMotor->moveToPosition(100));
  BOOST_CHECK(_stepperMotor->isSystemIdle() == false);
  //while ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "movingState"){}
  //usleep(10000);
  waitForMoveState();
  BOOST_CHECK_THROW(_stepperMotor->setStepperMotorUnitsConverter(_testUnitConverter), mtca4u::MotorDriverException);
  _motorControlerDummy->moveTowardsTarget(1);
   _stepperMotor->waitForIdle();

  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 100);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 100);
  BOOST_CHECK(_stepperMotor->getMinPositionLimit() == -900);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimit() ==  900);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -900);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() ==  900);
}

void StepperMotorChimeraTKTest::testLogSettings(){
  _stepperMotor->setLogLevel(Logger::NO_LOGGING);
  BOOST_CHECK( _stepperMotor->getLogLevel() == Logger::NO_LOGGING );

  _stepperMotor->setLogLevel(Logger::ERROR);
  BOOST_CHECK(_stepperMotor->getLogLevel() == Logger::ERROR);

  _stepperMotor->setLogLevel(Logger::WARNING);
  BOOST_CHECK(_stepperMotor->getLogLevel() == Logger::WARNING);

  _stepperMotor->setLogLevel(Logger::INFO);
  BOOST_CHECK(_stepperMotor->getLogLevel() == Logger::INFO);

  _stepperMotor->setLogLevel(Logger::DETAIL);
  BOOST_CHECK(_stepperMotor->getLogLevel() == Logger::DETAIL);

  _stepperMotor->setLogLevel(Logger::FULL_DETAIL);
  BOOST_CHECK(_stepperMotor->getLogLevel() == Logger::FULL_DETAIL);

  _stepperMotor->setLogLevel(Logger::NO_LOGGING);
}

void StepperMotorChimeraTKTest::testGetSetSpeedLimit() {
  // setup
  auto dmapFile = ChimeraTK::getDMapFilePath();
  ChimeraTK::setDMapFilePath("./dummies.dmap");
  auto dummyModeStatus = MotorDriverCardFactory::instance().getDummyMode();
  MotorDriverCardFactory::instance().setDummyMode(false);
  mtca4u::StepperMotor motor("DFMC_MD22_PERSISTENT_BACKEND", "MD22_0", 0, "custom_speed_and_curruent_limits.xml");


  // Expected max speed  capability in this case:[from custom_speed_and_curruent_limits.xml]
  //   Vmax = 1398 (0x576)
  //   fclk = 32000000
  //   pulse_div = 6
  // expectedMaxSpeedLimitInUstepsPerSecond = Vmax * fclk/((2^pulse_div) * 2048 * 32)
  //
  auto expectedMaxSpeedLimit = (1398.0 * 32000000.0) / (exp2(6) * 2048 * 32);

  // expectedMinimumSpeedLimitInUstepsPerSec = Vmin * fclk/((2^pulse_div) * 2048 * 32)
  auto expectedMinimumSpeed = (1.0 * 32000000.0) / // VMin = 1 (1.0)
                              (exp2(6) * 2048 * 32);

  //verify getMaxSpeedCapability
  BOOST_CHECK(motor.getMaxSpeedCapability() == expectedMaxSpeedLimit);

  // set speed > MaxAllowedSpeed
  BOOST_CHECK(motor.setUserSpeedLimit(expectedMaxSpeedLimit) == expectedMaxSpeedLimit);
  BOOST_CHECK(motor.getUserSpeedLimit() == expectedMaxSpeedLimit);
  BOOST_CHECK(motor.setUserSpeedLimit(expectedMaxSpeedLimit + 10) == expectedMaxSpeedLimit);
  BOOST_CHECK(motor.getUserSpeedLimit() == expectedMaxSpeedLimit);

  // set speed < MinAllowedSpeed
  BOOST_CHECK(motor.setUserSpeedLimit(expectedMinimumSpeed) == expectedMinimumSpeed);
  BOOST_CHECK(motor.getUserSpeedLimit() == expectedMinimumSpeed);
  BOOST_CHECK(motor.setUserSpeedLimit(expectedMinimumSpeed - 10) == expectedMinimumSpeed);
  BOOST_CHECK(motor.getUserSpeedLimit() == expectedMinimumSpeed);
  BOOST_CHECK(motor.setUserSpeedLimit(-100) == expectedMinimumSpeed);
  BOOST_CHECK(motor.getUserSpeedLimit() == expectedMinimumSpeed);

  // set speed in valid range [Vmin, Vmax]
  // speed = 8000 microsteps per second -> results in Vmax = 1048.576
  // Vmax 1048.576  floors to 1048 => which gives returned/actual set speed as
  // 7995.6054... microsteps per second
  auto expectedSetSpeed = (1048.0 * 32000000.0) / (exp2(6) * 2048 * 32);
  BOOST_CHECK(motor.setUserSpeedLimit(8000) == expectedSetSpeed);
  BOOST_CHECK(motor.getUserSpeedLimit() == expectedSetSpeed);

  // Verify that Vmax is 1048 in the internal register (which is the
  // VMax corresponding to expectedSetSpeed)
  auto md22_instance = boost::dynamic_pointer_cast<DFMC_MD22Dummy>(
      BackendFactory::getInstance().createBackend(
          "DFMC_MD22_PERSISTENT_BACKEND"));
  BOOST_ASSERT(md22_instance != NULL);
  TMC429OutputWord vMaxRegister = readDFMCDummyMotor0VMaxRegister(md22_instance);
 BOOST_CHECK(vMaxRegister.getDATA() == 1048);

  //teardown
    MotorDriverCardFactory::instance().setDummyMode(dummyModeStatus);
    ChimeraTK::setDMapFilePath(dmapFile);
}

void StepperMotorChimeraTKTest::testSetGetCurrent() {
  auto dmapFile = ChimeraTK::getDMapFilePath();
  ChimeraTK::setDMapFilePath("./dummies.dmap");
  auto dummyModeStatus = MotorDriverCardFactory::instance().getDummyMode();
  MotorDriverCardFactory::instance().setDummyMode(false);
  mtca4u::StepperMotor motor("DFMC_MD22_PERSISTENT_BACKEND", "MD22_0", 0, "custom_speed_and_curruent_limits.xml");

  // The motor has been configured for a maximum current of .24 Amps, with 1.8A
  // being the maximum the driver ic can provide. This translates to a current
  // scale according to the formula:
  // (.24 * 32)/1.8 - 1 == 3.266
  // the 32 in the above formula comes from the 32 current scale levels for the
  // tmc260.
  // The tmc260 current scale register takes a whole number. So floor(3.266)
  // goes into the current scale register. Because of this we get an effective
  // current limit of:
  // 1.8 * (3 + 1)/32 == .225 A

  auto expectedSafeCurrentLimitValue = 1.8 * (4.0)/32.0;
  auto safeCurrentLimit = motor.getSafeCurrentLimit();
  BOOST_CHECK(safeCurrentLimit == expectedSafeCurrentLimitValue);

  // set Current to Beyond the safe limit
  BOOST_CHECK(motor.setUserCurrentLimit(safeCurrentLimit + 10) == safeCurrentLimit);
  BOOST_CHECK(motor.getUserCurrentLimit() == safeCurrentLimit);
  BOOST_CHECK(motor.setUserCurrentLimit(safeCurrentLimit) == safeCurrentLimit);
  BOOST_CHECK(motor.getUserCurrentLimit() == safeCurrentLimit);

  // set a current below minimum value.
  // Minimum value of current (when current scale is 0):
  // 1.8 * (0 + 1)/32 == .05625A
  auto minimumCurrent = 1.8 * 1.0/32.0;
  BOOST_CHECK(motor.setUserCurrentLimit(0) == minimumCurrent);
  BOOST_CHECK(motor.getUserCurrentLimit() == minimumCurrent);
  BOOST_CHECK(motor.setUserCurrentLimit(-10) == minimumCurrent);
  BOOST_CHECK(motor.getUserCurrentLimit() == minimumCurrent);

  // Current in a valid range: 0 - .24A
  // I = .2A
  // CS = (.20 * 32)/1.8 - 1 = floor(2.556) = 2
  // For CS 2: Expected current = (1.8 * (2 + 1))/32 = .16875A
  auto expectedCurrent = (1.8 * (2 + 1))/32.0;
  BOOST_CHECK(motor.setUserCurrentLimit(.2) == expectedCurrent);
  BOOST_CHECK(motor.getUserCurrentLimit() == expectedCurrent);
  // Verify that CS is 2 in the internal register ()
  auto md22_instance = boost::dynamic_pointer_cast<DFMC_MD22Dummy>(
      BackendFactory::getInstance().createBackend("DFMC_MD22_PERSISTENT_BACKEND"));
  BOOST_ASSERT(md22_instance != NULL);
  StallGuardControlData stallGuard = readDFMCDummyMotor0CurrentScale(md22_instance);
  BOOST_CHECK(stallGuard.getCurrentScale() == 2);

  //teardown
    MotorDriverCardFactory::instance().setDummyMode(dummyModeStatus);
    ChimeraTK::setDMapFilePath(dmapFile);
}

mtca4u::TMC429OutputWord readDFMCDummyMotor0VMaxRegister(boost::shared_ptr<mtca4u::DFMC_MD22Dummy>& dfmc_md22){
  mtca4u::TMC429InputWord readVmaxRegister;
  readVmaxRegister.setSMDA(0);
  readVmaxRegister.setIDX_JDX(tmc429::IDX_MAXIMUM_VELOCITY);
  readVmaxRegister.setRW(tmc429::RW_READ);
  return mtca4u::TMC429OutputWord(
      dfmc_md22->readTMC429Register(readVmaxRegister.getDataWord()));
}


mtca4u::StallGuardControlData readDFMCDummyMotor0CurrentScale(boost::shared_ptr<mtca4u::DFMC_MD22Dummy>& dfmc_md2){
  auto motorId = 0;
  return StallGuardControlData(dfmc_md2->readTMC260Register(
      motorId, mtca4u::DFMC_MD22Dummy::TMC260Register::STALLGUARD2_CONTROL_REGISTER));
}

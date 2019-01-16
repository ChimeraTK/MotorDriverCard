#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE StepperMotorChimeraTKTest

#include <boost/test/included/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
using namespace boost::unit_test_framework;

#include "StepperMotor.h"
#include "BasicStepperMotor.h"
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



class TestUnitConverter : public ChimeraTK::StepperMotorUnitsConverter {
public:
  TestUnitConverter() {
  }

  virtual float stepsToUnits(int steps) {
    return float(steps/10.0f);
  }
  virtual int unitsToSteps(float units) {
    return int(10.0f*units);
  }
};


static mtca4u::TMC429OutputWord readDFMCDummyMotor0VMaxRegister(boost::shared_ptr<mtca4u::DFMC_MD22Dummy>& dfmc_md22){
  mtca4u::TMC429InputWord readVmaxRegister;
  readVmaxRegister.setSMDA(0);
  readVmaxRegister.setIDX_JDX(mtca4u::tmc429::IDX_MAXIMUM_VELOCITY);
  readVmaxRegister.setRW(mtca4u::tmc429::RW_READ);
  return mtca4u::TMC429OutputWord(
      dfmc_md22->readTMC429Register(readVmaxRegister.getDataWord()));
}


static mtca4u::StallGuardControlData readDFMCDummyMotor0CurrentScale(boost::shared_ptr<mtca4u::DFMC_MD22Dummy>& dfmc_md2){
  auto motorId = 0U;
  return mtca4u::StallGuardControlData(dfmc_md2->readTMC260Register(
      motorId, mtca4u::DFMC_MD22Dummy::TMC260Register::STALLGUARD2_CONTROL_REGISTER));
}


class StepperMotorChimeraTKFixture{
public:
  StepperMotorChimeraTKFixture();
  // Helper functions
  bool waitForState(std::string stateName, unsigned timeoutInSeconds = 10);
  void moveThreadFcn();
  void configThreadFcn();

protected:
  std::unique_ptr<ChimeraTK::StepperMotor> _stepperMotor;
  boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;
  std::shared_ptr<TestUnitConverter> _testUnitConverter;
};


using namespace ChimeraTK;


// Fixture for common setup
StepperMotorChimeraTKFixture::StepperMotorChimeraTKFixture() :
        _stepperMotor(),
        _motorControlerDummy(),
        _testUnitConverter(){
  std::string deviceFileName(ChimeraTK::DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).deviceName);
  std::string mapFileName(ChimeraTK::DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).mapFileName);

  _testUnitConverter = std::make_unique<TestUnitConverter>();

  mtca4u::MotorDriverCardFactory::instance().setDummyMode();
  _motorControlerDummy = boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(deviceFileName,  moduleName, stepperMotorDeviceConfigFile)->getMotorControler(0));

  // Omit the optional 5th argument for the units converter here so we get the default 1:1 converter for the encoder readout
  ChimeraTK::StepperMotorParameters parameters;
  parameters.deviceName = stepperMotorDeviceName;
  parameters.moduleName = moduleName;
  parameters.configFileName = stepperMotorDeviceConfigFile;
  _stepperMotor = std::make_unique<BasicStepperMotor>(parameters);

  // Allow autostart to save some lines in the tests
  _stepperMotor->setAutostart(true);

  // We test a motor /wo end switches
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);

  //Set common settings for all tests
  _stepperMotor->setMaxPositionLimitInSteps(1000);
  _stepperMotor->setMinPositionLimitInSteps(-1000);
}


void StepperMotorChimeraTKFixture::moveThreadFcn(){

  for(int i = 0; i<1000; i++){

    _stepperMotor->moveRelativeInSteps((i+1)*2);

    waitForState("moving");
    //std::cout << "    **** State: " << _stepperMotor->getState() << std::endl;
    _motorControlerDummy->moveTowardsTarget(1.0f);
    _stepperMotor->waitForIdle();
  }

}

void StepperMotorChimeraTKFixture::configThreadFcn(){
  for(int i = 0; i<1000; i++){
    if(_stepperMotor->isSystemIdle()){
       std::cout << "    **** State: " << _stepperMotor->getState() << std::endl;
      _stepperMotor->setUserSpeedLimit(10000);
    }
  }
}

bool StepperMotorChimeraTKFixture::waitForState(std::string stateName, unsigned timeoutInSeconds){

  unsigned cnt = 0;
  bool isCorrectState = false;
  do{
    usleep(1000);

    if(_stepperMotor->getState() == stateName){
      isCorrectState = true;
      break;
    }
    else{
      cnt++;
    }
  }while(cnt <= timeoutInSeconds);

  return isCorrectState;
}



BOOST_FIXTURE_TEST_SUITE(StepperMotorChimeraTKTest, StepperMotorChimeraTKFixture)

BOOST_AUTO_TEST_CASE(  testStepperMotorFactory ){

  StepperMotorFactory& inst = StepperMotorFactory::instance();

  StepperMotorParameters parametersBasicMotor;
  parametersBasicMotor.deviceName = stepperMotorDeviceName;
  parametersBasicMotor.moduleName = moduleName;
  parametersBasicMotor.configFileName = stepperMotorDeviceConfigFile;
  parametersBasicMotor.motorUnitsConverter = std::move(_testUnitConverter);

  auto _motor = inst.create(parametersBasicMotor);

  BOOST_CHECK_EQUAL(_motor->hasHWReferenceSwitches(), false);
  BOOST_CHECK_THROW(_motor->isNegativeEndSwitchEnabled(), mtca4u::MotorDriverException);

  StepperMotorParameters parametersLinearMotor;
  parametersLinearMotor.motorType  = StepperMotorType::LINEAR;
  parametersLinearMotor.deviceName = stepperMotorDeviceName;
  parametersLinearMotor.moduleName = moduleName;
  parametersLinearMotor.driverId = 1U; /* Motor with ID 0 already exists */
  parametersLinearMotor.configFileName = stepperMotorDeviceConfigFile;
  parametersLinearMotor.motorUnitsConverter = std::move(_testUnitConverter);

 _motor = inst.create(parametersLinearMotor);

  BOOST_CHECK_EQUAL(_motor->hasHWReferenceSwitches(), true);
  BOOST_CHECK_NO_THROW(_motor->isNegativeEndSwitchEnabled());
}


BOOST_AUTO_TEST_CASE( testUnitsConverterInitialization ){

  // _stepperMotor was constructed with default 1:1 converter
  int steps = 1000;
  BOOST_CHECK_EQUAL(_stepperMotor->recalculateStepsInUnits(steps), steps);

 std::unique_ptr<StepperMotorUtility::EncoderUnitsConverter> encoderUnitsConverter
     = std::make_unique<StepperMotorUtility::EncoderUnitsScalingConverter>(10.);

 StepperMotorParameters parameters;
 parameters.deviceName = stepperMotorDeviceName;
 parameters.moduleName = moduleName;
 parameters.driverId = 1U;
 parameters.configFileName = stepperMotorDeviceConfigFile;
 parameters.motorUnitsConverter = std::move(_testUnitConverter);
 parameters.encoderUnitsConverter = std::move(encoderUnitsConverter);

 std::unique_ptr<ChimeraTK::StepperMotor>  motorWithCustomConverter
   = std::make_unique<ChimeraTK::BasicStepperMotor>(parameters);

  // Should convert according to the TestUnitConverter
  BOOST_CHECK_EQUAL(motorWithCustomConverter->recalculateStepsInUnits(steps), 100.f);

  // The raw encoder position is 10000 from the implementation of
  // the underlying MotorControlerDummy, so we should end up at 100000
  BOOST_CHECK_EQUAL(motorWithCustomConverter->getEncoderPosition(), 100000.);
}


BOOST_AUTO_TEST_CASE( testSoftLimits ){

  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getSoftwareLimitsEnabled() == false);

  // Limts have been set to [-1000, 1000] in fixture ctor, so this should throw
  BOOST_CHECK_THROW(_stepperMotor->setMaxPositionLimitInSteps(-1000), mtca4u::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->setMinPositionLimitInSteps(1000), mtca4u::MotorDriverException);

  //BOOST_CHECK_THROW(_stepperMotor->setMinPositionLimitInSteps(-1000), mtca4u::MotorDriverException);
  BOOST_CHECK_NO_THROW(_stepperMotor->setMaxPositionLimitInSteps(1000));
  BOOST_CHECK_NO_THROW(_stepperMotor->setMinPositionLimitInSteps(-1000));
  BOOST_CHECK_THROW(_stepperMotor->setMaxPositionLimitInSteps(-1000), mtca4u::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->setMaxPositionLimitInSteps(-2000), mtca4u::MotorDriverException);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() == 1000);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -1000);
}

BOOST_AUTO_TEST_CASE( testEnable ){

  BOOST_CHECK_EQUAL(_stepperMotor->getState(), "disabled");
  BOOST_CHECK_EQUAL(_stepperMotor->getEnabled(), false);
  _stepperMotor->setEnabled(true);

  BOOST_CHECK_EQUAL(_stepperMotor->getState(), "idle");
  BOOST_CHECK_EQUAL(_stepperMotor->getEnabled(), true);
  _stepperMotor->setEnabled(false);
  _motorControlerDummy->moveTowardsTarget(1);

  BOOST_CHECK_EQUAL(_stepperMotor->getEnabled(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->getState(), "disabled");

  _stepperMotor->setEnabled(true);
  BOOST_CHECK_EQUAL(_stepperMotor->getEnabled(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->getState(), "idle");
}

BOOST_AUTO_TEST_CASE( testSetActualPosition){

  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), true);
  BOOST_CHECK(_stepperMotor->isCalibrated() == false);
  BOOST_CHECK(_stepperMotor->getCalibrationTime() == 0);
  BOOST_CHECK_THROW(_stepperMotor->setTargetPositionInSteps(10), mtca4u::MotorDriverException);
  _stepperMotor->setActualPositionInSteps(0);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 0);
  BOOST_CHECK(_stepperMotor->isCalibrated() == true);
  BOOST_CHECK(_stepperMotor->getCalibrationTime() != 0);

  uint32_t timeEG = static_cast<uint32_t>(time(nullptr));

  _motorControlerDummy->setCalibrationTime(timeEG);
  BOOST_CHECK_EQUAL(_stepperMotor->getCalibrationTime(), timeEG);
}

BOOST_AUTO_TEST_CASE(testTranslateAxis){

  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->getMaxPositionLimitInSteps(), 1000);
  BOOST_CHECK_EQUAL(_stepperMotor->getMinPositionLimitInSteps(), -1000);
  BOOST_CHECK(_stepperMotor->translateAxisInSteps(100) == StepperMotorConfigurationResult::SUCCESS);
  BOOST_CHECK_EQUAL(_stepperMotor->getMaxPositionLimitInSteps(), 1100);
  BOOST_CHECK_EQUAL(_stepperMotor->getMinPositionLimitInSteps(), -900);
  BOOST_CHECK(_stepperMotor->translateAxisInSteps(-300) == StepperMotorConfigurationResult::SUCCESS);
  BOOST_CHECK_EQUAL(_stepperMotor->getMaxPositionLimitInSteps(), 800);
  BOOST_CHECK_EQUAL(_stepperMotor->getMinPositionLimitInSteps(), -1200);
  BOOST_CHECK(_stepperMotor->translateAxisInSteps(std::numeric_limits<int>::max()) == StepperMotorConfigurationResult::SUCCESS);
  BOOST_CHECK_EQUAL(_stepperMotor->getMaxPositionLimitInSteps(), std::numeric_limits<int>::max());
  BOOST_CHECK_EQUAL(_stepperMotor->getMinPositionLimitInSteps(), -1200 + std::numeric_limits<int>::max());
  BOOST_CHECK_NO_THROW(_stepperMotor->setMinPositionLimitInSteps(-1000));
  BOOST_CHECK_NO_THROW(_stepperMotor->setMaxPositionLimitInSteps(1000));
  BOOST_CHECK(_stepperMotor->translateAxisInSteps(std::numeric_limits<int>::min()) == StepperMotorConfigurationResult::SUCCESS);
  BOOST_CHECK_EQUAL(_stepperMotor->getMaxPositionLimitInSteps(), std::numeric_limits<int>::min() + 1000);
  BOOST_CHECK_EQUAL(_stepperMotor->getMinPositionLimitInSteps(), std::numeric_limits<int>::min());
  BOOST_CHECK_NO_THROW(_stepperMotor->setMaxPositionLimitInSteps(1000));
  BOOST_CHECK_NO_THROW(_stepperMotor->setMinPositionLimitInSteps(-1000));
  _stepperMotor->setActualPositionInSteps(0);
}

BOOST_AUTO_TEST_CASE(testMove){

  _stepperMotor->setSoftwareLimitsEnabled(true);
  _stepperMotor->setEnabled(true);

  BOOST_CHECK_EQUAL(_stepperMotor->getSoftwareLimitsEnabled(), true);
  BOOST_CHECK_THROW(_stepperMotor->setTargetPositionInSteps(100000), mtca4u::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->setTargetPositionInSteps(-100000), mtca4u::MotorDriverException);

  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(10));
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), false);

  waitForState("moving");
  BOOST_CHECK_THROW(_stepperMotor->setTargetPositionInSteps(20), mtca4u::MotorDriverException);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), false);
  BOOST_CHECK_THROW(_stepperMotor->moveRelativeInSteps(10), mtca4u::MotorDriverException);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), false);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 10);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), true);
  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(20));

  waitForState("moving");
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), false);
  BOOST_CHECK_THROW(_stepperMotor->setTargetPositionInSteps(20), mtca4u::MotorDriverException);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), false);
  BOOST_CHECK_THROW(_stepperMotor->moveRelativeInSteps(10), mtca4u::MotorDriverException);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), false);
  BOOST_CHECK_THROW(_stepperMotor->setSoftwareLimitsEnabled(false), mtca4u::MotorDriverException);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), false);
  BOOST_CHECK(_stepperMotor->getSoftwareLimitsEnabled() == true);
  BOOST_CHECK_THROW(_stepperMotor->setMinPositionLimitInSteps(20000), mtca4u::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->setMaxPositionLimitInSteps(40000), mtca4u::MotorDriverException);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() == 1000);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -1000);
  BOOST_CHECK(_stepperMotor->setActualPositionInSteps(10000) == StepperMotorConfigurationResult::ERROR_SYSTEM_IN_ACTION);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 10);
  BOOST_CHECK_THROW(_stepperMotor->setUserCurrentLimit(100), mtca4u::MotorDriverException);
  BOOST_CHECK_EQUAL(_stepperMotor->setUserSpeedLimit(100), false);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 20);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), true);

  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(30));

  waitForState("moving");
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 30);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), true);
}

BOOST_AUTO_TEST_CASE(testMoveRelative){

  _stepperMotor->setEnabled(true);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveRelativeInSteps(5));
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), false);

  waitForState("moving");
  double firstEncoderPosition = _stepperMotor->getEncoderPosition();
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  double secondEncoderPosition = _stepperMotor->getEncoderPosition();
  BOOST_CHECK(static_cast<int>(secondEncoderPosition - firstEncoderPosition) == 5);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 35);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), true);
}

BOOST_AUTO_TEST_CASE( testTargetPositionAndStart ){

  // Don't want to dig through relation of positions between test cases
  // just restore where we came from in the end
  int initialPosition = _stepperMotor->getCurrentPositionInSteps();
  BOOST_CHECK_NO_THROW(_stepperMotor->setEnabled(true));

  // Autostart should be true from test constructor
  BOOST_CHECK(_stepperMotor->getAutostart() == true);
  int targetPosSteps = 350;
  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(targetPosSteps));
  waitForState("moving");
  _motorControlerDummy->moveTowardsTarget(1.f);
  BOOST_CHECK(_stepperMotor->getTargetPositionInSteps() == targetPosSteps);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == targetPosSteps);
  _stepperMotor->waitForIdle();

  // Test movement with autostart set to false
  _stepperMotor->setAutostart(false);
  float targetPos = 432.f;

  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPosition(targetPos));
  BOOST_CHECK(_stepperMotor->getAutostart() == false);
  BOOST_CHECK_NO_THROW(_stepperMotor->start());

  waitForState("moving");
  _motorControlerDummy->moveTowardsTarget(1.f);
  _stepperMotor->waitForIdle();

  BOOST_CHECK(_stepperMotor->getTargetPosition() == targetPos);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == _stepperMotor->getTargetPosition());

  // Reset to true for other tests
  _stepperMotor->setAutostart(true);
  _stepperMotor->setActualPositionInSteps(initialPosition);
  BOOST_CHECK(_stepperMotor->getAutostart() == true);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), true);
}

BOOST_AUTO_TEST_CASE( testStop ){

  BOOST_CHECK_NO_THROW(_stepperMotor->setEnabled(true));
  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(50));

  waitForState("moving");
  _motorControlerDummy->moveTowardsTarget(0.4);
  _stepperMotor->stop();
  _stepperMotor->waitForIdle();

  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 41);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), true);
}

BOOST_AUTO_TEST_CASE( testEmergencyStop ){

  BOOST_CHECK_NO_THROW(_stepperMotor->setEnabled(true));
  BOOST_CHECK_EQUAL(_stepperMotor->getEnabled(), true);
  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(56));

  waitForState("moveState");
  _motorControlerDummy->moveTowardsTarget(0.8);
  _stepperMotor->emergencyStop();

  // After emergencyStop system should be in errorState,
  // power be disabled and calibration reset
  BOOST_CHECK(waitForState("error"));
  BOOST_CHECK_EQUAL(_stepperMotor->getEnabled(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);

  // Reset should bring the motor to disabled state
  _stepperMotor->resetError();
  BOOST_CHECK(waitForState("disabled"));

  // Set some new reference position, calibrated should become true,
  // and enable motor again
  _stepperMotor->setActualPositionInSteps(53);
  BOOST_CHECK_EQUAL(_stepperMotor->getEnabled(), false);

  _stepperMotor->setEnabled(true);
  BOOST_CHECK_EQUAL(_stepperMotor->getEnabled(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), true);
}

BOOST_AUTO_TEST_CASE( testFullStepping ){

  _motorControlerDummy->resetInternalStateToDefaults();
  _motorControlerDummy->setCalibrationTime(32);
  _stepperMotor->setEnabled(true);
  BOOST_CHECK(_stepperMotor->getEnabled() == true);
  BOOST_CHECK(_stepperMotor->isCalibrated() == true);
  BOOST_CHECK(_stepperMotor->isFullStepping() == false);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 0);
  BOOST_CHECK_NO_THROW(_stepperMotor->enableFullStepping());
  BOOST_CHECK(_stepperMotor->isFullStepping() == true);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), true);
  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(23));

  _stepperMotor->waitForIdle();

  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 0);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 1023);

  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(-12));
  _stepperMotor->waitForIdle();

  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 0);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 1023);

  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(-32));
  _stepperMotor->waitForIdle();

  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 0);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 1023);

  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(-33));
  waitForState("moving");

  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == -64);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 767);

  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(-48));
  _stepperMotor->waitForIdle();

  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == -64);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 767);

  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(48));
  waitForState("moving");

  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 64);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 255);

  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(67));
  _stepperMotor->waitForIdle();

  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 64);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 255);

  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(95));
  _stepperMotor->waitForIdle();

  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 64);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 255);

  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(96));
  waitForState("moving");

  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 128);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 511);

  BOOST_CHECK_NO_THROW(_stepperMotor->enableFullStepping(false));
}

BOOST_AUTO_TEST_CASE( testLocking ){

  std::cout << "  ** Performing testLocking" << std::endl;

  auto currentPosition = _stepperMotor->getCurrentPositionInSteps();

  BOOST_CHECK(_stepperMotor->isSystemIdle());
  _stepperMotor->setSoftwareLimitsEnabled(false);
  BOOST_CHECK_NO_THROW(_stepperMotor->setEnabled(true));

//  BOOST_CHECK_NO_THROW(
//  );
  std::thread moveThread(&StepperMotorChimeraTKFixture::moveThreadFcn, this);
  std::thread configThread(&StepperMotorChimeraTKFixture::configThreadFcn, this);

  moveThread.join();
  configThread.join();

  _stepperMotor->setActualPositionInSteps(currentPosition);
  _stepperMotor->setSoftwareLimitsEnabled(true);
}

BOOST_AUTO_TEST_CASE( testDisable ){

  _motorControlerDummy->resetInternalStateToDefaults();
  _motorControlerDummy->setCalibrationTime(32);
  BOOST_CHECK_NO_THROW(_stepperMotor->setEnabled(true));

  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPositionInSteps(96));
  BOOST_CHECK(waitForState("moving"));

  _motorControlerDummy->moveTowardsTarget(0.5);
  _stepperMotor->setEnabled(false);
  BOOST_CHECK(waitForState("disabled"));

  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 64);
  BOOST_CHECK(_motorControlerDummy->getMicroStepCount() == 255);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->getEnabled(), false);
  BOOST_CHECK(_stepperMotor->isCalibrated() == true);
  _stepperMotor->setActualPositionInSteps(68);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  _stepperMotor->setEnabled(true);
  BOOST_CHECK(_stepperMotor->getEnabled() == true);
}


BOOST_AUTO_TEST_CASE( testConverter ){

  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 68);
  BOOST_CHECK(_stepperMotor->getMinPositionLimit() == -1000);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimit() ==  1000);

  // Should throw because we attempt to set to nullptr
  _testUnitConverter.reset();
  BOOST_CHECK(
        _stepperMotor->setStepperMotorUnitsConverter(_testUnitConverter)
        == StepperMotorConfigurationResult::ERROR_INVALID_PARAMETER);

  // Now set to a proper converter (1:10)
  _testUnitConverter = std::make_shared<TestUnitConverter>();
  BOOST_CHECK_NO_THROW(_stepperMotor->setStepperMotorUnitsConverter(_testUnitConverter));
  BOOST_CHECK_CLOSE(_stepperMotor->getCurrentPosition(), 6.8, 1e-4);
  BOOST_CHECK(_stepperMotor->getMinPositionLimit() == -100);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimit() ==  100);

  _stepperMotor->setActualPosition(10.3f);
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

  BOOST_CHECK_NO_THROW(_stepperMotor->setEnabled(true));
  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPosition(10));
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), false);

  waitForState("moving");
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 10);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 100);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), true);

  BOOST_CHECK_NO_THROW(_stepperMotor->moveRelative(5));
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), false);

  waitForState("moving");
  // Attempt to set unitsConverter while moving, should throw
  BOOST_CHECK_THROW(_stepperMotor->setStepperMotorUnitsConverterToDefault(), mtca4u::MotorDriverException);
  _motorControlerDummy->moveTowardsTarget(1);
  _stepperMotor->waitForIdle();
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 15);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 150);
  BOOST_CHECK(_stepperMotor->getError() == ChimeraTK::StepperMotorError::NO_ERROR);
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), true);

  // Now, it should be safe to set the converter
  _testUnitConverter = std::make_shared<TestUnitConverter>();
  BOOST_CHECK_NO_THROW(_stepperMotor->setStepperMotorUnitsConverter(_testUnitConverter));
  _stepperMotor->waitForIdle();
  _stepperMotor->setTargetPositionInSteps(100);

  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), false);

  waitForState("moving");
  _motorControlerDummy->moveTowardsTarget(0.5);
  _stepperMotor->stop();

  _stepperMotor->waitForIdle();
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPosition(), 12.5);
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), 125);
  // In idle, resetting should be fine now
  _stepperMotor->setStepperMotorUnitsConverterToDefault();

  BOOST_CHECK_NO_THROW(_stepperMotor->setTargetPosition(100));
  BOOST_CHECK_EQUAL(_stepperMotor->isSystemIdle(), false);

  waitForState("moving");
  // Again, this should throw because motor is moving
  _testUnitConverter = std::make_shared<TestUnitConverter>();
  BOOST_CHECK(
        _stepperMotor->setStepperMotorUnitsConverter(_testUnitConverter)
        == StepperMotorConfigurationResult::ERROR_SYSTEM_IN_ACTION);

  _motorControlerDummy->moveTowardsTarget(1);
   _stepperMotor->waitForIdle();

  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 100);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == 100);
  BOOST_CHECK(_stepperMotor->getMinPositionLimit() == -900);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimit() ==  900);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == -900);
  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() ==  900);
}


BOOST_AUTO_TEST_CASE( testLogSettings ){

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


BOOST_AUTO_TEST_CASE( testGetSetSpeedLimit ){

  // setup
  auto dmapFile = ChimeraTK::getDMapFilePath();
  ChimeraTK::setDMapFilePath("./dummies.dmap");
  auto dummyModeStatus = mtca4u::MotorDriverCardFactory::instance().getDummyMode();
  mtca4u::MotorDriverCardFactory::instance().setDummyMode(false);
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
  auto md22_instance = boost::dynamic_pointer_cast<mtca4u::DFMC_MD22Dummy>(
      BackendFactory::getInstance().createBackend(
          "DFMC_MD22_PERSISTENT_BACKEND"));
  BOOST_ASSERT(md22_instance != NULL);
  mtca4u::TMC429OutputWord vMaxRegister = readDFMCDummyMotor0VMaxRegister(md22_instance);
 BOOST_CHECK(vMaxRegister.getDATA() == 1048);

  //teardown
    mtca4u::MotorDriverCardFactory::instance().setDummyMode(dummyModeStatus);
    ChimeraTK::setDMapFilePath(dmapFile);
}


BOOST_AUTO_TEST_CASE( testGetSetCurrent ){

  auto dmapFile = ChimeraTK::getDMapFilePath();
  ChimeraTK::setDMapFilePath("./dummies.dmap");
  auto dummyModeStatus = mtca4u::MotorDriverCardFactory::instance().getDummyMode();
  mtca4u::MotorDriverCardFactory::instance().setDummyMode(false);
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
  auto md22_instance = boost::dynamic_pointer_cast<mtca4u::DFMC_MD22Dummy>(
      BackendFactory::getInstance().createBackend("DFMC_MD22_PERSISTENT_BACKEND"));
  BOOST_ASSERT(md22_instance != nullptr);
  mtca4u::StallGuardControlData stallGuard = readDFMCDummyMotor0CurrentScale(md22_instance);
  BOOST_CHECK(stallGuard.getCurrentScale() == 2);

  //teardown
    mtca4u::MotorDriverCardFactory::instance().setDummyMode(dummyModeStatus);
    ChimeraTK::setDMapFilePath(dmapFile);
}

BOOST_AUTO_TEST_SUITE_END()


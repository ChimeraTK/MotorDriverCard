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

static const std::string stepperMotorDeviceName("STEPPER-MOTOR-DUMMY");
static const std::string stepperMotorDeviceConfigFile("VT21-MotorDriverCardConfig.xml");
static const std::string dmapPath(".");
static const std::string moduleName("");
static mtca4u::TMC429OutputWord readDFMCDummyMotor0VMaxRegister(boost::shared_ptr<mtca4u::DFMC_MD22Dummy>& dfmc_md22);
static mtca4u::StallGuardControlData readDFMCDummyMotor0CurrentScale(boost::shared_ptr<mtca4u::DFMC_MD22Dummy>& dfmc_md2);

class TestUnitConveter : public ChimeraTK::StepperMotorUnitsConverter {
public:
  TestUnitConveter() {
  };

  virtual float stepsToUnits(int steps) {
    return float(steps/10.0);
  };
  virtual int unitsToSteps(float units) {
    return int(10.0*units);
  };
};

class StepperMotorChimeraTKTest{
public:
  StepperMotorChimeraTKTest();
  void testSoftLimits();
  //  void testSetActualPosition();
  //  void testGetactualPosition();
  //  void testMove();
  //  void testMoveDelta();
  //  void testStop();
  //  void testEmergencyStop();
private:

  boost::shared_ptr<ChimeraTK::StepperMotor> _stepperMotor;
  boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;
  boost::shared_ptr<TestUnitConveter> _testUnitConveter;
};

class StepperMotorChimeraTKTestSuite : public test_suite {
public:

  StepperMotorChimeraTKTestSuite() : test_suite("Stepper Motor ChimeraTK suite") {
    // create an instance of the test class
    boost::shared_ptr<StepperMotorChimeraTKTest> stepperMotorChimeraTK(new StepperMotorChimeraTKTest);
    add(BOOST_CLASS_TEST_CASE(&StepperMotorChimeraTKTest::testSoftLimits, stepperMotorChimeraTK));
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

  _testUnitConveter.reset(new TestUnitConveter);

  mtca4u::MotorDriverCardFactory::instance().setDummyMode();
  _motorControlerDummy = boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(deviceFileName,  moduleName, stepperMotorDeviceConfigFile)->getMotorControler(0));

  _stepperMotor.reset(new ChimeraTK::StepperMotor(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));

  //!!!! CHANGE THIS FOR LINEAR STEPER MOTOR TESTS
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);
}

void StepperMotorChimeraTKTest::testSoftLimits(){
  _stepperMotor->setMaxPositionLimit(10000);
  _stepperMotor->setMinPositionLimit(-10000);

}

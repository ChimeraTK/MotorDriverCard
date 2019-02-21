
#include <sstream>

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "LinearStepperMotor.h"
#include "MotorControlerDummy.h"
#include "MotorDriverCardFactory.h"
#include "testConfigConstants.h"
#include <ChimeraTK/DMapFilesParser.h>

#include <boost/thread.hpp>

using namespace mtca4u;

// static const unsigned int THE_ID = 17;

static const std::string stepperMotorDeviceName("STEPPER-MOTOR-DUMMY");
static const std::string
    stepperMotorDeviceConfigFile("VT21-MotorDriverCardConfig.xml");
static const std::string dmapPath(".");
static const std::string moduleName("");

class TestUnitConveter : public StepperMotorUnitsConverter {
public:
  TestUnitConveter(){};

  virtual float stepsToUnits(int steps) { return float(steps / 10.0); };

  virtual int unitsToSteps(float units) { return int(10.0 * units); };
};

class LinearStepperMotorTest {
public:
  LinearStepperMotorTest();

  void testSoftLimits();
  void testStop();
  void testEmergencyStop();
  void testCalibration();
  void threadCalibration(int i);
  void testSetPositionAs();

  void testMoveToPosition();
  void threadMoveToPostion(int i);
  void testIsMoving();

private:
  boost::shared_ptr<LinearStepperMotor> _stepperMotor;
  boost::shared_ptr<MotorControlerDummy> _motorControlerDummy;

  boost::shared_ptr<TestUnitConveter> _testUnitConveter;
  bool isMoveToNegativeEndSwitchRequested();
  bool isMoveToPositiveEndSwitchRequested();
  bool isMoveToValidPositionRequested();
};

class LinearStepperMotorTestSuite : public test_suite {
public:
  LinearStepperMotorTestSuite() : test_suite("LinearStepperMotor suite") {
    // create an instance of the test class
    boost::shared_ptr<LinearStepperMotorTest> stepperMotorTest(
        new LinearStepperMotorTest);

    add(BOOST_CLASS_TEST_CASE(&LinearStepperMotorTest::testSoftLimits,
                              stepperMotorTest));

    add(BOOST_CLASS_TEST_CASE(&LinearStepperMotorTest::testStop,
                              stepperMotorTest));

    add(BOOST_CLASS_TEST_CASE(&LinearStepperMotorTest::testEmergencyStop,
                              stepperMotorTest));

    add(BOOST_CLASS_TEST_CASE(&LinearStepperMotorTest::testSetPositionAs,
                              stepperMotorTest));

    add(BOOST_CLASS_TEST_CASE(&LinearStepperMotorTest::testMoveToPosition,
                              stepperMotorTest));

    add(BOOST_CLASS_TEST_CASE(&LinearStepperMotorTest::testCalibration,
                              stepperMotorTest));

    add(BOOST_CLASS_TEST_CASE(&LinearStepperMotorTest::testIsMoving,
                              stepperMotorTest));
  }
};

test_suite *init_unit_test_suite(int /*argc*/, char * /*argv*/ []) {
  framework::master_test_suite().p_name.value = "LinearStepperMotor test suite";
  return new LinearStepperMotorTestSuite;
}

LinearStepperMotorTest::LinearStepperMotorTest()
    : _stepperMotor(), _motorControlerDummy(), _testUnitConveter() {

  // std::string
  // deviceFileName(DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).dev_file);
  std::string deviceFileName(DMapFilesParser(dmapPath)
                                 .getdMapFileElem(stepperMotorDeviceName)
                                 .deviceName);
  std::string mapFileName(DMapFilesParser(dmapPath)
                              .getdMapFileElem(stepperMotorDeviceName)
                              .mapFileName);

  _testUnitConveter.reset(new TestUnitConveter);

  //_motorControlerDummy =
  //boost::dynamic_pointer_cast<MotorControlerDummy>(MotorDriverCardFactory::instance().createMotorDriverCard(deviceFileName,
  //mapFileName, moduleName,
  //stepperMotorDeviceConfigFile)->getMotorControler(0));
  MotorDriverCardFactory::instance().setDummyMode();
  _motorControlerDummy = boost::dynamic_pointer_cast<MotorControlerDummy>(
      MotorDriverCardFactory::instance()
          .createMotorDriverCard(deviceFileName, moduleName,
                                 stepperMotorDeviceConfigFile)
          ->getMotorControler(0));

  _stepperMotor.reset(new LinearStepperMotor(stepperMotorDeviceName, moduleName,
                                             0, stepperMotorDeviceConfigFile));

  //!!!! CHANGE THIS FOR LINEAR STEPER MOTOR TESTS
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(true);
}

void LinearStepperMotorTest::testSoftLimits() {
  std::cout << "testSoftLimits()" << std::endl;

  _stepperMotor->setEnabled(true);
  _testUnitConveter.reset(new TestUnitConveter());
  _stepperMotor->setStepperMotorUnitsConverter(_testUnitConveter);

  _stepperMotor->setSoftwareLimitsEnabled(true);
  BOOST_CHECK(_stepperMotor->getSoftwareLimitsEnabled() == true);

  _stepperMotor->setSoftwareLimitsEnabled(false);
  BOOST_CHECK(_stepperMotor->getSoftwareLimitsEnabled() == false);

  _stepperMotor->setMaxPositionLimitInSteps(150);
  _stepperMotor->setMinPositionLimitInSteps(50);

  BOOST_CHECK(_stepperMotor->getMaxPositionLimit() == 15);
  BOOST_CHECK(_stepperMotor->getMinPositionLimit() == 5);

  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() == 150);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == 50);

  _stepperMotor->setMaxPositionLimit(150);
  _stepperMotor->setMinPositionLimit(50);

  BOOST_CHECK(_stepperMotor->getMaxPositionLimit() == 150);
  BOOST_CHECK(_stepperMotor->getMinPositionLimit() == 50);

  BOOST_CHECK(_stepperMotor->getMaxPositionLimitInSteps() == 1500);
  BOOST_CHECK(_stepperMotor->getMinPositionLimitInSteps() == 500);
  /*
      _stepperMotor->setSoftwareLimitsEnabled(true); FIXME repeated tests from
     StepperMotor? BOOST_CHECK(_stepperMotor->getSoftwareLimitsEnabled() ==
     true);

      _stepperMotor->setSoftwareLimitsEnabled(false);
      BOOST_CHECK(_stepperMotor->getSoftwareLimitsEnabled() == false);


      _stepperMotor->setMaxPositionLimit(150);
      _stepperMotor->setMinPositionLimit(50);

      BOOST_CHECK(_stepperMotor->getMaxPositionLimit() == 150);
      BOOST_CHECK(_stepperMotor->getMinPositionLimit() == 50);


      _stepperMotor->setSoftwareLimitsEnabled(true);

      _stepperMotor->setTargetPosition(200);
      BOOST_CHECK(_stepperMotor->getTargetPosition() == 150);

      _stepperMotor->setTargetPosition(0);
      BOOST_CHECK(_stepperMotor->getTargetPosition() == 50);

      _stepperMotor->setTargetPosition(100);
      BOOST_CHECK(_stepperMotor->getTargetPosition() == 100);

      _stepperMotor->setSoftwareLimitsEnabled(false);

      _stepperMotor->setTargetPosition(200);
      BOOST_CHECK(_stepperMotor->getTargetPosition() == 200);

      _stepperMotor->setTargetPosition(0);
      BOOST_CHECK(_stepperMotor->getTargetPosition() == 0);

      _stepperMotor->setTargetPosition(100);
      BOOST_CHECK(_stepperMotor->getTargetPosition() == 100);


      //should trigger error state - target position should not be changed
      _stepperMotor->setMaxPositionLimit(50);
      _stepperMotor->setMinPositionLimit(150);

      _stepperMotor->setSoftwareLimitsEnabled(true);

      _stepperMotor->setTargetPosition(200);
      BOOST_CHECK(_stepperMotor->getTargetPosition() == 100);

      _stepperMotor->setTargetPosition(0);
      BOOST_CHECK(_stepperMotor->getTargetPosition() == 100);

      _stepperMotor->setTargetPosition(100);
      BOOST_CHECK(_stepperMotor->getTargetPosition() == 100);

      _stepperMotor->setSoftwareLimitsEnabled(false);

      _stepperMotor->setTargetPosition(200);
      BOOST_CHECK(_stepperMotor->getTargetPosition() == 200);

      _stepperMotor->setTargetPosition(0);
      BOOST_CHECK(_stepperMotor->getTargetPosition() == 0);

      _stepperMotor->setTargetPosition(100);
      BOOST_CHECK(_stepperMotor->getTargetPosition() == 100);


      _stepperMotor->setMaxPositionLimit(150);
      _stepperMotor->setMinPositionLimit(50);
      _stepperMotor->setTargetPosition(200);
      _stepperMotor->setSoftwareLimitsEnabled(true);

      BOOST_CHECK((_stepperMotor->getStatusAndError().status ==
     StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON) == true);

      _stepperMotor->setSoftwareLimitsEnabled(false);
      _stepperMotor->setTargetPosition(0);
      _stepperMotor->setSoftwareLimitsEnabled(true);

      BOOST_CHECK((_stepperMotor->getStatusAndError().status ==
     StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON) == true);

      _stepperMotor->setSoftwareLimitsEnabled(false);
      _stepperMotor->setEnabled(false);*/
}

void LinearStepperMotorTest::testStop() {
  std::cout << "testStop()" << std::endl;

  _motorControlerDummy->setTargetPosition(0);
  _motorControlerDummy->setActualPosition(0);

  _stepperMotor->setEnabled(true);

  _stepperMotor->setAutostart(false);

  // test start function
  _stepperMotor->setTargetPosition(-500);
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              StepperMotorStatusTypes::M_NOT_IN_POSITION);
  _stepperMotor->start();
  BOOST_CHECK(_motorControlerDummy->getTargetPosition() == (-500 * 10));
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              StepperMotorStatusTypes::M_IN_MOVE);
  _motorControlerDummy->moveTowardsTarget(0.5);
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              StepperMotorStatusTypes::M_IN_MOVE);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == (-500 * 0.5));
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == (-500 * 0.5 * 10));
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() ==
              _motorControlerDummy->getActualPosition());

  _motorControlerDummy->moveTowardsTarget(0.5);
  _stepperMotor->stop();
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              StepperMotorStatusTypes::M_OK);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == (-500 * 0.75));
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() ==
              _motorControlerDummy->getActualPosition());
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() ==
              _motorControlerDummy->getTargetPosition());
}

void LinearStepperMotorTest::testEmergencyStop() {
  std::cout << "testEmergencyStop()" << std::endl;

  _motorControlerDummy->setTargetPosition(0);
  _motorControlerDummy->setActualPosition(0);

  _stepperMotor->setEnabled(true);

  _stepperMotor->setAutostart(false);

  // test start function
  _stepperMotor->setTargetPositionInSteps(500);
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              StepperMotorStatusTypes::M_NOT_IN_POSITION);
  _stepperMotor->start();
  BOOST_CHECK(_motorControlerDummy->getTargetPosition() == (500));
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              StepperMotorStatusTypes::M_IN_MOVE);
  _motorControlerDummy->moveTowardsTarget(0.5);

  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              StepperMotorStatusTypes::M_IN_MOVE);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == (50 * 0.5));
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() ==
              _motorControlerDummy->getActualPosition());

  _motorControlerDummy->moveTowardsTarget(0.5);
  _stepperMotor->emergencyStop();
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              StepperMotorStatusTypes::M_DISABLED);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == (50 * 0.75));
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() ==
              _motorControlerDummy->getActualPosition());
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() ==
              _motorControlerDummy->getTargetPosition());
  BOOST_CHECK(_motorControlerDummy->getTargetPosition() ==
              _motorControlerDummy->getActualPosition());
}

void LinearStepperMotorTest::testSetPositionAs() {
  std::cout << "testSetPositionAs()" << std::endl;

  _motorControlerDummy->setTargetPosition(0);
  _motorControlerDummy->setActualPosition(0);

  _stepperMotor->setAutostart(false);

  _stepperMotor->setEnabled(true);
  _stepperMotor->setCurrentPositionAs(500);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 500);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() ==
              _motorControlerDummy->getActualPosition());
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() ==
              _motorControlerDummy->getTargetPosition());
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              StepperMotorStatusTypes::M_OK);

  _stepperMotor->setEnabled(false);
  _stepperMotor->setCurrentPositionAs(-200);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == -200);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() ==
              _motorControlerDummy->getActualPosition());
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() ==
              _motorControlerDummy->getTargetPosition());
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              StepperMotorStatusTypes::M_DISABLED);
}

void LinearStepperMotorTest::testMoveToPosition() {
  std::cout << "testMoveToPosition()" << std::endl;

  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor->setEnabled(true);

  boost::thread workerThread(&LinearStepperMotorTest::threadMoveToPostion, this,
                             1);
  auto statusAndError = _stepperMotor->moveToPosition(500);
  workerThread.join();
  BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_OK);
  BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_NO_ERROR);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == 500);

  // set motor in error state - configuration error end test routine
  _stepperMotor->setMaxPositionLimit(50);
  _stepperMotor->setMinPositionLimit(100);
  _stepperMotor->setSoftwareLimitsEnabled(true);
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              StepperMotorStatusTypes::M_ERROR);
  BOOST_CHECK(
      _stepperMotor->getStatusAndError().error ==
      StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX);
  statusAndError = _stepperMotor->moveToPosition(0);
  BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_ERROR);
  BOOST_CHECK(
      statusAndError.error ==
      StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX);
  BOOST_CHECK(_motorControlerDummy->getTargetPosition() == 5000);
  _stepperMotor->setSoftwareLimitsEnabled(false);

  auto currentPosition = _stepperMotor->getCurrentPosition();
  _stepperMotor->setMaxPositionLimit(currentPosition - 100);
  _stepperMotor->setMinPositionLimit(currentPosition - 200);
  _stepperMotor->setSoftwareLimitsEnabled(true);
  boost::thread workerThread1(&LinearStepperMotorTest::threadMoveToPostion,
                              this, 1);
  statusAndError = _stepperMotor->moveToPosition(currentPosition + 100);
  workerThread1.join();
  BOOST_CHECK(statusAndError.status ==
              LinearStepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON);
  BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_NO_ERROR);
  BOOST_CHECK(_motorControlerDummy->getTargetPosition() ==
              (currentPosition - 100) * 10);
  _stepperMotor->setSoftwareLimitsEnabled(false);

  // test stop blocking function
  boost::thread workedThread2(&LinearStepperMotorTest::threadMoveToPostion,
                              this, 2);
  int targetPosition = _motorControlerDummy->getActualPosition() + 300;
  statusAndError = _stepperMotor->moveToPositionInSteps(targetPosition);
  BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_OK);
  BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_NO_ERROR);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() != targetPosition);
  // std::cout << "Status main 2 " << statusAndError.status  << " Error main "
  // << statusAndError.error << " POS:
  // "<<_motorControlerDummy->getActualPosition()  << std::endl;
  workedThread2.join();

  // test when we will succesfully finish movement but with autostart ON
  _stepperMotor->setAutostart(true);
  targetPosition = _motorControlerDummy->getActualPosition() + 300;
  boost::thread workedThread3_2(&LinearStepperMotorTest::threadMoveToPostion,
                                this, 1);
  statusAndError = _stepperMotor->moveToPositionInSteps(targetPosition);
  BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_OK);
  BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_NO_ERROR);
  BOOST_CHECK(_stepperMotor->getCurrentPositionInSteps() == targetPosition);
  _stepperMotor->setAutostart(false);
  workedThread3_2.join();

  // assume we are in the positive end switch
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor->setTargetPosition(
      _motorControlerDummy->getPositiveEndSwitch());
  _stepperMotor->start();
  _motorControlerDummy->moveTowardsTarget(1);
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON);

  // try to move more in positive - should be timeout
  currentPosition = _stepperMotor->getCurrentPosition();
  boost::thread workedThread4_1(&LinearStepperMotorTest::threadMoveToPostion,
                                this, 3);
  statusAndError = _stepperMotor->moveToPosition(currentPosition + 100);
  workedThread4_1.join();
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == currentPosition);

  boost::thread workedThread4_2(&LinearStepperMotorTest::threadMoveToPostion,
                                this, 1);
  statusAndError = _stepperMotor->moveToPosition(currentPosition - 100);
  workedThread4_2.join();
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              LinearStepperMotorStatusTypes::M_OK);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == (currentPosition - 100));

  // now the same for negative end  switch
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor->setTargetPosition(
      _motorControlerDummy->getNEgativeEndSwitch());
  _stepperMotor->start();
  _motorControlerDummy->moveTowardsTarget(1);
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON);

  // try to move more in negative - should be timeout; same comments as above
  currentPosition = _stepperMotor->getCurrentPosition();
  boost::thread workedThread4_3(&LinearStepperMotorTest::threadMoveToPostion,
                                this, 4);
  statusAndError = _stepperMotor->moveToPosition(currentPosition - 100);
  workedThread4_3.join();
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == currentPosition);

  boost::thread workedThread4_4(&LinearStepperMotorTest::threadMoveToPostion,
                                this, 1);
  statusAndError = _stepperMotor->moveToPosition(currentPosition + 100);
  workedThread4_4.join();
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              LinearStepperMotorStatusTypes::M_OK);
  BOOST_CHECK(_stepperMotor->getCurrentPosition() == (currentPosition + 100));
}

void LinearStepperMotorTest::threadMoveToPostion(int testCase) {

  if (testCase == 1) {
    while (_stepperMotor->isMoving() ==
           false) { // the parent thread has not called moveToPosition yet.
      usleep(1000);
    }
    // simulate the movement
    _motorControlerDummy->moveTowardsTarget(1);
  }

  if (testCase == 2) {
    while (_stepperMotor->isMoving() ==
           false) { // the parent thread has not called moveToPosition yet.
      usleep(1000);
    }
    // simulate the movement
    _motorControlerDummy->moveTowardsTarget(0.4, false);
    _stepperMotor->stop();
  }

  if (testCase == 3) {
    while (_motorControlerDummy->getReferenceSwitchData()
               .getPositiveSwitchActive() == false) {
      usleep(1000);
    }
    _motorControlerDummy->moveTowardsTarget(1);
  }

  if (testCase == 4) {
    while (_motorControlerDummy->getReferenceSwitchData()
               .getNegativeSwitchActive() == false) {
      usleep(1000);
    }
    _motorControlerDummy->moveTowardsTarget(1);
  }
}

void LinearStepperMotorTest::testCalibration() {
  std::cout << "testCalibration()" << std::endl;
  _motorControlerDummy->resetInternalStateToDefaults();

  // check calibration status
  StepperMotorCalibrationStatus calibrationStatus =
      _stepperMotor->getCalibrationStatus();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_NOT_CALIBRATED);

  _stepperMotor->setEnabled(false);
  calibrationStatus = _stepperMotor->calibrateMotor();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED);

  _stepperMotor->setEnabled(true);
  // in this tread we will try to run second calibration and check its status
  boost::thread workedThread1(&LinearStepperMotorTest::threadCalibration, this,
                              1);
  calibrationStatus = _stepperMotor->calibrateMotor();
  workedThread1.join();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_CALIBRATED);

  // test situation when end switches are not available
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  calibrationStatus = _stepperMotor->calibrateMotor();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);
  calibrationStatus = _stepperMotor->calibrateMotor();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  calibrationStatus = _stepperMotor->calibrateMotor();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE);

  // test situation when we have error when both end-switches are ON
  _motorControlerDummy->moveTowardsTarget(
      0.1, false, true); // in this case both end switches should get active in
                         // the same time
  calibrationStatus = _stepperMotor->calibrateMotor();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED);

  // test stop calibration
  _motorControlerDummy
      ->resetInternalStateToDefaults(); // just to reset flags - _blockMotor and
                                        // _bothEndSwitchesAlwaysOn
  boost::thread workedThread2(&LinearStepperMotorTest::threadCalibration, this,
                              2);
  calibrationStatus = _stepperMotor->calibrateMotor();
  workedThread2.join();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER);

  // successful calibration
  boost::thread workedThread3(&LinearStepperMotorTest::threadCalibration, this,
                              3);
  calibrationStatus = _stepperMotor->calibrateMotor();
  workedThread3.join();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_CALIBRATED);
  _stepperMotor->setCurrentPositionAs(0);
  BOOST_CHECK((_stepperMotor->getNegativeEndSwitchPosition() * -1) ==
              _stepperMotor->getPositiveEndSwitchPosition());
  BOOST_CHECK((_stepperMotor->getNegativeEndSwitchPositionInSteps() == -10000));
  BOOST_CHECK((_stepperMotor->getPositiveEndSwitchPositionInSteps() == 10000));
  _stepperMotor->setCurrentPositionInStepsAs(500);
  BOOST_CHECK(_stepperMotor->getNegativeEndSwitchPositionInSteps() ==
              -10000 + 500);
  BOOST_CHECK(_stepperMotor->getPositiveEndSwitchPositionInSteps() ==
              10000 + 500);

  // test how emergency stop changes the status of object
  _stepperMotor->emergencyStop();
  BOOST_CHECK(_stepperMotor->getCalibrationStatus() ==
              StepperMotorCalibrationStatusType::M_NOT_CALIBRATED);
  BOOST_CHECK(_stepperMotor->getEnabled() == false);

  // now stop calibration after negative end switch reached
  _stepperMotor->setEnabled(true);
  boost::thread workedThread4(&LinearStepperMotorTest::threadCalibration, this,
                              4);
  calibrationStatus = _stepperMotor->calibrateMotor();
  workedThread4.join();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER);

  // now stop calibration after negative end switch reached and positive end
  // switch reached; Has timing issues
  _motorControlerDummy
      ->resetInternalStateToDefaults(); // just to reset flags - _blockMotor and
                                        // _bothEndSwitchesAlwaysOn
  boost::thread workedThread5(&LinearStepperMotorTest::threadCalibration, this,
                              5);
  calibrationStatus = _stepperMotor->calibrateMotor();
  workedThread5.join();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER);

  // now stop calibration after negative end switch reached and positive end
  // switch reached, but with error trigger - so motor is not calibrated
  boost::thread workedThread6(&LinearStepperMotorTest::threadCalibration, this,
                              6);
  calibrationStatus = _stepperMotor->calibrateMotor();
  workedThread6.join();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED);

  // now stop calibration after negative end switch reached and positive end
  // switch reached, but with error trigger - so motor is not calibrated
  _motorControlerDummy->resetInternalStateToDefaults();
  boost::thread workedThread7(&LinearStepperMotorTest::threadCalibration, this,
                              7);
  calibrationStatus = _stepperMotor->calibrateMotor();
  workedThread7.join();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED);

  // do one more time correct calibration
  _motorControlerDummy->resetInternalStateToDefaults();
  boost::thread workedThread3a(&LinearStepperMotorTest::threadCalibration, this,
                               3);
  calibrationStatus = _stepperMotor->calibrateMotor();
  workedThread3a.join();
  BOOST_CHECK(calibrationStatus ==
              StepperMotorCalibrationStatusType::M_CALIBRATED);
  _stepperMotor->setCurrentPositionAs(0);
  BOOST_CHECK((_stepperMotor->getNegativeEndSwitchPosition() * -1) ==
              _stepperMotor->getPositiveEndSwitchPosition());
}

void LinearStepperMotorTest::threadCalibration(int testCase) {
  if (testCase == 1) {

    // we start only once the calibration routine from the parent thread has
    // started moving the motor
    while (_stepperMotor->isMoving() == false) {
      usleep(1000);
    }
    StepperMotorCalibrationStatus calibrationStatus =
        _stepperMotor->calibrateMotor();

    BOOST_CHECK(calibrationStatus ==
                StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS);

    while (isMoveToNegativeEndSwitchRequested() == false) {
      usleep(1000);
    }
    // go to negative end switch once the calibration routine has asked for it
    _motorControlerDummy->moveTowardsTarget(1.0);

    while (isMoveToPositiveEndSwitchRequested() == false) {
      usleep(1000);
    }
    // go to positive end switch once the calibration routine has asked for it
    _motorControlerDummy->moveTowardsTarget(1.0);

    while (isMoveToValidPositionRequested() == false) {
      usleep(1000);
    }
    // go to middle of range between end switches
    _motorControlerDummy->moveTowardsTarget(1.0);
  }

  if (testCase == 2) {
    // we start only once the calibration routine from the parent thread has
    // started moving the motor
    while (_stepperMotor->isMoving() == false) {
      usleep(1000);
    }
    _stepperMotor->stop(); // this should stop moveToPosition and calibration
  }

  if (testCase == 3) {
    while (isMoveToNegativeEndSwitchRequested() == false) {
      usleep(1000);
    }
    // go to negative end switch once the calibration routine has asked for it
    _motorControlerDummy->moveTowardsTarget(1.0);

    while (isMoveToPositiveEndSwitchRequested() == false) {
      usleep(1000);
    }
    // go to positive end switch once the calibration routine has asked for it
    _motorControlerDummy->moveTowardsTarget(1.0);

    while (isMoveToValidPositionRequested() == false) {
      usleep(1000);
    }
    // go to middle of range between end switches
    _motorControlerDummy->moveTowardsTarget(1.0);
  }

  if (testCase == 4) { // FIXME: same as 2?
    while (isMoveToNegativeEndSwitchRequested() == false) {
      usleep(1000);
    }
    // go to negative end switch once the calibration routine has asked for it
    _motorControlerDummy->moveTowardsTarget(1.0);

    while (isMoveToPositiveEndSwitchRequested() == false) {
      usleep(1000);
    }

    _stepperMotor->stop();
  }

  if (testCase == 5) {
    while (isMoveToNegativeEndSwitchRequested() == false) {
      usleep(1000);
    }
    // go to negative end switch once the calibration routine has asked for it
    _motorControlerDummy->moveTowardsTarget(1.0);

    while (isMoveToPositiveEndSwitchRequested() == false) {
      usleep(1000);
    }
    // go to positive end switch once the calibration routine has asked for it
    _motorControlerDummy->moveTowardsTarget(1.0);

    while (isMoveToValidPositionRequested() == false) {
      usleep(1000);
    }
    _stepperMotor->stop();
  }

  if (testCase == 6) {
    while (isMoveToNegativeEndSwitchRequested() == false) {
      usleep(1000);
    }
    // go to negative end switch once the calibration routine has asked for it
    _motorControlerDummy->moveTowardsTarget(1.0);

    while (isMoveToPositiveEndSwitchRequested() == false) {
      usleep(1000);
    }
    // go to positive end switch once the calibration routine has asked for it
    _motorControlerDummy->moveTowardsTarget(1.0);

    while (isMoveToValidPositionRequested() == false) {
      usleep(1000);
    }
    _motorControlerDummy->moveTowardsTarget(
        0, false, true); // trigger both end switches error
  }

  if (testCase == 7) {
    while (isMoveToNegativeEndSwitchRequested() == false) {
      usleep(1000);
    }
    // go to negative end switch once the calibration routine has asked for it
    _motorControlerDummy->moveTowardsTarget(1.0);

    while (isMoveToPositiveEndSwitchRequested() == false) {
      usleep(1000);
    }
    _motorControlerDummy->moveTowardsTarget(
        0, false, true); // trigger both end switches error
  }

  //_motorControlerDummy->moveTowardsTarget(1, false);
}
void LinearStepperMotorTest::testIsMoving() {
  // set up the motor position to a value in a good operating range.
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor->setEnabled(true);
  _stepperMotor->setSoftwareLimitsEnabled(false);
  _stepperMotor->setAutostart(true);

  // The actual tests
  // 1. Move between the +/- endwitch range
  BOOST_CHECK(_stepperMotor->isMoving() == false);
  _stepperMotor->setTargetPosition(100);
  BOOST_CHECK(_stepperMotor->isMoving() == true);
  _motorControlerDummy->moveTowardsTarget(.5);
  BOOST_CHECK(_stepperMotor->isMoving() == true);
  _motorControlerDummy->moveTowardsTarget(1); // motor at target position;
  BOOST_CHECK(_stepperMotor->isMoving() == false);

  // 2. Move beyond an end switch
  _motorControlerDummy->resetInternalStateToDefaults();
  auto negativeEndSwitchPosition = _motorControlerDummy->getNEgativeEndSwitch();
  BOOST_CHECK(_stepperMotor->isMoving() == false);
  _stepperMotor->setTargetPosition(negativeEndSwitchPosition - 100);
  BOOST_CHECK(_stepperMotor->isMoving() == true);
  _motorControlerDummy->moveTowardsTarget(1); // at negative end switch
  BOOST_CHECK(_stepperMotor->isMoving() == false);
  BOOST_CHECK(_stepperMotor->getStatusAndError().status ==
              LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON);
}

bool LinearStepperMotorTest::isMoveToNegativeEndSwitchRequested() {
  return _motorControlerDummy->getTargetPosition() <=
         _motorControlerDummy->getNEgativeEndSwitch();
}

bool LinearStepperMotorTest::isMoveToPositiveEndSwitchRequested() {
  return _motorControlerDummy->getTargetPosition() >=
         _motorControlerDummy->getPositiveEndSwitch();
}

bool LinearStepperMotorTest::isMoveToValidPositionRequested() {
  return (_motorControlerDummy->getTargetPosition() <
          _motorControlerDummy->getPositiveEndSwitch()) &&
         (_motorControlerDummy->getTargetPosition() >
          _motorControlerDummy->getNEgativeEndSwitch());
}

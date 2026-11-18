#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE StepperMotorRotaryTest

#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>

#include <chrono>
#include <memory>
#include <thread>

using namespace boost::unit_test_framework;

#include "DFMC_MD22Dummy.h"
#include "MotorControlerDummy.h"
#include "MotorDriverCard.h"
#include "MotorDriverCardFactory.h"
#include "ReferenceStateMachine.h"
#include "RotaryStepperMotor.h"
#include "StepperMotor.h"
#include "testConfigConstants.h"
#include "TMC429Constants.h"

using namespace ChimeraTK::MotorDriver;

static const std::string stepperMotorDeviceConfigFile("VT21-MotorDriverCardConfig.xml");
static const std::string moduleName("");

// The rotary state machine searches the home switch in steps of
// RotaryStepperMotorStateMachine::END_SWITCH_SEARCH_OFFSET (5000 steps), so the
// dummy end switch must be reachable with a single move from the origin.
static const int POS_POSITIVE_ENDSWITCH_MOTORCONTROLLER = 50000;

class StepperMotorRotaryTestFixture {
 public:
  StepperMotorRotaryTestFixture();
  void waitToSetTargetPos(int targetPos);
  void waitForPositiveEndSwitchActive();
  bool waitForState(std::string stateName, unsigned timeoutInSeconds);
  bool waitForCalibrationFinished(unsigned timeoutInSeconds);

  /// Drive the dummy "hardware" until the end switch is reached.
  void driveUntilEndSwitchActive();

  /// Perform a successful calibration and check the resulting state.
  void performCalibrationForTest();

  // Helper functions to check protected members in test
  bool getCalibrationFailed();

 protected:
  StepperMotorParameters _stepperMotorParameters;
  std::shared_ptr<RotaryStepperMotor> _stepperMotor;
  boost::shared_ptr<mtca4u::MotorDriverCard> _motorDriverCard;
  boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;
};

StepperMotorRotaryTestFixture::StepperMotorRotaryTestFixture()
: _stepperMotorParameters(), _stepperMotor(), _motorControlerDummy() {
  mtca4u::MotorDriverCardFactory::instance().setDummyMode();
  _motorDriverCard = mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(
      DUMMY_DEVICE_FILE_NAME, moduleName, stepperMotorDeviceConfigFile);
  _motorControlerDummy =
      boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(_motorDriverCard->getMotorControler(0));

  // Define the dummy end switch and reset the dummy
  _motorControlerDummy->setPositiveEndSwitch(POS_POSITIVE_ENDSWITCH_MOTORCONTROLLER);
  _motorControlerDummy->resetInternalStateToDefaults();

  // Create the RotaryStepperMotor object
  _stepperMotorParameters.deviceName = DUMMY_DEVICE_FILE_NAME;
  _stepperMotorParameters.moduleName = moduleName;
  _stepperMotorParameters.configFileName = stepperMotorDeviceConfigFile;
  _stepperMotorParameters.driverId = 0;
  _stepperMotor = std::make_shared<RotaryStepperMotor>(_stepperMotorParameters);
  _stepperMotor->setEnabled(true);
}

bool StepperMotorRotaryTestFixture::waitForState(std::string reqStateName, unsigned timeoutInSeconds = 10) {
  unsigned sleepPeriodInMilliSec = 10;
  unsigned timeoutInSteps = timeoutInSeconds * 1000 / sleepPeriodInMilliSec;
  unsigned cnt = 0;
  bool isCorrectState = false;
  do {
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepPeriodInMilliSec));

    if(_stepperMotor->getState() == reqStateName) {
      isCorrectState = true;
      break;
    }
    cnt++;
  } while(cnt <= timeoutInSteps);

  return isCorrectState;
}

bool StepperMotorRotaryTestFixture::waitForCalibrationFinished(unsigned timeoutInSeconds = 10) {
  auto startTime = std::chrono::steady_clock::now();
  while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - startTime).count() <
      timeoutInSeconds) {
    if(_stepperMotor->getState() == "idle") {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return _stepperMotor->getState() == "idle";
}

void StepperMotorRotaryTestFixture::waitToSetTargetPos(int targetPos) {
  while(true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if(_stepperMotor->getTargetPositionInSteps() == targetPos) {
      break;
    }
  }
}

void StepperMotorRotaryTestFixture::waitForPositiveEndSwitchActive() {
  while(!_motorControlerDummy->getReferenceSwitchData().getPositiveSwitchActive()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void StepperMotorRotaryTestFixture::driveUntilEndSwitchActive() {
  while(!_motorControlerDummy->getReferenceSwitchData().getPositiveSwitchActive()) {
    _motorControlerDummy->moveTowardsTarget(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

bool StepperMotorRotaryTestFixture::getCalibrationFailed() {
  return _stepperMotor->_calibrationFailed.load();
}

void StepperMotorRotaryTestFixture::performCalibrationForTest() {
  BOOST_CHECK(waitForState("idle"));
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("calibrating"));

  driveUntilEndSwitchActive();
  BOOST_CHECK(waitForCalibrationFinished());
}

BOOST_FIXTURE_TEST_SUITE(StepperMotorRotaryTest, StepperMotorRotaryTestFixture)

/**
 * After calibration the home switch must be the origin of the coordinate
 * system: the counter is re-based to 0 and the stored end switch references are
 * 0, independent of where the hardware counter was before the calibration.
 */
BOOST_AUTO_TEST_CASE(testCalibrateRebasesToZero) {
  // Pretend the hardware counter is somewhere in the middle of the range. The
  // homing search must find the switch within the search offset from the
  // origin, so move the dummy back onto the switch.
  _motorControlerDummy->setActualPosition(1000);
  _motorControlerDummy->setTargetPosition(1000);
  _motorControlerDummy->simulateBlockedMotor(false);

  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);

  performCalibrationForTest();

  BOOST_CHECK_EQUAL(getCalibrationFailed(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), true);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::FULL);
  BOOST_CHECK(_stepperMotor->getError() == Error::NO_ERROR);

  // The counter has been re-based to the home switch
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), 0);
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), 0);
  BOOST_CHECK_EQUAL(_stepperMotor->getNegativeEndReferenceInSteps(), 0);

  // The firmware registers hold the same (counter independent) values
  BOOST_CHECK_NE(_motorControlerDummy->getCalibrationTime(), 0U);
  BOOST_CHECK_EQUAL(_motorControlerDummy->getPositiveReferenceSwitchCalibration(), 0);
  BOOST_CHECK_EQUAL(_motorControlerDummy->getNegativeReferenceSwitchCalibration(), 0);

  // Moving away from the home switch works, and moving back towards it too.
  // After the re-base the dummy hardware counter still sits on the home switch
  // (its hardware position was not moved), so the motor can only leave the
  // switch in the negative direction, which is what a homed stage does anyway.
  BOOST_CHECK(_stepperMotor->setTargetPositionInSteps(-300) == ExitStatus::SUCCESS);
  BOOST_CHECK_NO_THROW(_stepperMotor->start());
  BOOST_CHECK(waitForState("moving", 2));
  _motorControlerDummy->moveTowardsTarget(1);
  BOOST_CHECK(waitForCalibrationFinished());
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), -300);

  BOOST_CHECK(_stepperMotor->setTargetPositionInSteps(0) == ExitStatus::SUCCESS);
  BOOST_CHECK_NO_THROW(_stepperMotor->start());
  BOOST_CHECK(waitForState("moving", 2));
  _motorControlerDummy->moveTowardsTarget(1);
  BOOST_CHECK(waitForCalibrationFinished());
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), 0);
}

/**
 * Simulate a server restart: a freshly constructed motor (without resetting the
 * dummy, which would clear the calibration time) must pick up the calibration
 * from the firmware registers.
 */
BOOST_AUTO_TEST_CASE(testCalibrationSurvivesRestart) {
  performCalibrationForTest();
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::FULL);

  // "Restart": drop the motor object and create a new one on the same dummy,
  // which still holds CAL_TIME and the end switch registers.
  _stepperMotor.reset();

  _stepperMotor = std::make_shared<RotaryStepperMotor>(_stepperMotorParameters);

  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), true);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::FULL);
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), 0);
  BOOST_CHECK_EQUAL(_stepperMotor->getNegativeEndReferenceInSteps(), 0);
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), 0);

  // A second calibration after the restart must not be required; the motor is
  // usable right away.
  _stepperMotor->setEnabled(true);
  BOOST_CHECK(waitForState("idle"));
  BOOST_CHECK(_stepperMotor->setTargetPositionInSteps(-100) == ExitStatus::SUCCESS);
  BOOST_CHECK_NO_THROW(_stepperMotor->start());
  _motorControlerDummy->moveTowardsTarget(1);
  BOOST_CHECK(waitForCalibrationFinished());
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), -100);
}

/**
 * A failed calibration must reset the counter and the end switch registers so
 * that the hardware counter does not drift and the next attempt starts from 0.
 */
BOOST_AUTO_TEST_CASE(testFailedCalibrationResetsCount) {
  // The motor cannot move at all, so the home switch is never reached.
  _motorControlerDummy->setActualPosition(4711);
  _motorControlerDummy->setTargetPosition(4711);
  _motorControlerDummy->simulateBlockedMotor(true);

  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("calibrating", 5));
  BOOST_CHECK(waitForCalibrationFinished());

  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);
  BOOST_CHECK(_stepperMotor->getError() == Error::CALIBRATION_ERROR);

  // Nothing is persisted and the counter was re-based
  BOOST_CHECK_EQUAL(_motorControlerDummy->getCalibrationTime(), 0U);
  BOOST_CHECK_EQUAL(_motorControlerDummy->getPositiveReferenceSwitchCalibration(), 0);
  BOOST_CHECK_EQUAL(_motorControlerDummy->getNegativeReferenceSwitchCalibration(), 0);
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), 0);

  // The next attempt starts from 0 again and succeeds once the motor can move.
  _motorControlerDummy->simulateBlockedMotor(false);
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("calibrating", 5));
  driveUntilEndSwitchActive();
  BOOST_CHECK(waitForCalibrationFinished());

  BOOST_CHECK_EQUAL(getCalibrationFailed(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), true);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::FULL);
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), 0);
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), 0);
  BOOST_CHECK_EQUAL(_stepperMotor->getNegativeEndReferenceInSteps(), 0);
  BOOST_CHECK_NE(_motorControlerDummy->getCalibrationTime(), 0U);
}

BOOST_AUTO_TEST_SUITE_END()

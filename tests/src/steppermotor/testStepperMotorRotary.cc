#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE StepperMotorRotaryTest

#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>

#include <chrono>
#include <memory>
#include <stdexcept>
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
// dummy end switch must be reachable with a ten moves from the origin.
static const int POS_POSITIVE_ENDSWITCH_MOTORCONTROLLER = 50000;

class StepperMotorRotaryTestFixture {
 public:
  StepperMotorRotaryTestFixture();
  void waitToSetTargetPos(int targetPos);
  void waitForPositiveEndSwitchActive();
  void awaitCalibrationSearch();
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
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);

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

void StepperMotorRotaryTestFixture::awaitCalibrationSearch() {
  BOOST_REQUIRE(waitForState("idle"));
  BOOST_REQUIRE(_stepperMotor->calibrate() == ExitStatus::SUCCESS);
  BOOST_REQUIRE(waitForState("calibrating"));
  // The worker thread has started the search once the dummy holds a search
  // target that differs from the position it started from.
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  while(std::chrono::steady_clock::now() < deadline && _stepperMotor->getTargetPositionInSteps() == 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  BOOST_REQUIRE_NE(_stepperMotor->getTargetPositionInSteps(), 0);
}

void StepperMotorRotaryTestFixture::driveUntilEndSwitchActive() {
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
  while(std::chrono::steady_clock::now() < deadline && _stepperMotor->getState() == "calibrating") {
    auto switches = _motorControlerDummy->getReferenceSwitchData();
    BOOST_CHECK(!switches.getNegativeSwitchEnabled());
    if(switches.getPositiveSwitchActive()) {
      return;
    }
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
  BOOST_REQUIRE(waitForCalibrationFinished());
  auto switches = _motorControlerDummy->getReferenceSwitchData();
  BOOST_CHECK(!switches.getPositiveSwitchEnabled());
  BOOST_CHECK(!switches.getNegativeSwitchEnabled());
}

BOOST_FIXTURE_TEST_SUITE(StepperMotorRotaryTest, StepperMotorRotaryTestFixture)

BOOST_AUTO_TEST_CASE(testRequiresDisabledHardwareStops) {
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
  BOOST_CHECK_THROW(std::make_shared<RotaryStepperMotor>(_stepperMotorParameters), ChimeraTK::logic_error);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(true);
  BOOST_CHECK_THROW(std::make_shared<RotaryStepperMotor>(_stepperMotorParameters), ChimeraTK::logic_error);
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);
  _motorControlerDummy->moveTowardsTarget(0, false, true);
  BOOST_REQUIRE(_motorControlerDummy->getReferenceSwitchData().getPositiveSwitchActive());
  BOOST_CHECK_NO_THROW(std::make_shared<RotaryStepperMotor>(_stepperMotorParameters));
}

BOOST_AUTO_TEST_CASE(testCancellationClearsTargetDespiteReferenceWriteFailure) {
  BOOST_REQUIRE(waitForState("idle"));
  BOOST_REQUIRE(_stepperMotor->calibrate() == ExitStatus::SUCCESS);
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  while(_stepperMotor->getTargetPositionInSteps() == 0 && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  BOOST_CHECK_NE(_stepperMotor->getTargetPositionInSteps(), 0);
  auto switches = _motorControlerDummy->getReferenceSwitchData();
  BOOST_CHECK(switches.getPositiveSwitchEnabled());
  BOOST_CHECK(!switches.getNegativeSwitchEnabled());
  _motorControlerDummy->failNextReferenceSwitchOperation(
      mtca4u::MotorControlerDummy::ReferenceSwitchFailure::WRITE_POSITIVE_CALIBRATION);
  _stepperMotor->stop();
  _motorControlerDummy->moveTowardsTarget(1);
  BOOST_REQUIRE(waitForCalibrationFinished());
  BOOST_CHECK_EQUAL(_stepperMotor->getTargetPositionInSteps(), 0);
  switches = _motorControlerDummy->getReferenceSwitchData();
  BOOST_CHECK(!switches.getPositiveSwitchEnabled());
  BOOST_CHECK(!switches.getNegativeSwitchEnabled());
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  BOOST_CHECK(_stepperMotor->getError() == Error::CALIBRATION_ERROR);
}

/**
 * Disabling the motor while the home search runs must stop the motion, leave
 * the motor disabled and report the aborted calibration.
 */
BOOST_AUTO_TEST_CASE(testDisableDuringCalibrationStopsAndDisables) {
  awaitCalibrationSearch();
  auto switches = _motorControlerDummy->getReferenceSwitchData();
  BOOST_REQUIRE(switches.getPositiveSwitchEnabled());
  BOOST_REQUIRE(!switches.getNegativeSwitchEnabled());

  // The dummy stops at the enabled home stop, which lies beyond the search
  // target, so a partial step leaves the motor short of its target.
  _motorControlerDummy->moveTowardsTarget(0.5);
  BOOST_REQUIRE_LT(_stepperMotor->getCurrentPositionInSteps(), _stepperMotor->getTargetPositionInSteps());

  _stepperMotor->setEnabled(false);

  BOOST_CHECK(waitForState("disabled"));
  BOOST_CHECK_EQUAL(_stepperMotor->getEnabled(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), _stepperMotor->getTargetPositionInSteps());
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  BOOST_CHECK(_stepperMotor->getError() == Error::CALIBRATION_ERROR);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);
  BOOST_CHECK_EQUAL(_motorControlerDummy->getCalibrationTime(), 0U);
}

/**
 * Stopping the motor while the home search runs must end the calibration and
 * leave the motor standing, without commanding another search chunk.
 */
BOOST_AUTO_TEST_CASE(testStopDuringCalibrationStopsTheSearch) {
  awaitCalibrationSearch();

  // Advance part of the running chunk, so the stop lands in a moving search.
  _motorControlerDummy->moveTowardsTarget(0.5);
  BOOST_REQUIRE_LT(_stepperMotor->getCurrentPositionInSteps(), _stepperMotor->getTargetPositionInSteps());

  _stepperMotor->stop();

  // Finish the chunk the worker is waiting in, so it reaches its abort check.
  _motorControlerDummy->moveTowardsTarget(1);
  BOOST_REQUIRE(waitForCalibrationFinished());

  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  BOOST_CHECK(_stepperMotor->getError() == Error::CALIBRATION_ERROR);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);
  BOOST_CHECK_EQUAL(_stepperMotor->getEnabled(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->getTargetPositionInSteps(), _stepperMotor->getCurrentPositionInSteps());
  BOOST_CHECK_EQUAL(_motorControlerDummy->getCalibrationTime(), 0U);

  // No further search chunk: the target stays where the abort left it, even
  // beyond one wakeup period of the search loop.
  auto const targetAfterAbort = _stepperMotor->getTargetPositionInSteps();
  std::this_thread::sleep_for(std::chrono::milliseconds(1200));
  BOOST_CHECK_EQUAL(_stepperMotor->getTargetPositionInSteps(), targetAfterAbort);
  BOOST_CHECK_EQUAL(_stepperMotor->getState(), "idle");
}

/**
 * A rotary move verification must not accept a move which stopped away from its
 * target just because a reference switch is active.
 */
BOOST_AUTO_TEST_CASE(testPositiveSwitchDoesNotVerifyMove) {
  performCalibrationForTest();

  // The dummy counter still sits on the positive switch, which reports active
  // once enabled. The dummy then refuses to move in its direction.
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
  BOOST_REQUIRE(_motorControlerDummy->getReferenceSwitchData().getPositiveSwitchActive());

  BOOST_CHECK(
      _stepperMotor->setTargetPositionInSteps(_stepperMotor->getCurrentPositionInSteps() + 100) == ExitStatus::SUCCESS);
  BOOST_CHECK_NO_THROW(_stepperMotor->start());

  // Poll for the outcome: the move is only verified after the worker thread
  // has seen the standstill, and the error state hides the switch status.
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  while(std::chrono::steady_clock::now() < deadline && _stepperMotor->getState() == "moving") {
    _motorControlerDummy->moveTowardsTarget(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  BOOST_CHECK_EQUAL(_stepperMotor->getState(), "error");
  BOOST_CHECK(_stepperMotor->getError() == Error::MOVE_INTERRUPTED);
}

BOOST_AUTO_TEST_CASE(testSearchExceptionDisablesHomeStop) {
  using Failure = mtca4u::MotorControlerDummy::ReferenceSwitchFailure;
  BOOST_REQUIRE(waitForState("idle"));
  _motorControlerDummy->failNextReferenceSwitchOperation(Failure::READ_WHILE_ENABLED);
  BOOST_REQUIRE(_stepperMotor->calibrate() == ExitStatus::SUCCESS);
  BOOST_REQUIRE(waitForCalibrationFinished());
  auto switches = _motorControlerDummy->getReferenceSwitchData();
  BOOST_CHECK(!switches.getPositiveSwitchEnabled());
  BOOST_CHECK(!switches.getNegativeSwitchEnabled());
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);
  BOOST_CHECK(_stepperMotor->getError() == Error::CALIBRATION_ERROR);
}

BOOST_AUTO_TEST_CASE(testHomeStopDisableFailureInvalidatesCalibration) {
  using Failure = mtca4u::MotorControlerDummy::ReferenceSwitchFailure;
  BOOST_REQUIRE(waitForState("idle"));
  _motorControlerDummy->moveTowardsTarget(0, false, true);
  _motorControlerDummy->failNextReferenceSwitchOperation(Failure::DISABLE_POSITIVE);
  BOOST_REQUIRE(_stepperMotor->calibrate() == ExitStatus::SUCCESS);
  BOOST_REQUIRE(waitForCalibrationFinished());
  BOOST_CHECK(_motorControlerDummy->getReferenceSwitchData().getPositiveSwitchEnabled());
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);
  BOOST_CHECK(_stepperMotor->getError() == Error::CALIBRATION_ERROR);
  BOOST_CHECK_EQUAL(_motorControlerDummy->getCalibrationTime(), 0U);
}

BOOST_AUTO_TEST_CASE(testUnconfirmedTargetResetKeepsHomeStopEnabled) {
  using Failure = mtca4u::MotorControlerDummy::ReferenceSwitchFailure;
  BOOST_REQUIRE(waitForState("idle"));
  _motorControlerDummy->moveTowardsTarget(0, false, true);
  // Fail verification after both the initial reset and the cleanup retry.
  _motorControlerDummy->failNextReferenceSwitchOperation(Failure::READ_HOMING_TARGET_TWICE);
  BOOST_REQUIRE(_stepperMotor->calibrate() == ExitStatus::SUCCESS);
  BOOST_REQUIRE(waitForCalibrationFinished());
  auto switches = _motorControlerDummy->getReferenceSwitchData();
  BOOST_CHECK(switches.getPositiveSwitchEnabled());
  BOOST_CHECK(!switches.getNegativeSwitchEnabled());
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);
  BOOST_CHECK(_stepperMotor->getError() == Error::CALIBRATION_ERROR);
  BOOST_CHECK_EQUAL(_motorControlerDummy->getCalibrationTime(), 0U);
}

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

/**
 * After an emergency stop the motor is in the SM "error" state and every
 * move request must be rejected with ERR_SYSTEM_IN_ERROR_STATE until the
 * error is reset.
 */
BOOST_AUTO_TEST_CASE(testEmergencyStopBlocksMoves) {
  performCalibrationForTest();
  BOOST_CHECK(_stepperMotor->setTargetPositionInSteps(-300) == ExitStatus::SUCCESS);
  BOOST_CHECK_NO_THROW(_stepperMotor->start());
  BOOST_CHECK(waitForState("moving", 2));
  _motorControlerDummy->moveTowardsTarget(0.8);
  _stepperMotor->emergencyStop();

  BOOST_CHECK(waitForState("error"));
  BOOST_CHECK_EQUAL(_stepperMotor->getEnabled(), false);

  // While in the error state, moves must be rejected with ERR_SYSTEM_IN_ERROR_STATE
  BOOST_CHECK(_stepperMotor->setTargetPositionInSteps(10) == ExitStatus::ERR_SYSTEM_IN_ERROR_STATE);
  BOOST_CHECK(_stepperMotor->moveRelativeInSteps(10) == ExitStatus::ERR_SYSTEM_IN_ERROR_STATE);

  _stepperMotor->resetError();
  BOOST_CHECK(waitForState("disabled"));
}

BOOST_AUTO_TEST_SUITE_END()

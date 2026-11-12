// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE LinearStepperMotorHomingTest

#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <thread>

using namespace boost::unit_test_framework;

#include "LinearStepperMotor.h"
#include "LinearStepperMotorStateMachine.h"
#include "MotorControlerDummy.h"
#include "MotorDriverCard.h"
#include "MotorDriverCardFactory.h"
#include "StepperMotor.h"
#include "testConfigConstants.h"

using namespace ChimeraTK::MotorDriver;

static const std::string stepperMotorDeviceConfigFile("VT21-MotorDriverCardConfig.xml");
static const std::string moduleName("");

// The dummy resolves its switches from the hardware position, which equals the motor controller position for a fresh
// dummy, and stops the motor at an active switch.
const int POSITIVE_END_SWITCH_POSITION = 10000;
const int NEGATIVE_END_SWITCH_POSITION = -10000;

// The offset of the production searches.
const int END_SWITCH_SEARCH_OFFSET = 50000;

const unsigned WAIT_PERIOD_MS = 5U;
const std::chrono::milliseconds WAIT_TIMEOUT(10000);

// The searches wait for their chunk, so the dummy has to be stepped well within that wait.
const std::chrono::milliseconds CHUNK_PERIOD(100);

// A failing search needs the chunk which runs into the failure and the one after it, which gives up. This bound only
// guards against a runaway.
const unsigned MAX_SEARCH_CHUNKS = 4U;

/// Polls a condition until it holds or the timeout expires.
template<typename Condition>
static bool waitFor(const Condition& condition) {
  auto const start = std::chrono::steady_clock::now();
  while(std::chrono::steady_clock::now() - start < WAIT_TIMEOUT) {
    if(condition()) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_PERIOD_MS));
  }
  return condition();
}

namespace {

  /**
   * Linear state machine whose searches end on the homing timeout within a test.
   *
   * A search which keeps issuing chunks waits for the homing timeout of the production state machine between them,
   * which is longer than a test may take.
   */
  class ShortTimeoutLinearStepperMotorStateMachine : public LinearStepperMotorStateMachine {
   public:
    explicit ShortTimeoutLinearStepperMotorStateMachine(LinearStepperMotor& motor)
    : LinearStepperMotorStateMachine(motor) {}

   protected:
    int getHomingTimeoutInMilliseconds() const override { return HOMING_TIMEOUT_MS; }
  };

} // namespace

/// Linear motor on a dummy motor controller which is hand-stepped through its chunks by the tests.
class LinearStepperMotorHomingTestFixture {
 public:
  void startCalibration();
  /// Drives the dummy to the given motor controller position.
  void driveTo(int motorControllerPosition);
  /// Drives the dummy onto the positive end switch.
  void driveToPositiveEndSwitch();
  /// Lets the background search advance by one chunk.
  void advanceOneChunk() const;
  /// Steps the searches until the calibration has left the calibrating state.
  void runCalibrationUntilAbort();

 protected:
  LinearStepperMotorHomingTestFixture();
  ~LinearStepperMotorHomingTestFixture();

  /// Replaces the state machine of the motor before it is used for the first time.
  void useStateMachine(std::shared_ptr<LinearStepperMotorStateMachine> stateMachine);

  StepperMotorParameters _stepperMotorParameters;
  std::shared_ptr<LinearStepperMotor> _stepperMotor;
  std::shared_ptr<LinearStepperMotorStateMachine> _stateMachine;
  boost::shared_ptr<mtca4u::MotorDriverCard> _motorDriverCard;
  boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;
};

LinearStepperMotorHomingTestFixture::LinearStepperMotorHomingTestFixture() {
  mtca4u::MotorDriverCardFactory::instance().setDummyMode();
  _motorDriverCard = mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(
      DUMMY_DEVICE_FILE_NAME, moduleName, stepperMotorDeviceConfigFile);
  _motorControlerDummy =
      boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(_motorDriverCard->getMotorControler(0));

  _motorControlerDummy->setPositiveEndSwitch(POSITIVE_END_SWITCH_POSITION);
  _motorControlerDummy->setNegativeEndSwitch(NEGATIVE_END_SWITCH_POSITION);
  _motorControlerDummy->resetInternalStateToDefaults();

  _stepperMotorParameters.motorType = StepperMotorType::LINEAR;
  _stepperMotorParameters.deviceName = DUMMY_DEVICE_FILE_NAME;
  _stepperMotorParameters.moduleName = moduleName;
  _stepperMotorParameters.configFileName = stepperMotorDeviceConfigFile;
  _stepperMotor = std::make_shared<LinearStepperMotor>(_stepperMotorParameters);
  _stateMachine = std::make_shared<LinearStepperMotorStateMachine>(*_stepperMotor);
  _stepperMotor->_stateMachine = _stateMachine;
  _stepperMotor->initStateMachine();
}

LinearStepperMotorHomingTestFixture::~LinearStepperMotorHomingTestFixture() {
  // A calibration which is still running would keep stepping a motor which is about to be destroyed. A blocked motor
  // never finishes its search, so the stop has to abort it. The motor may then be in the error state, from which only
  // a reset leads back to a settleable state.
  waitFor([this] {
    _stepperMotor->stop();
    _stepperMotor->resetError();
    return _stepperMotor->isSystemIdle();
  });
  _stepperMotor->setEnabled(false);
  _stateMachine.reset();
  _stepperMotor.reset();
  _motorControlerDummy->resetInternalStateToDefaults();
}

void LinearStepperMotorHomingTestFixture::useStateMachine(
    std::shared_ptr<LinearStepperMotorStateMachine> stateMachine) {
  _stateMachine = std::move(stateMachine);
  _stepperMotor->_stateMachine = _stateMachine;
  _stepperMotor->initStateMachine();
}

void LinearStepperMotorHomingTestFixture::startCalibration() {
  _stepperMotor->setEnabled(true);
  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "idle"; }));
  BOOST_TEST(static_cast<int>(_stepperMotor->calibrate()) == static_cast<int>(ExitStatus::SUCCESS));
  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "calibrating"; }));
}

void LinearStepperMotorHomingTestFixture::driveTo(int motorControllerPosition) {
  // The dummy only steps while the motor current is on and lets the motor stop at an end switch.
  _stepperMotor->setEnabled(true);
  _motorControlerDummy->setTargetPosition(motorControllerPosition);
  while(_motorControlerDummy->isMotorMoving()) {
    _motorControlerDummy->moveTowardsTarget(1.0F, false);
  }
}

void LinearStepperMotorHomingTestFixture::driveToPositiveEndSwitch() {
  driveTo(POSITIVE_END_SWITCH_POSITION);
  BOOST_TEST_REQUIRE(_stepperMotor->isPositiveReferenceActive());
}

void LinearStepperMotorHomingTestFixture::advanceOneChunk() const {
  // The searches wait for their move to end, so the dummy has to finish the chunk before the search looks at it again.
  _motorControlerDummy->moveTowardsTarget(1.0F, false);
  std::this_thread::sleep_for(CHUNK_PERIOD);
}

void LinearStepperMotorHomingTestFixture::runCalibrationUntilAbort() {
  for(unsigned chunk = 0; chunk < MAX_SEARCH_CHUNKS; ++chunk) {
    advanceOneChunk();
    if(_stepperMotor->getState() != "calibrating") {
      return;
    }
  }
  BOOST_TEST_REQUIRE(_stepperMotor->getState() != "calibrating",
      "the calibration did not abort within " << MAX_SEARCH_CHUNKS << " chunks");
}

BOOST_FIXTURE_TEST_SUITE(LinearStepperMotorHomingTest, LinearStepperMotorHomingTestFixture)

/**
 * Requirements, releasing an active switch.
 *
 * "Calibration must move the motor off an already-active positive reference switch before homing."
 *
 * The motor starts on the active switch, so the calibration leaves the switch position by moving negatively instead
 * of homing from it.
 */
BOOST_AUTO_TEST_CASE(HomingFromActiveSwitchMovesOffFirst) {
  driveToPositiveEndSwitch();
  startCalibration();

  // The release search moves negatively, away from the positive end switch.
  BOOST_TEST_REQUIRE(waitFor([this] {
    return _stepperMotor->getTargetPositionInSteps() == POSITIVE_END_SWITCH_POSITION - END_SWITCH_SEARCH_OFFSET;
  }));

  BOOST_TEST(_stepperMotor->getState() == "calibrating");
  BOOST_TEST(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::NONE));
}

/**
 * Requirements, releasing an active switch.
 *
 * "Calibration must fail with a calibration error and leave the calibration mode at CalibrationMode::NONE if the
 * switch does not release."
 *
 * The blocked motor stays on the switch, so the release search finds the motor at a standstill with the switch still
 * active after its chunk and aborts.
 */
BOOST_AUTO_TEST_CASE(BlockedMotorOnActiveSwitchFailsWithCalibrationError) {
  driveToPositiveEndSwitch();
  startCalibration();

  BOOST_TEST_REQUIRE(waitFor([this] {
    return _stepperMotor->getTargetPositionInSteps() == POSITIVE_END_SWITCH_POSITION - END_SWITCH_SEARCH_OFFSET;
  }));

  _motorControlerDummy->simulateBlockedMotor(true);

  runCalibrationUntilAbort();

  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "idle"; }));
  BOOST_TEST(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::NONE));
  BOOST_TEST(static_cast<int>(_stepperMotor->getError()) == static_cast<int>(Error::CALIBRATION_ERROR));
  BOOST_TEST(_stepperMotor->isCalibrated() == false);
}

/**
 * Requirements, releasing an active switch.
 *
 * "Calibration must fail with a calibration error and leave the calibration mode at CalibrationMode::NONE if the
 * switch does not release."
 *
 * The motor follows its chunk into a span which no switch covers, but the positive switch keeps reporting an active
 * state, so the release has not happened. The switches are placed at the ends of the travel, so each further chunk runs
 * over the positive switch and back, which is what a broken switch looks like.
 */
BOOST_AUTO_TEST_CASE(ReleaseSearchNotReleasingTheSwitchFailsWithCalibrationError) {
  useStateMachine(std::make_shared<ShortTimeoutLinearStepperMotorStateMachine>(*_stepperMotor));

  _motorControlerDummy->setNegativeEndSwitch(POSITIVE_END_SWITCH_POSITION);
  _motorControlerDummy->setPositiveEndSwitch(NEGATIVE_END_SWITCH_POSITION);

  startCalibration();

  BOOST_TEST_REQUIRE(
      waitFor([this] { return _stepperMotor->getTargetPositionInSteps() == -END_SWITCH_SEARCH_OFFSET; }));
  BOOST_TEST(_stepperMotor->isPositiveReferenceActive() == true);

  runCalibrationUntilAbort();

  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "idle"; }));
  BOOST_TEST(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::NONE));
  BOOST_TEST(static_cast<int>(_stepperMotor->getError()) == static_cast<int>(Error::CALIBRATION_ERROR));
  BOOST_TEST(_stepperMotor->isCalibrated() == false);
}

/**
 * Requirements, bounded search.
 *
 * "A search that does not find its switch within the homing timeout of the stage must abort the calibration."
 * "The abort reports a calibration error and leaves the calibration mode at CalibrationMode::NONE."
 *
 * The switches sit outside the travel of the stage, so the dummy stops every chunk at the negative switch and the
 * positive switch stays inactive. The calibration therefore starts with its homing search, which keeps moving without
 * reaching its switch.
 */
BOOST_AUTO_TEST_CASE(SearchNotReachingTheSwitchTimesOut) {
  useStateMachine(std::make_shared<ShortTimeoutLinearStepperMotorStateMachine>(*_stepperMotor));

  _motorControlerDummy->setNegativeEndSwitch(POSITIVE_END_SWITCH_POSITION + END_SWITCH_SEARCH_OFFSET);
  _motorControlerDummy->setPositiveEndSwitch(NEGATIVE_END_SWITCH_POSITION - END_SWITCH_SEARCH_OFFSET);

  startCalibration();

  BOOST_TEST_REQUIRE(waitFor([this] { return _stepperMotor->getTargetPositionInSteps() == END_SWITCH_SEARCH_OFFSET; }));
  BOOST_TEST(_stepperMotor->isPositiveReferenceActive() == false);

  runCalibrationUntilAbort();

  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "idle"; }));
  BOOST_TEST(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::NONE));
  BOOST_TEST(static_cast<int>(_stepperMotor->getError()) == static_cast<int>(Error::CALIBRATION_ERROR));
  BOOST_TEST(_stepperMotor->isCalibrated() == false);
}

/**
 * Requirements, bounded search.
 *
 * "The abort reports a calibration error and leaves the calibration mode at CalibrationMode::NONE."
 *
 * The motor starts on the active negative switch, which makes the dummy stop every chunk at once. The homing search
 * therefore finds neither switch by moving, which leaves the span uncalibrated.
 */
BOOST_AUTO_TEST_CASE(TimedOutSearchKeepsTheSpan) {
  _motorControlerDummy->setNegativeEndSwitch(NEGATIVE_END_SWITCH_POSITION);
  _motorControlerDummy->setPositiveEndSwitch(POSITIVE_END_SWITCH_POSITION + END_SWITCH_SEARCH_OFFSET);
  _motorControlerDummy->moveTowardsTarget(0.0F, false);

  startCalibration();

  BOOST_TEST_REQUIRE(waitFor([this] { return _stepperMotor->getTargetPositionInSteps() == END_SWITCH_SEARCH_OFFSET; }));

  runCalibrationUntilAbort();

  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "idle"; }));
  BOOST_TEST(_stepperMotor->isCalibrated() == false);
}

BOOST_AUTO_TEST_SUITE_END()

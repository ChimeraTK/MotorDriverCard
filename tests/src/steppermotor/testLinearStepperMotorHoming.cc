// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE LinearStepperMotorHomingTest

#include "LinearStepperMotor.h"
#include "LinearStepperMotorStateMachine.h"
#include "MotorControlerDummy.h"
#include "MotorDriverCard.h"
#include "MotorDriverCardFactory.h"
#include "StepperMotor.h"
#include "testConfigConstants.h"

#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <thread>

using namespace boost::unit_test_framework;
using namespace ChimeraTK::MotorDriver;

static const std::string stepperMotorDeviceConfigFile("VT21-MotorDriverCardConfig.xml");
static const std::string moduleName;

// The offset of the production searches, which is also the chunk of every step in the tests.
const int SEARCH_OFFSET = 50000;

// The placement of the stage. The dummy reports a switch as active once the hardware position reaches it and stops
// every move at the first switch in the direction of the move. The hardware position runs from the negative switch to
// the positive switch, one motor controller step each.
const int NEGATIVE_SWITCH_POSITION = -SEARCH_OFFSET;
const int POSITIVE_SWITCH_POSITION = 4 * SEARCH_OFFSET;
// A switch position behind both ends of the travel, so the switch stays reachable but never releases the search.
const int UNREACHABLE_SWITCH_POSITION = 8 * SEARCH_OFFSET;

// The tests poll the running calibration, as the searches wait for their chunk to end.
const unsigned POLL_PERIOD_MS = 1U;
const std::chrono::milliseconds WAIT_TIMEOUT(10000);

namespace {

  /**
   * Linear state machine whose searches end on the homing timeout within a test.
   *
   * The production timeout is longer than a test may take. It outlasts a single search chunk, which the test finishes
   * first, and expires before the next chunk is issued.
   */
  class ShortTimeoutLinearStepperMotorStateMachine : public LinearStepperMotorStateMachine {
   public:
    static constexpr int SHORT_HOMING_TIMEOUT_MS = 100;

    explicit ShortTimeoutLinearStepperMotorStateMachine(LinearStepperMotor& motor)
    : LinearStepperMotorStateMachine(motor) {}

    [[nodiscard]] int getHomingTimeoutInMilliseconds() const override { return SHORT_HOMING_TIMEOUT_MS; }
  };

  /// Linear state machine which only exposes the production homing timeout.
  class ReportingLinearStepperMotorStateMachine : public LinearStepperMotorStateMachine {
   public:
    explicit ReportingLinearStepperMotorStateMachine(LinearStepperMotor& motor)
    : LinearStepperMotorStateMachine(motor) {}

    using LinearStepperMotorStateMachine::getHomingTimeoutInMilliseconds;
  };

  /**
   * Linear motor whose positive switch is readable on its own.
   *
   * isEndSwitchActive() raises an error when both switches are active, so it cannot be used where the tests need the
   * positive switch alone.
   */
  class LinearStepperMotorWithRawSwitch : public LinearStepperMotor {
   public:
    explicit LinearStepperMotorWithRawSwitch(const StepperMotorParameters& parameters)
    : LinearStepperMotor(parameters) {}

    bool isRawPositiveSwitchActive() const { return positiveSwitchActive(); }
  };

} // namespace

/// Linear motor on a dummy motor controller which the tests place and hand-step.
class LinearStepperMotorHomingTestFixture {
 public:
  void startCalibration();
  /// Places the motor on the given switch, so that only a move away from it releases the switch.
  void placeMotorOnPositiveSwitch();
  /// Runs the calibration and steps the dummy once per search chunk until the calibration has finished.
  void runCalibration();

 protected:
  LinearStepperMotorHomingTestFixture();
  ~LinearStepperMotorHomingTestFixture();

  /// Replaces the state machine of the motor before it is used for the first time.
  void useStateMachine(std::shared_ptr<LinearStepperMotorStateMachine> stateMachine);

  /// Polls a condition until it holds or the timeout expires.
  template<typename Condition>
  bool waitFor(Condition condition);

  /// Waits until the search has issued the chunk with the given target position.
  void waitForChunk(int targetPositionInSteps);
  /// Moves the dummy to the target position of the last issued chunk.
  void stepChunk();

  StepperMotorParameters _stepperMotorParameters;
  std::shared_ptr<LinearStepperMotorWithRawSwitch> _stepperMotor;
  std::shared_ptr<LinearStepperMotorStateMachine> _stateMachine;
  boost::shared_ptr<mtca4u::MotorDriverCard> _motorDriverCard;
  boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;
  /// Whether the test has blocked the motor. The dummy forgets the block on each move, so it has to be preserved.
  bool _motorBlocked = false;
};

LinearStepperMotorHomingTestFixture::LinearStepperMotorHomingTestFixture() {
  mtca4u::MotorDriverCardFactory::instance().setDummyMode();
  _motorDriverCard = mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(
      DUMMY_DEVICE_FILE_NAME, moduleName, stepperMotorDeviceConfigFile);
  _motorControlerDummy =
      boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(_motorDriverCard->getMotorControler(0));

  _motorControlerDummy->setNegativeEndSwitch(NEGATIVE_SWITCH_POSITION);
  _motorControlerDummy->setPositiveEndSwitch(POSITIVE_SWITCH_POSITION);
  _motorControlerDummy->resetInternalStateToDefaults();
  _motorBlocked = false;

  _stepperMotorParameters.motorType = StepperMotorType::LINEAR;
  _stepperMotorParameters.deviceName = DUMMY_DEVICE_FILE_NAME;
  _stepperMotorParameters.moduleName = moduleName;
  _stepperMotorParameters.configFileName = stepperMotorDeviceConfigFile;
  _stepperMotor = std::make_shared<LinearStepperMotorWithRawSwitch>(_stepperMotorParameters);
  _stateMachine = std::make_shared<LinearStepperMotorStateMachine>(*_stepperMotor);
  _stepperMotor->_stateMachine = _stateMachine;
  _stepperMotor->initStateMachine();
}

LinearStepperMotorHomingTestFixture::~LinearStepperMotorHomingTestFixture() {
  // A calibration which is still running would keep stepping a motor which is about to be destroyed. The searches wait
  // for a move to end, so the motor must not stay blocked for the stop to take effect.
  _motorBlocked = false;
  _motorControlerDummy->simulateBlockedMotor(false);
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

template<typename Condition>
bool LinearStepperMotorHomingTestFixture::waitFor(Condition condition) {
  auto const start = std::chrono::steady_clock::now();
  while(std::chrono::steady_clock::now() - start < WAIT_TIMEOUT) {
    if(condition()) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(POLL_PERIOD_MS));
  }
  return condition();
}

void LinearStepperMotorHomingTestFixture::startCalibration() {
  _stepperMotor->setEnabled(true);
  BOOST_TEST_REQUIRE(waitFor([this] { return _stepperMotor->getState() == "idle"; }));
  BOOST_TEST_REQUIRE(static_cast<int>(_stepperMotor->calibrate()) == static_cast<int>(ExitStatus::SUCCESS));
  BOOST_TEST_REQUIRE(waitFor([this] { return _stepperMotor->getState() == "calibrating"; }));
}

void LinearStepperMotorHomingTestFixture::placeMotorOnPositiveSwitch() {
  // A move the dummy cannot carry out leaves the positions untouched, so the motor is driven onto the positive switch
  // step by step to keep the hardware position in line with the motor controller position.
  _stepperMotor->setEnabled(true);
  while(_stepperMotor->getCurrentPositionInSteps() < POSITIVE_SWITCH_POSITION) {
    _motorControlerDummy->setTargetPosition(_stepperMotor->getCurrentPositionInSteps() + SEARCH_OFFSET);
    while(_motorControlerDummy->isMotorMoving()) {
      _motorControlerDummy->moveTowardsTarget(1.0F, false);
    }
  }
  BOOST_TEST_REQUIRE(waitFor([this] { return _stepperMotor->isSystemIdle(); }));
  BOOST_TEST_REQUIRE(_stepperMotor->getCurrentPositionInSteps() == POSITIVE_SWITCH_POSITION);
  BOOST_TEST_REQUIRE(_stepperMotor->isRawPositiveSwitchActive());
}

void LinearStepperMotorHomingTestFixture::waitForChunk(int targetPositionInSteps) {
  BOOST_TEST_REQUIRE(waitFor([this, targetPositionInSteps] {
    return _stepperMotor->getTargetPositionInSteps() == targetPositionInSteps ||
        _stepperMotor->getState() != "calibrating";
  }));
}

void LinearStepperMotorHomingTestFixture::stepChunk() {
  _motorControlerDummy->moveTowardsTarget(1.0F, _motorBlocked);
}

void LinearStepperMotorHomingTestFixture::runCalibration() {
  // The searches issue one chunk at a time and wait for the dummy to finish it. The dummy only reports a move as
  // pending after it has been stepped, so every pending move is finished here and a running calibration is never left
  // waiting. The calibration ends on its own once its searches have found their switches, or on a failure of the
  // searches.
  BOOST_TEST_REQUIRE(waitFor([this] {
    if(_stepperMotor->getState() != "calibrating") {
      return true;
    }
    if(_stepperMotor->getTargetPositionInSteps() != _stepperMotor->getCurrentPositionInSteps()) {
      stepChunk();
    }
    return _stepperMotor->getState() != "calibrating";
  }),
      "the calibration did not finish");
}

BOOST_FIXTURE_TEST_SUITE(LinearStepperMotorHomingTest, LinearStepperMotorHomingTestFixture)

/**
 * Requirements, releasing an active switch.
 *
 * "Calibration must move the motor off an already-active positive reference switch before homing."
 *
 * The motor starts on the active switch, which only a positive move reaches again, so the release search moves
 * negatively and its chunk ends off the switch.
 */
BOOST_AUTO_TEST_CASE(HomingFromActiveSwitchMovesOffFirst) {
  placeMotorOnPositiveSwitch();

  startCalibration();
  waitForChunk(POSITIVE_SWITCH_POSITION - SEARCH_OFFSET);
  stepChunk();

  BOOST_TEST_REQUIRE(waitFor([this] { return _stepperMotor->isRawPositiveSwitchActive() == false; }));
}

/**
 * Requirements, releasing an active switch.
 *
 * "The positive reference must be the position where the switch activates when approached positively, independent of
 * the start position."
 *
 * The motor starts on the active switch. The release search moves it one chunk away from the switch, and the positive
 * search moves it back by one chunk. The dummy does not stop a move whose target lies exactly on the switch, so the
 * positive reference is the position where the search ends, one chunk past the switch. The negative search then finds
 * the negative switch, which becomes the zero of the span.
 */
BOOST_AUTO_TEST_CASE(PositiveReferenceIsTheApproachedPosition) {
  placeMotorOnPositiveSwitch();

  startCalibration();
  runCalibration();

  BOOST_TEST(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::FULL));
  BOOST_TEST(_stepperMotor->getPositiveEndReferenceInSteps() == POSITIVE_SWITCH_POSITION + SEARCH_OFFSET);
  BOOST_TEST(_stepperMotor->getNegativeEndReferenceInSteps() == 0);
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
  placeMotorOnPositiveSwitch();
  _motorBlocked = true;
  _motorControlerDummy->simulateBlockedMotor(true);

  startCalibration();
  runCalibration();

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
 * The motor starts on a switch which never releases, as its position is behind both ends of the travel. The release
 * search therefore ends on the homing timeout. The motor only gets there without being blocked by the switch, so the
 * motor controller position is aligned with the switch position by hand.
 */
BOOST_AUTO_TEST_CASE(MovingMotorWhichDoesNotReleaseTheSwitchFailsWithCalibrationError) {
  useStateMachine(std::make_shared<ShortTimeoutLinearStepperMotorStateMachine>(*_stepperMotor));
  _motorControlerDummy->setPositiveEndSwitch(UNREACHABLE_SWITCH_POSITION);
  _stepperMotor->setEnabled(true);
  _motorControlerDummy->setTargetPosition(UNREACHABLE_SWITCH_POSITION);
  BOOST_TEST_REQUIRE(waitFor([this] {
    _motorControlerDummy->moveTowardsTarget(1.0F, false);
    return _stepperMotor->isRawPositiveSwitchActive();
  }),
      "the motor did not reach the switch");
  BOOST_TEST_REQUIRE(_stepperMotor->isRawPositiveSwitchActive());

  startCalibration();
  runCalibration();

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
 * Both switches sit outside the travel of the stage, so no search can find its switch.
 */
BOOST_AUTO_TEST_CASE(HomingSearchWhichDoesNotFindItsSwitchAborts) {
  useStateMachine(std::make_shared<ShortTimeoutLinearStepperMotorStateMachine>(*_stepperMotor));
  _motorControlerDummy->setPositiveEndSwitch(UNREACHABLE_SWITCH_POSITION);
  _motorControlerDummy->setNegativeEndSwitch(-UNREACHABLE_SWITCH_POSITION);

  startCalibration();
  runCalibration();

  BOOST_TEST(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::NONE));
  BOOST_TEST(static_cast<int>(_stepperMotor->getError()) == static_cast<int>(Error::CALIBRATION_ERROR));
  BOOST_TEST(_stepperMotor->isCalibrated() == false);
}

/**
 * Specifications, LinearStepperMotorStateMachine.
 *
 * "getHomingTimeoutInMilliseconds() is overridden in the class declaration and returns 600000 ms (600 s)."
 */
BOOST_AUTO_TEST_CASE(HomingTimeoutOfTheLinearStateMachine) {
  ReportingLinearStepperMotorStateMachine stateMachine(*_stepperMotor);
  BOOST_TEST(stateMachine.getHomingTimeoutInMilliseconds() == 600000);
}

BOOST_AUTO_TEST_SUITE_END()

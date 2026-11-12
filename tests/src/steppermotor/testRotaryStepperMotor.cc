// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RotaryStepperMotorTest

#include "MotorControlerDummy.h"
#include "MotorDriverCard.h"
#include "MotorDriverCardFactory.h"
#include "RotaryStepperMotor.h"
#include "StepperMotor.h"
#include "testConfigConstants.h"

#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>

#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <thread>

using namespace boost::unit_test_framework;
using namespace ChimeraTK::MotorDriver;

static const std::string stepperMotorDeviceConfigFile("VT21-MotorDriverCardConfig.xml");
static const std::string moduleName;

// The positive end switch of the dummy sits at this hardware position. The dummy resolves the switch from the
// hardware position only, and the hardware position equals the motor controller position for a fresh dummy. The
// end switch is therefore reached exactly when the motor controller reports this position.
const int POSITIVE_END_SWITCH_POSITION = 10000;

// Both the release search and the homing search move in chunks of END_SWITCH_SEARCH_OFFSET (5000) steps relative to
// the current motor controller position.
const int END_SWITCH_SEARCH_OFFSET = 5000;

const unsigned WAIT_PERIOD_MS = 5U;
const std::chrono::milliseconds WAIT_TIMEOUT(10000);

// The calibration runs in a detached thread which advances the dummy one chunk at a time. The chunk is only picked up
// while the motor stands still, so the chunks have to be spaced out.
const std::chrono::milliseconds CHUNK_PERIOD(150);

// The tests advance the dummy in single steps of this size to hold a chunk open while they stop the motor.
const int CHUNK_STEP = 5;

// A full rotary calibration needs a release chunk and a homing chunk, this bound only guards against a runaway.
const unsigned MAX_CALIBRATION_CHUNKS = 10U;

// Ten samples of two moves each. The worker polls for each move to finish every few milliseconds, so this bound is
// generous and only guards against a runaway.
const unsigned MAX_TOLERANCE_ITERATIONS = 2000U;

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

/// Rotary motor on a dummy motor controller which is hand-stepped through its chunks by the tests.
class RotaryStepperMotorFixture {
 public:
  void startCalibration();
  /// Drives the dummy to the given motor controller position, using the motor current of the motor.
  void driveTo(int motorControllerPosition);
  void driveToEndSwitch();
  /// Lets the background thread advance by one chunk.
  void advanceOneChunk() const;
  /// Runs the tolerance calculation until the motor is idle.
  void runToleranceCalculationToEnd();
  /// Runs the calibration until the motor is idle.
  void runCalibrationToEnd();
  /// Stops the motor while the chunk which starts at the given position runs.
  void stopDuringChunkAt(int chunkStartPosition);

 protected:
  RotaryStepperMotorFixture();
  ~RotaryStepperMotorFixture();

  /// The thread which stops the motor, see stopDuringChunkAt(). It runs alongside the calibration thread, so the
  /// object holding the threads is a member and not a temporary.
  std::thread _stopThread;

  StepperMotorParameters _stepperMotorParameters;
  std::shared_ptr<StepperMotor> _stepperMotor;
  std::shared_ptr<RotaryStepperMotor> _rotaryMotor;
  boost::shared_ptr<mtca4u::MotorDriverCard> _motorDriverCard;
  boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;
};

RotaryStepperMotorFixture::RotaryStepperMotorFixture() {
  mtca4u::MotorDriverCardFactory::instance().setDummyMode();
  _motorDriverCard = mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(
      DUMMY_DEVICE_FILE_NAME, moduleName, stepperMotorDeviceConfigFile);
  _motorControlerDummy =
      boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(_motorDriverCard->getMotorControler(0));

  _motorControlerDummy->setPositiveEndSwitch(POSITIVE_END_SWITCH_POSITION);
  _motorControlerDummy->resetInternalStateToDefaults();

  _stepperMotorParameters.motorType = StepperMotorType::ROTARY;
  _stepperMotorParameters.deviceName = DUMMY_DEVICE_FILE_NAME;
  _stepperMotorParameters.moduleName = moduleName;
  _stepperMotorParameters.configFileName = stepperMotorDeviceConfigFile;
  _stepperMotor = StepperMotorFactory::instance().create(_stepperMotorParameters);
  _rotaryMotor = std::dynamic_pointer_cast<RotaryStepperMotor>(_stepperMotor);
  BOOST_TEST_REQUIRE(_rotaryMotor != nullptr);
}

RotaryStepperMotorFixture::~RotaryStepperMotorFixture() {
  // A calibration which is still running would keep stepping a motor which is about to be destroyed, and its state
  // machine would still be directable by a later test. A blocked motor never finishes its search, so the stop has to
  // abort it. The motor may then be in the error state, from which only a reset leads back to a settleable state.
  waitFor([this] {
    _stepperMotor->stop();
    _stepperMotor->resetError();
    return _stepperMotor->isSystemIdle();
  });
  if(_stopThread.joinable()) {
    _stopThread.join();
  }
  _stepperMotor->setEnabled(false);
  _stepperMotor.reset();
  _motorControlerDummy->resetInternalStateToDefaults();
}

void RotaryStepperMotorFixture::startCalibration() {
  _stepperMotor->setEnabled(true);
  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "idle"; }));
  BOOST_TEST(static_cast<int>(_stepperMotor->calibrate()) == static_cast<int>(ExitStatus::SUCCESS));
  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "calibrating"; }));
}

void RotaryStepperMotorFixture::driveTo(int motorControllerPosition) {
  // The dummy only steps while the motor current is on and lets the motor stop at an end switch.
  _stepperMotor->setEnabled(true);
  _motorControlerDummy->setTargetPosition(motorControllerPosition);
  while(_motorControlerDummy->isMotorMoving()) {
    _motorControlerDummy->moveTowardsTarget(1.0F, false);
  }
}

void RotaryStepperMotorFixture::driveToEndSwitch() {
  driveTo(POSITIVE_END_SWITCH_POSITION);
  BOOST_TEST_REQUIRE(_stepperMotor->isPositiveReferenceActive());
}

void RotaryStepperMotorFixture::advanceOneChunk() const {
  std::this_thread::sleep_for(CHUNK_PERIOD);
  _motorControlerDummy->moveTowardsTarget(1.0F, false);
}

void RotaryStepperMotorFixture::stopDuringChunkAt(int chunkStartPosition) {
  // A stop issued before the first chunk only aborts the search between two chunks. To interrupt a chunk itself, the
  // stop has to wait until the motor has not reached the chunk target yet: only then can the stop thread stay ahead
  // of the position. The motor must also be moving, because a chunk left alone anyway must stay a race.
  const int stopAheadOfPosition = 2 * CHUNK_STEP;
  _stopThread = std::thread([this, chunkStartPosition] {
    BOOST_TEST(waitFor([this, chunkStartPosition] {
      return _stepperMotor->getTargetPositionInSteps() != chunkStartPosition &&
          _stepperMotor->getTargetPositionInSteps() != _stepperMotor->getCurrentPositionInSteps();
    }));
    BOOST_TEST(waitFor([this] { return _motorControlerDummy->isMotorMoving(); }));
    BOOST_TEST(waitFor([this, chunkStartPosition] {
      return _stepperMotor->getCurrentPositionInSteps() > chunkStartPosition - stopAheadOfPosition;
    }));
    _stepperMotor->stop();
  });
}

void RotaryStepperMotorFixture::runToleranceCalculationToEnd() {
  // The tolerance calculation waits for each move to reach its target, so the dummy has to reach its target for the
  // worker to continue. A move towards an active end switch does not start in the dummy, so it is left alone.
  for(unsigned iteration = 0; iteration < MAX_TOLERANCE_ITERATIONS; ++iteration) {
    if(_stepperMotor->isSystemIdle()) {
      return;
    }
    if(_motorControlerDummy->isMotorMoving()) {
      _motorControlerDummy->moveTowardsTarget(1.0F, false);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_PERIOD_MS));
  }
  BOOST_FAIL("tolerance calculation did not finish within " << MAX_TOLERANCE_ITERATIONS << " iterations");
}

void RotaryStepperMotorFixture::runCalibrationToEnd() {
  // Each chunk moves the motor to its target, so the background thread only issues the next one afterwards.
  for(unsigned chunk = 0; chunk < MAX_CALIBRATION_CHUNKS; ++chunk) {
    if(_stepperMotor->isSystemIdle()) {
      return;
    }
    advanceOneChunk();
  }
  BOOST_TEST_REQUIRE(
      _stepperMotor->isSystemIdle(), "calibration did not finish within " << MAX_CALIBRATION_CHUNKS << " chunks");
}

BOOST_FIXTURE_TEST_SUITE(RotaryStepperMotorTest, RotaryStepperMotorFixture)

/**
 * Specification: `RotaryStepperMotorStateMachine::performCalibration()`.
 *
 * "Move off the switch when `isEndSwitchActive(Sign::POSITIVE)` is already true at the start."
 * "Move negatively in chunks of `getOffset()` until the switch releases."
 * "Then home positively as today."
 */
BOOST_AUTO_TEST_CASE(HomingFromActiveSwitchMovesOffFirst) {
  driveToEndSwitch();
  startCalibration();

  // The release search moves negatively, away from the positive end switch.
  BOOST_TEST(waitFor([this] {
    return _stepperMotor->getTargetPositionInSteps() == POSITIVE_END_SWITCH_POSITION - END_SWITCH_SEARCH_OFFSET;
  }));

  runCalibrationToEnd();

  BOOST_TEST(_stepperMotor->getState() == "idle");
  BOOST_TEST(_stepperMotor->isCalibrated() == true);
  BOOST_TEST(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::FULL));
  BOOST_TEST(_stepperMotor->isPositiveReferenceActive() == true);
  BOOST_TEST(_stepperMotor->getPositiveEndReferenceInSteps() == _stepperMotor->getNegativeEndReferenceInSteps());
  BOOST_TEST(_stepperMotor->getPositiveEndReferenceInSteps() == POSITIVE_END_SWITCH_POSITION);
}

/**
 * Specification: `RotaryStepperMotor::verifyMoveAction()`.
 *
 * "Keep the base disjunction: accept the move when `isEndSwitchActive(Sign::POSITIVE)` is true or the target
 * position equals the actual position."
 * "Drop only the negative switch bit, so it cannot mask a blocked move."
 *
 * The tolerance calculation moves away from the end switch and verifies that move while the positive switch is
 * inactive. With the conditions combined as a conjunction the calculation would fail.
 */
BOOST_AUTO_TEST_CASE(ToleranceCalculationSucceedsAfterHoming) {
  startCalibration();
  runCalibrationToEnd();
  BOOST_TEST_REQUIRE(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::FULL));

  BOOST_TEST(static_cast<int>(_stepperMotor->determineTolerance()) == static_cast<int>(ExitStatus::SUCCESS));
  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "calculatingTolerance"; }));

  runToleranceCalculationToEnd();

  BOOST_TEST(_stepperMotor->getState() == "idle");
  BOOST_TEST(static_cast<int>(_stepperMotor->getError()) == static_cast<int>(Error::NO_ERROR));
  BOOST_TEST(std::isfinite(_stepperMotor->getTolerancePositiveEndSwitch()));
}

/**
 * Specification: `ReferenceStateMachine`.
 *
 * "`_calculatingTolerance` shares `actionStop()`, so stopping a tolerance calculation also commands standstill."
 *
 * During the calculation the motor is moving already before the first iteration completes.
 */
BOOST_AUTO_TEST_CASE(StopDuringToleranceCalculationCommandsStandstill) {
  startCalibration();
  runCalibrationToEnd();
  BOOST_TEST_REQUIRE(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::FULL));

  BOOST_TEST_REQUIRE(static_cast<int>(_stepperMotor->determineTolerance()) == static_cast<int>(ExitStatus::SUCCESS));
  BOOST_TEST_REQUIRE(waitFor([this] { return _stepperMotor->getState() == "calculatingTolerance"; }));

  // The calculation starts by moving close to the end switch, 1000 steps away from it. The move is left running, so
  // the stop takes the target of a moving motor back.
  BOOST_TEST_REQUIRE(waitFor([this] {
    return _stepperMotor->getTargetPositionInSteps() == POSITIVE_END_SWITCH_POSITION - 1000 &&
        _motorControlerDummy->isMotorMoving();
  }));

  _stepperMotor->stop();

  BOOST_TEST(!_motorControlerDummy->isMotorMoving());
  BOOST_TEST(_stepperMotor->getTargetPositionInSteps() == _stepperMotor->getCurrentPositionInSteps());
  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "idle"; }));
}

/**
 * Requirements, rotary switch semantics.
 *
 * "A rotary motor must report the negative end switch as inactive."
 * "The both-end-switches-active error must never be raised for a rotary motor."
 *
 * Both end switch bits of the dummy are set, but only the positive one belongs to the rotary motor.
 */
BOOST_AUTO_TEST_CASE(NegativeSwitchBitIsIgnored) {
  _stepperMotor->setEnabled(true);
  _motorControlerDummy->moveTowardsTarget(0.0F, false, true);

  BOOST_TEST_REQUIRE(_motorControlerDummy->getReferenceSwitchBit() == 1U);

  BOOST_TEST(_stepperMotor->isNegativeReferenceActive() == false);
  BOOST_TEST(_stepperMotor->isPositiveReferenceActive() == true);
  BOOST_TEST(_stepperMotor->getState() == "idle");
  BOOST_TEST(static_cast<int>(_stepperMotor->getError()) == static_cast<int>(Error::NO_ERROR));
}

/**
 * Requirements, rotary switch semantics.
 *
 * "Move verification must not treat the negative switch bit as a reason to accept a move."
 *
 * The motor is calibrated, then a move is blocked while the negative switch bit stays set. The move has to be
 * reported as interrupted.
 */
BOOST_AUTO_TEST_CASE(BlockedMoveReportedAsInterrupted) {
  // Calibrate first, a move is only accepted on a calibrated motor.
  startCalibration();
  runCalibrationToEnd();
  BOOST_TEST_REQUIRE(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::FULL));

  // Calibration left the motor on the positive end switch, so it has to move away before a move can be checked.
  driveTo(POSITIVE_END_SWITCH_POSITION - END_SWITCH_SEARCH_OFFSET);
  BOOST_TEST_REQUIRE(!_stepperMotor->isPositiveReferenceActive());

  // The dummy reports a negative end switch at the low end of its travel, which a rotary motor has to ignore.
  _motorControlerDummy->setNegativeEndSwitch(POSITIVE_END_SWITCH_POSITION - END_SWITCH_SEARCH_OFFSET);
  _motorControlerDummy->moveTowardsTarget(0.0F, false, false);
  BOOST_TEST_REQUIRE(_motorControlerDummy->getReferenceSwitchData().getNegativeSwitchActive() == 1U);
  BOOST_TEST_REQUIRE(!_stepperMotor->isPositiveReferenceActive());

  BOOST_TEST_REQUIRE(static_cast<int>(_stepperMotor->setTargetPositionInSteps(POSITIVE_END_SWITCH_POSITION -
                         2 * END_SWITCH_SEARCH_OFFSET)) == static_cast<int>(ExitStatus::SUCCESS));
  _motorControlerDummy->setTargetPosition(POSITIVE_END_SWITCH_POSITION - 2 * END_SWITCH_SEARCH_OFFSET);
  _motorControlerDummy->simulateBlockedMotor(true);
  _stepperMotor->start();

  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "error"; }));

  BOOST_TEST(static_cast<int>(_stepperMotor->getError()) == static_cast<int>(Error::MOVE_INTERRUPTED));
}

/**
 * Specification: `RotaryStepperMotorStateMachine::performCalibration()`.
 *
 * "The release search detects a blocked motor itself. After each chunk it checks whether the target position still
 * equals the position it had before the chunk, and sets `_moveInterrupted` when the motor did not move while the
 * switch stays active."
 * "Abort the release search on stop or move interruption."
 */
BOOST_AUTO_TEST_CASE(BlockedMotorDuringReleaseSearchFailsWithCalibrationError) {
  driveToEndSwitch();
  startCalibration();

  BOOST_TEST_REQUIRE(waitFor([this] {
    return _stepperMotor->getTargetPositionInSteps() == POSITIVE_END_SWITCH_POSITION - END_SWITCH_SEARCH_OFFSET;
  }));

  // The motor cannot move and the switch stays active, so the search has to give up instead of grinding chunks.
  _motorControlerDummy->simulateBlockedMotor(true);

  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() != "calibrating"; }));

  BOOST_TEST(_stepperMotor->getState() == "idle");
  BOOST_TEST(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::NONE));
  BOOST_TEST(static_cast<int>(_stepperMotor->getError()) == static_cast<int>(Error::CALIBRATION_ERROR));
  BOOST_TEST(_stepperMotor->isCalibrated() == false);
}

/**
 * Requirements, calibration abort.
 *
 * "Stopping during calibration must stop motion."
 * "The motor must end in the idle state."
 *
 * The positive switch is inactive at the start, so the stop aborts the homing search.
 */
BOOST_AUTO_TEST_CASE(StopDuringCalibrationStopsMotor) {
  startCalibration();

  BOOST_TEST_REQUIRE(waitFor([this] {
    return _stepperMotor->getTargetPositionInSteps() == END_SWITCH_SEARCH_OFFSET &&
        _motorControlerDummy->isMotorMoving();
  }));

  _stepperMotor->stop();

  BOOST_TEST(waitFor([this] { return !_motorControlerDummy->isMotorMoving(); }));
  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "idle"; }));
  BOOST_TEST(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::NONE));
}

/**
 * Specification: `RotaryStepperMotorStateMachine::performCalibration()`.
 *
 * "Abort the release search on stop or move interruption."
 *
 * The positive switch is active at the start, so the calibration runs the release search. The stop lands while the
 * first negative chunk runs.
 */
BOOST_AUTO_TEST_CASE(StopDuringReleaseSearchStopsMotor) {
  driveToEndSwitch();
  startCalibration();

  BOOST_TEST_REQUIRE(waitFor([this] {
    return _stepperMotor->getTargetPositionInSteps() == POSITIVE_END_SWITCH_POSITION - END_SWITCH_SEARCH_OFFSET &&
        _motorControlerDummy->isMotorMoving();
  }));
  stopDuringChunkAt(POSITIVE_END_SWITCH_POSITION);

  BOOST_TEST(waitFor([this] { return !_motorControlerDummy->isMotorMoving(); }));
  // The released switch makes the search stop moving at once, so the stop may also be processed between two chunks.
  BOOST_TEST(_stepperMotor->getTargetPositionInSteps() == _stepperMotor->getCurrentPositionInSteps());
  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "idle"; }));
  BOOST_TEST(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::NONE));
  BOOST_TEST(static_cast<int>(_stepperMotor->getError()) == static_cast<int>(Error::CALIBRATION_ERROR));
}

/**
 * Requirements, calibration abort.
 *
 * "Disabling during calibration must stop motion."
 * "The motor must end in the disabled state with motor current and end-switch power off."
 */
BOOST_AUTO_TEST_CASE(DisableDuringCalibrationStopsMotorAndDisables) {
  startCalibration();

  BOOST_TEST_REQUIRE(waitFor([this] {
    return _stepperMotor->getTargetPositionInSteps() == END_SWITCH_SEARCH_OFFSET &&
        _motorControlerDummy->isMotorMoving();
  }));

  _stepperMotor->setEnabled(false);

  BOOST_TEST(waitFor([this] { return !_motorControlerDummy->isMotorMoving(); }));
  BOOST_TEST(waitFor([this] { return _stepperMotor->getState() == "disabled"; }));
  BOOST_TEST(_stepperMotor->getEnabled() == false);
  BOOST_TEST(_motorControlerDummy->isMotorCurrentEnabled() == false);
  BOOST_TEST(_motorControlerDummy->isEndSwitchPowerEnabled() == false);
  BOOST_TEST(static_cast<int>(_stepperMotor->getCalibrationMode()) == static_cast<int>(CalibrationMode::NONE));
  BOOST_TEST(_stepperMotor->isCalibrated() == false);
}

BOOST_AUTO_TEST_SUITE_END()

// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RotaryStepperMotorTest

#include "DFMC_MD22Dummy.h"
#include "MotorControlerDummy.h"
#include "MotorDriverCard.h"
#include "MotorDriverCardFactory.h"
#include "RotaryStepperMotor.h"
#include "StepperMotor.h"
#include "testConfigConstants.h"

#include <ChimeraTK/Exception.h>

#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <thread>

using namespace boost::unit_test_framework;
using namespace ChimeraTK::MotorDriver;

namespace {

  const std::string stepperMotorDeviceConfigFile("VT21-MotorDriverCardConfig.xml");
  const std::string moduleName("");

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

  // A full rotary calibration needs a release chunk and a homing chunk, this bound only guards against a runaway.
  const unsigned MAX_CALIBRATION_CHUNKS = 10U;

  /// Polls a condition until it holds or the timeout expires.
  template<typename Condition>
  bool waitFor(const Condition& condition) {
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
    RotaryStepperMotorFixture();
    ~RotaryStepperMotorFixture();

    void startCalibration();
    /// Drives the dummy to the given motor controller position, using the motor current of the motor.
    void driveTo(int motorControllerPosition);
    void driveToEndSwitch();
    /// Lets the background calibration thread advance by one chunk.
    void advanceOneChunk();
    /// Runs the calibration until the motor is idle.
    void runCalibrationToEnd();

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
    BOOST_REQUIRE(_rotaryMotor != nullptr);
  }

  RotaryStepperMotorFixture::~RotaryStepperMotorFixture() {
    // A calibration thread which is still running would keep stepping a motor which is about to be destroyed.
    _stepperMotor->setEnabled(false);
    waitFor([this] { return !_motorControlerDummy->isMotorMoving(); });
    _stepperMotor.reset();
    _motorControlerDummy->resetInternalStateToDefaults();
  }

  void RotaryStepperMotorFixture::startCalibration() {
    _stepperMotor->setEnabled(true);
    BOOST_CHECK(waitFor([this] { return _stepperMotor->getState() == "idle"; }));
    BOOST_CHECK(_stepperMotor->calibrate() == ExitStatus::SUCCESS);
    BOOST_CHECK(waitFor([this] { return _stepperMotor->getState() == "calibrating"; }));
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
    BOOST_REQUIRE(_stepperMotor->isPositiveReferenceActive());
  }

  void RotaryStepperMotorFixture::advanceOneChunk() {
    std::this_thread::sleep_for(CHUNK_PERIOD);
    _motorControlerDummy->moveTowardsTarget(1.0F, false);
  }

  void RotaryStepperMotorFixture::runCalibrationToEnd() {
    // Each chunk moves the motor to its target, so the background thread only issues the next one afterwards.
    for(unsigned chunk = 0; chunk < MAX_CALIBRATION_CHUNKS; ++chunk) {
      if(_stepperMotor->isSystemIdle()) {
        return;
      }
      advanceOneChunk();
    }
    BOOST_REQUIRE_MESSAGE(
        _stepperMotor->isSystemIdle(), "calibration did not finish within " << MAX_CALIBRATION_CHUNKS << " chunks");
  }

} // namespace

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
  BOOST_CHECK(waitFor([this] {
    return _stepperMotor->getTargetPositionInSteps() == POSITIVE_END_SWITCH_POSITION - END_SWITCH_SEARCH_OFFSET;
  }));

  runCalibrationToEnd();

  BOOST_CHECK_EQUAL(_stepperMotor->getState(), "idle");
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), true);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::FULL);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveReferenceActive(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), _stepperMotor->getNegativeEndReferenceInSteps());
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), POSITIVE_END_SWITCH_POSITION);
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

  BOOST_REQUIRE_EQUAL(_motorControlerDummy->getReferenceSwitchBit(), 1U);

  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeReferenceActive(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveReferenceActive(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->getState(), "idle");
  BOOST_CHECK(_stepperMotor->getError() == Error::NO_ERROR);
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
  BOOST_REQUIRE(_stepperMotor->getCalibrationMode() == CalibrationMode::FULL);

  // Calibration left the motor on the positive end switch, so it has to move away before a move can be checked.
  driveTo(POSITIVE_END_SWITCH_POSITION - END_SWITCH_SEARCH_OFFSET);
  BOOST_REQUIRE(!_stepperMotor->isPositiveReferenceActive());

  // The dummy reports a negative end switch at the low end of its travel, which a rotary motor has to ignore.
  _motorControlerDummy->setNegativeEndSwitch(POSITIVE_END_SWITCH_POSITION - END_SWITCH_SEARCH_OFFSET);
  _motorControlerDummy->moveTowardsTarget(0.0F, false, false);
  BOOST_REQUIRE_EQUAL(_motorControlerDummy->getReferenceSwitchData().getNegativeSwitchActive(), 1U);
  BOOST_REQUIRE(!_stepperMotor->isPositiveReferenceActive());

  _stepperMotor->setTargetPositionInSteps(POSITIVE_END_SWITCH_POSITION - 2 * END_SWITCH_SEARCH_OFFSET);
  _motorControlerDummy->setTargetPosition(POSITIVE_END_SWITCH_POSITION - 2 * END_SWITCH_SEARCH_OFFSET);
  _motorControlerDummy->simulateBlockedMotor(true);
  _stepperMotor->start();

  BOOST_CHECK(waitFor([this] { return _stepperMotor->getState() == "error"; }));

  BOOST_CHECK(_stepperMotor->getError() == Error::MOVE_INTERRUPTED);
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

  BOOST_REQUIRE(waitFor([this] {
    return _stepperMotor->getTargetPositionInSteps() == POSITIVE_END_SWITCH_POSITION - END_SWITCH_SEARCH_OFFSET;
  }));

  // The motor cannot move and the switch stays active, so the search has to give up instead of grinding chunks.
  _motorControlerDummy->simulateBlockedMotor(true);

  BOOST_CHECK(waitFor([this] { return _stepperMotor->getState() != "calibrating"; }));

  BOOST_CHECK_EQUAL(_stepperMotor->getState(), "idle");
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);
  BOOST_CHECK(_stepperMotor->getError() == Error::CALIBRATION_ERROR);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
}

/**
 * Requirements, calibration abort.
 *
 * "Stopping during calibration must stop motion."
 * "The motor must end in the idle state."
 */
BOOST_AUTO_TEST_CASE(StopDuringCalibrationStopsMotor) {
  startCalibration();

  BOOST_REQUIRE(waitFor([this] {
    return _stepperMotor->getTargetPositionInSteps() == END_SWITCH_SEARCH_OFFSET &&
        _motorControlerDummy->isMotorMoving();
  }));

  _stepperMotor->stop();

  BOOST_CHECK(waitFor([this] { return !_motorControlerDummy->isMotorMoving(); }));
  BOOST_CHECK(waitFor([this] { return _stepperMotor->getState() == "idle"; }));
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);
}

/**
 * Requirements, calibration abort.
 *
 * "Disabling during calibration must stop motion."
 * "The motor must end in the disabled state with motor current and end-switch power off."
 */
BOOST_AUTO_TEST_CASE(DisableDuringCalibrationStopsMotorAndDisables) {
  startCalibration();

  BOOST_REQUIRE(waitFor([this] {
    return _stepperMotor->getTargetPositionInSteps() == END_SWITCH_SEARCH_OFFSET &&
        _motorControlerDummy->isMotorMoving();
  }));

  _stepperMotor->setEnabled(false);

  BOOST_CHECK(waitFor([this] { return !_motorControlerDummy->isMotorMoving(); }));
  BOOST_CHECK(waitFor([this] { return _stepperMotor->getState() == "disabled"; }));
  BOOST_CHECK_EQUAL(_stepperMotor->getEnabled(), false);
  BOOST_CHECK_EQUAL(_motorControlerDummy->isMotorCurrentEnabled(), false);
  BOOST_CHECK_EQUAL(_motorControlerDummy->isEndSwitchPowerEnabled(), false);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
}

BOOST_AUTO_TEST_SUITE_END()

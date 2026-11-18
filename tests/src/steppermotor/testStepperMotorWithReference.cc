#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE StepperMotorWithReferenceTest

/**
 *TODO: Rename to testLinearStepperMotor
 *  This can be done when the mtca4u is removed or
 *  specific output directories for both interfaces
 *  are required to avoid nameing conflicts.
 */

#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>

#include <memory>
#include <thread>
using namespace boost::unit_test_framework;

#include "DFMC_MD22Dummy.h"
#include "LinearStepperMotor.h"
#include "MotorControlerDummy.h"
#include "MotorDriverCard.h"
#include "MotorDriverCardFactory.h"
#include "ReferenceStateMachine.h"
#include "StepperMotor.h"
#include "testConfigConstants.h"
#include "TMC429Constants.h"

#include <ChimeraTK/Utilities.h>

using namespace ChimeraTK::MotorDriver;

static const std::string stepperMotorDeviceConfigFile("VT21-MotorDriverCardConfig.xml");
static const std::string moduleName("");

// Position of end switches
// Calibration routine defines zero at negative endswitch
static const int POS_POSITIVE_ENDSWITCH_MOTORCONTROLLER = 10000;
static const int POS_NEGATIVE_ENDSWITCH_MOTORCONTROLLER = -10000;
static const int POS_POSITIVE_ENDSWITCH_STEPPERMOTOR = 20000;
static const int POS_NEGATIVE_ENDSWITCH_STEPPERMOTOR = 0;

// Calibration routine iterates in steps of 50000 relative to the current
// position (in MotorController coordinates), so these should be the target
// positions set by the routine.
static const int POS_BEYOND_POSITVE_ENDSWITCH = 50000;
static const int POS_BEYOND_NEGATIVE_ENDSWITCH = -40000;

class StepperMotorWithReferenceTestFixture {
 public:
  StepperMotorWithReferenceTestFixture();
  void waitToSetTargetPos(int);
  void waitForPositiveEndSwitchActive();
  void waitForNegativeEndSwitchActive();
  bool waitForState(std::string stateName, unsigned timeoutInSeconds);

  void performCalibrationForTest();

  // Helper functions to check protected members in test
  bool getCalibrationFailed();
  bool getMoveInterrupted();

 protected:
  StepperMotorParameters _stepperMotorParameters;
  std::shared_ptr<LinearStepperMotor> _stepperMotor;
  boost::shared_ptr<mtca4u::MotorDriverCard> _motorDriverCard;
  boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;
};

StepperMotorWithReferenceTestFixture::StepperMotorWithReferenceTestFixture()
: _stepperMotorParameters(), _stepperMotor(), _motorControlerDummy() {
  mtca4u::MotorDriverCardFactory::instance().setDummyMode();
  _motorDriverCard = mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(
      DUMMY_DEVICE_FILE_NAME, moduleName, stepperMotorDeviceConfigFile);
  _motorControlerDummy =
      boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(_motorDriverCard->getMotorControler(0));

  // Define dummy end switches and reset the dummy
  _motorControlerDummy->setPositiveEndSwitch(POS_POSITIVE_ENDSWITCH_MOTORCONTROLLER);
  _motorControlerDummy->setNegativeEndSwitch(POS_NEGATIVE_ENDSWITCH_MOTORCONTROLLER);
  _motorControlerDummy->resetInternalStateToDefaults();

  // Create the StepperMotor object
  _stepperMotorParameters.deviceName = DUMMY_DEVICE_FILE_NAME;
  _stepperMotorParameters.moduleName = moduleName;
  _stepperMotorParameters.configFileName = stepperMotorDeviceConfigFile;
  _stepperMotor = std::make_shared<LinearStepperMotor>(_stepperMotorParameters);
}

bool StepperMotorWithReferenceTestFixture::waitForState(std::string reqStateName, unsigned timeoutInSeconds = 10) {
  unsigned sleepPeriodInMilliSec = 100;
  unsigned timeoutInSteps = timeoutInSeconds * 1000 / sleepPeriodInMilliSec;
  unsigned cnt = 0;
  bool isCorrectState = false;
  do {
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepPeriodInMilliSec));

    auto actStateName = _stepperMotor->getState();
    if(actStateName == reqStateName) {
      isCorrectState = true;
      break;
    }
    else {
      cnt++;
    }
  } while(cnt <= timeoutInSteps);

  return isCorrectState;
}

void StepperMotorWithReferenceTestFixture::waitToSetTargetPos(int targetPos) {
  while(true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if(_stepperMotor->getTargetPositionInSteps() == targetPos) {
      break;
    }
  }
}

void StepperMotorWithReferenceTestFixture::waitForPositiveEndSwitchActive() {
  while(1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    _stepperMotor->_mutex.lock();
    if(!_stepperMotor->_motorController->getReferenceSwitchData().getPositiveSwitchActive()) {
      _stepperMotor->_mutex.unlock();
    }
    else {
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTestFixture::waitForNegativeEndSwitchActive() {
  while(1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    _stepperMotor->_mutex.lock();
    if(!_stepperMotor->_motorController->getReferenceSwitchData().getNegativeSwitchActive()) {
      _stepperMotor->_mutex.unlock();
    }
    else {
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

bool StepperMotorWithReferenceTestFixture::getCalibrationFailed() {
  return _stepperMotor->_calibrationFailed.load();
}

bool StepperMotorWithReferenceTestFixture::getMoveInterrupted() {
  return (std::dynamic_pointer_cast<ReferenceStateMachine>(_stepperMotor->_stateMachine))->isMoveInterrupted();
}

/**
 * Perfroms calibration which requires moving the motor up to the positive
 * and back to the negative end switch
 *
 * Notes:
 *   - This becomes obsolete once there is a proper dummy which reacts on writes
 *     to the registers instead of requiring an explicit call to
 * moveTowwardsTarget()
 */
void StepperMotorWithReferenceTestFixture::performCalibrationForTest() {
  BOOST_CHECK(waitForState("idle"));
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("calibrating"));

  // Attempt to move beyond end swtich
  waitToSetTargetPos(POS_BEYOND_POSITVE_ENDSWITCH);
  _motorControlerDummy->moveTowardsTarget(1);
  waitForPositiveEndSwitchActive();
  waitToSetTargetPos(POS_BEYOND_NEGATIVE_ENDSWITCH);
  _motorControlerDummy->moveTowardsTarget(1);
  waitForNegativeEndSwitchActive();
  BOOST_CHECK(waitForState("idle"));
}

BOOST_FIXTURE_TEST_SUITE(StepperMotorWithReferenceTest, StepperMotorWithReferenceTestFixture)

BOOST_AUTO_TEST_CASE(testCalibrate) {
  // Test calibration w/ disabled end switches
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);
  // End swtich enabled for StepperMotorWithReference gets only determined in
  // ctor, so reconstruct here
  _stepperMotor = std::make_shared<LinearStepperMotor>(_stepperMotorParameters);

  _stepperMotor->setEnabled(true);
  BOOST_CHECK(waitForState("idle"));

  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);

  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeEndSwitchEnabled(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveEndSwitchEnabled(), false);

  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());

  BOOST_CHECK(waitForState("idle"));
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);

  // Test calibration w/ disabled positive end switch
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(true);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _stepperMotor = std::make_shared<LinearStepperMotor>(_stepperMotorParameters);

  _stepperMotor->setEnabled(true);
  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeEndSwitchEnabled(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveEndSwitchEnabled(), false);
  BOOST_CHECK(waitForState("idle"));

  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("idle"));

  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);

  // Test calibration w/ disabled negative end switch
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
  _stepperMotor = std::make_shared<LinearStepperMotor>(_stepperMotorParameters);

  _stepperMotor->setEnabled(true);
  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeEndSwitchEnabled(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveEndSwitchEnabled(), true);
  BOOST_CHECK(waitForState("idle"));
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("idle"));
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);

  // Test calibration w/ end switches enabled
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(true);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
  _stepperMotor = std::make_shared<LinearStepperMotor>(_stepperMotorParameters);

  _stepperMotor->setEnabled(true);
  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeEndSwitchEnabled(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveEndSwitchEnabled(), true);
  BOOST_CHECK(waitForState("idle"));
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("calibrating"));
  waitToSetTargetPos(POS_BEYOND_POSITVE_ENDSWITCH);
  _motorControlerDummy->moveTowardsTarget(1);
  waitForPositiveEndSwitchActive();
  waitToSetTargetPos(POS_BEYOND_NEGATIVE_ENDSWITCH);

  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), POS_POSITIVE_ENDSWITCH_MOTORCONTROLLER);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveReferenceActive(), true);
  BOOST_CHECK(_stepperMotor->setTargetPositionInSteps(0) == ExitStatus::ERR_SYSTEM_IN_ACTION);
  BOOST_CHECK(_stepperMotor->moveRelativeInSteps(-100) == ExitStatus::ERR_SYSTEM_IN_ACTION);
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), POS_POSITIVE_ENDSWITCH_MOTORCONTROLLER);
  _motorControlerDummy->moveTowardsTarget(1);

  while(!_stepperMotor->isSystemIdle()) {
  }
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), POS_POSITIVE_ENDSWITCH_STEPPERMOTOR);
  BOOST_CHECK_EQUAL(_stepperMotor->getNegativeEndReferenceInSteps(), POS_NEGATIVE_ENDSWITCH_STEPPERMOTOR);
  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeReferenceActive(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), true);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), false);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::FULL);
  BOOST_CHECK(_stepperMotor->getError() == Error::NO_ERROR);
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), 0);
  while(!_stepperMotor->isSystemIdle()) {
  }
  _stepperMotor->setTargetPositionInSteps(100);
  _stepperMotor->start();
  BOOST_CHECK(waitForState("moving", 1));
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotor->isSystemIdle()) {
  }
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), 100);
}

BOOST_AUTO_TEST_CASE(testCalibrateError) {
  _stepperMotor->setEnabled(true);
  while(!_stepperMotor->isSystemIdle()) {
  }
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("calibrating"));
  waitToSetTargetPos(POS_BEYOND_POSITVE_ENDSWITCH);
  _motorControlerDummy->moveTowardsTarget(1);
  waitForPositiveEndSwitchActive();
  waitToSetTargetPos(POS_BEYOND_NEGATIVE_ENDSWITCH);
  _motorControlerDummy->moveTowardsTarget(0.01f);
  _motorControlerDummy->simulateBlockedMotor(true);
  while(!_stepperMotor->isSystemIdle()) {
  }

  // BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(),
  // POS_POSITIVE_ENDSWITCH_MOTORCONTROLLER);
  BOOST_CHECK_EQUAL(getMoveInterrupted(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  _motorControlerDummy->simulateBlockedMotor(false);
  BOOST_CHECK(_stepperMotor->getError() == Error::CALIBRATION_ERROR);
  _motorControlerDummy->resetInternalStateToDefaults();
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("calibrating"));
  waitToSetTargetPos(POS_BEYOND_POSITVE_ENDSWITCH);
  _motorControlerDummy->moveTowardsTarget(0.01f);
  _motorControlerDummy->simulateBlockedMotor(true);
  while(!_stepperMotor->isSystemIdle()) {
  }
  BOOST_CHECK_EQUAL(getMoveInterrupted(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  _motorControlerDummy->simulateBlockedMotor(false);
  BOOST_CHECK(_stepperMotor->getError() == Error::CALIBRATION_ERROR);
}

BOOST_AUTO_TEST_CASE(testCalibrateStop) {
  _motorControlerDummy->setPositiveEndSwitch(60000);
  _motorControlerDummy->setNegativeEndSwitch(-60000);

  _stepperMotor->setEnabled(true);

  while(!_stepperMotor->isSystemIdle()) {
  }
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("calibrating"));
  waitToSetTargetPos(POS_BEYOND_POSITVE_ENDSWITCH);
  _stepperMotor->stop();

  _motorControlerDummy->moveTowardsTarget(1);
  BOOST_CHECK(waitForState("idle"));

  BOOST_CHECK_EQUAL(getMoveInterrupted(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);
  BOOST_CHECK(_stepperMotor->getError() == Error::CALIBRATION_ERROR);

  _motorControlerDummy->resetInternalStateToDefaults();
  while(!_stepperMotor->isSystemIdle()) {
  }
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  waitForState("calibrating");
  waitToSetTargetPos(POS_BEYOND_POSITVE_ENDSWITCH);
  _motorControlerDummy->moveTowardsTarget(1);
  waitToSetTargetPos(100000);

  _stepperMotor->stop();
  _motorControlerDummy->moveTowardsTarget(1);
  BOOST_CHECK(waitForState("idle"));

  BOOST_CHECK_EQUAL(getMoveInterrupted(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::NONE);
  BOOST_CHECK(_stepperMotor->getError() == Error::CALIBRATION_ERROR);
}

BOOST_AUTO_TEST_CASE(testTranslation) {
  // Make sure we are in simple calibration mode
  _stepperMotor->setActualPosition(0.f);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::SIMPLE);

  while(!_stepperMotor->isSystemIdle()) {
  }

  _stepperMotor->setEnabled(true);
  BOOST_CHECK(_stepperMotor->translateAxisInSteps(100) == ExitStatus::SUCCESS);

  // Check if position limits have been shifted
  BOOST_CHECK_EQUAL(_stepperMotor->getMaxPositionLimitInSteps(), std::numeric_limits<int>::max());
  BOOST_CHECK_EQUAL(_stepperMotor->getMinPositionLimitInSteps(), std::numeric_limits<int>::min() + 100);

  // As the motor is not fully calibrated, end switches should not have been
  // shifted
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), std::numeric_limits<int>::max());
  BOOST_CHECK_EQUAL(_stepperMotor->getNegativeEndReferenceInSteps(), std::numeric_limits<int>::min());

  // Test calibrated motor now
  _motorControlerDummy->resetInternalStateToDefaults();
  performCalibrationForTest();

  // Check that endswitches are detected at positions defined
  // by the MotorControlerDummy and calibration mode is FULL
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), POS_POSITIVE_ENDSWITCH_STEPPERMOTOR);
  BOOST_CHECK_EQUAL(_stepperMotor->getNegativeEndReferenceInSteps(), POS_NEGATIVE_ENDSWITCH_STEPPERMOTOR);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == CalibrationMode::FULL);

  BOOST_CHECK(_stepperMotor->translateAxisInSteps(100) == ExitStatus::SUCCESS);
  BOOST_CHECK_EQUAL(_stepperMotor->getMaxPositionLimitInSteps(), std::numeric_limits<int>::max());
  BOOST_CHECK_EQUAL(_stepperMotor->getMinPositionLimitInSteps(), std::numeric_limits<int>::min() + 200);

  // In full calibration mode, also end switch positions have to be shifted
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), 20100);
  BOOST_CHECK_EQUAL(_stepperMotor->getNegativeEndReferenceInSteps(), 100);

  // Translation beyond full numeric range must fail
  // Set back to center of int limits
  _stepperMotor->translateAxisInSteps(-200);

  BOOST_CHECK(
      _stepperMotor->translateAxisInSteps(std::numeric_limits<int>::max()) == ExitStatus::ERR_INVALID_PARAMETER);
  BOOST_CHECK(
      _stepperMotor->translateAxisInSteps(std::numeric_limits<int>::min()) == ExitStatus::ERR_INVALID_PARAMETER);
}

BOOST_AUTO_TEST_SUITE_END()

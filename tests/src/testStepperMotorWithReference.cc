#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE StepperMotorWithReferenceTest

#include <boost/test/included/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/pointer_cast.hpp>
using namespace boost::unit_test_framework;

#include "StepperMotorWithReference.h"
#include "StepperMotorWithReferenceStateMachine.h"
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


class StepperMotorWithReferenceTestFixture{
public:
  StepperMotorWithReferenceTestFixture();
  void waitForCalibState();
  void waitForDetTolerance();
  void waitToSetPositiveTargetPos(int targetPos = 50000);
  void waitToSetNegativeTargetPos(int targetPos = -40000);
  void waitForMoveState();
  void waitForPositiveEndSwitchActive();
  void waitForNegativeEndSwitchActive();
  void waitForStop();
  bool waitForState(std::string stateName, unsigned timeoutInSeconds);

  void performCalibrationForTest();
  void performToleranceCalculationForTest();

  // Helper functions to check protected members in test
  bool getCalibrationFailed();
  bool getMoveInterrupted();
  bool getToleranceCalcFailed();
  bool getToleranceCalculated();


protected:
  boost::shared_ptr<ChimeraTK::StepperMotorWithReference> _stepperMotor;
  boost::shared_ptr<mtca4u::MotorControlerDummy> _motorControlerDummy;
};


using namespace ChimeraTK;


StepperMotorWithReferenceTestFixture::StepperMotorWithReferenceTestFixture() :
                    _stepperMotor(),
                    _motorControlerDummy()
{
  std::string deviceFileName(mtca4u::DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).deviceName);
  std::string mapFileName(mtca4u::DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).mapFileName);
  mtca4u::MotorDriverCardFactory::instance().setDummyMode();
  _motorControlerDummy = boost::dynamic_pointer_cast<mtca4u::MotorControlerDummy>(mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(deviceFileName,  moduleName, stepperMotorDeviceConfigFile)->getMotorControler(0));
}

bool StepperMotorWithReferenceTestFixture::waitForState(std::string stateName, unsigned timeoutInSeconds = 10){

  unsigned sleepPeriodInUsec = 100000;
  unsigned timeoutInSteps    = timeoutInSeconds*1000000/sleepPeriodInUsec;
  unsigned cnt = 0;
  bool isCorrectState = false;
  do{

    usleep(sleepPeriodInUsec);

    if(_stepperMotor->getState() == stateName){
      isCorrectState = true;
      break;
    }
    else{
      cnt++;
    }
  }while(cnt <= timeoutInSteps);

  return isCorrectState;
}


//FIXME Remove these
void StepperMotorWithReferenceTestFixture::waitForCalibState(){
  while (1){
    usleep(10);
    _stepperMotor->_mutex.lock();
    if ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "calibrating"){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTestFixture::waitForDetTolerance(){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if ((_stepperMotor->_stateMachine->getCurrentState())->getName() != "calculatingTolerance"){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTestFixture::waitToSetPositiveTargetPos(int targetPos){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if (_stepperMotor->_motorControler->getTargetPosition() != targetPos){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTestFixture::waitToSetNegativeTargetPos(int targetPos){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if (_stepperMotor->_motorControler->getTargetPosition() != targetPos){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTestFixture::waitForMoveState(){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if (_stepperMotor->_stateMachine->getCurrentState()->getName() == "movingState"){
      break;
    }
    _stepperMotor->_mutex.unlock();
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTestFixture::waitForPositiveEndSwitchActive(){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if (!_stepperMotor->_motorControler->getReferenceSwitchData().getPositiveSwitchActive()){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTestFixture::waitForNegativeEndSwitchActive(){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if (!_stepperMotor->_motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}

void StepperMotorWithReferenceTestFixture::waitForStop(){
  while (1){
    usleep(1000);
    _stepperMotor->_mutex.lock();
    if (std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotor->_stateMachine)->_stopAction != true){
      _stepperMotor->_mutex.unlock();
    }else{
      break;
    }
  }
  _stepperMotor->_mutex.unlock();
}


bool StepperMotorWithReferenceTestFixture::getCalibrationFailed(){
  return _stepperMotor->_calibrationFailed.load();
}

bool StepperMotorWithReferenceTestFixture::getMoveInterrupted(){
  return (std::dynamic_pointer_cast<StepperMotorWithReferenceStateMachine>(_stepperMotor->_stateMachine))->_moveInterrupted;
}

bool StepperMotorWithReferenceTestFixture::getToleranceCalcFailed(){
  return _stepperMotor->_toleranceCalcFailed.load();
}

bool StepperMotorWithReferenceTestFixture::getToleranceCalculated(){
  return _stepperMotor->_toleranceCalculated.load();
}

void StepperMotorWithReferenceTestFixture::performCalibrationForTest(){

  BOOST_CHECK(waitForState("idleState"));
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("calibrating"));
  waitToSetPositiveTargetPos();
  _motorControlerDummy->moveTowardsTarget(1);
  waitForPositiveEndSwitchActive();
  waitToSetNegativeTargetPos();
  _motorControlerDummy->moveTowardsTarget(1);
  waitForNegativeEndSwitchActive();
  BOOST_CHECK(waitForState("idleState"));
}

void StepperMotorWithReferenceTestFixture::performToleranceCalculationForTest(){
  // Tolerance calulation determines std deviation of end switch position
  // over 10 iterations
  const int N_TOLERANCE_CALC_SAMPLES = 10;
  const int MOVE_INFRONTOF_ES = 0, MOVE_ONTO_ES = 1;
  int moveState = MOVE_INFRONTOF_ES;
  int i = 0;
  bool isInitialStep = true;

  // Positive end switch detection step
  while(i<N_TOLERANCE_CALC_SAMPLES){

    switch(moveState){
      case MOVE_INFRONTOF_ES:

        if(_stepperMotor->getTargetPositionInSteps() < _stepperMotor->getCurrentPositionInSteps()
           || isInitialStep){
          isInitialStep = false;
          // Motor on end switch, move backwards
          _motorControlerDummy->moveTowardsTarget(1);
          moveState = MOVE_ONTO_ES;

        }
        break;

      case MOVE_ONTO_ES:
        if(_stepperMotor->getTargetPositionInSteps() > _stepperMotor->getCurrentPositionInSteps()){
          // Motor shortly before end switch, modify it and attempt to move beyond
          _motorControlerDummy->setPositiveEndSwitch(10000 + i);
          _motorControlerDummy->moveTowardsTarget(1);
          moveState = MOVE_INFRONTOF_ES;
          i++;
        }
        break;
    }
  }

  // Negative end switch detection step
  waitToSetNegativeTargetPos(_stepperMotor->getNegativeEndReferenceInSteps() + 1000);

  i = 0;
  isInitialStep = true;

  while(i<N_TOLERANCE_CALC_SAMPLES){

    switch(moveState){
      case MOVE_INFRONTOF_ES:

        if(_stepperMotor->getTargetPositionInSteps() > _stepperMotor->getCurrentPositionInSteps()
           || isInitialStep){
          isInitialStep = false;
          // Motor on end switch, move backwards
          _motorControlerDummy->moveTowardsTarget(1);
          moveState = MOVE_ONTO_ES;

        }
        break;

      case MOVE_ONTO_ES:
        if(_stepperMotor->getTargetPositionInSteps() < _stepperMotor->getCurrentPositionInSteps()){
          // Motor shortly before end switch, modify it and attempt to move beyond
          _motorControlerDummy->setNegativeEndSwitch(-10000 - i*10);
          _motorControlerDummy->moveTowardsTarget(1);
          moveState = MOVE_INFRONTOF_ES;
          i++;
        }
        break;
    }
  }

  waitForState("idleState");
}


BOOST_FIXTURE_TEST_SUITE(StepperMotorWithReferenceTest, StepperMotorWithReferenceTestFixture)



BOOST_AUTO_TEST_CASE(testCalibrate){

  // Test calibration w/ disabled end switches
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);

  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));

  _stepperMotor->setEnabled(true);
  BOOST_CHECK(waitForState("idleState"));

  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);

  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeEndSwitchEnabled(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveEndSwitchEnabled(), false);

  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());

  BOOST_CHECK(waitForState("idleState"));
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);


  // Test calibration w/ disabled positive end switch
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(true);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotor->setEnabled(true);
  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeEndSwitchEnabled(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveEndSwitchEnabled(), false);
  BOOST_CHECK(waitForState("idleState"));

  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("idleState"));

  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == StepperMotorCalibrationMode::NONE);


  // Test calibration w/ disabled negative end switch
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotor->setEnabled(true);
  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeEndSwitchEnabled(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveEndSwitchEnabled(), true);
  BOOST_CHECK(waitForState("idleState"));
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("idleState"));
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == StepperMotorCalibrationMode::NONE);


  // Test calibration w/ end switches enabled
  _motorControlerDummy->setNegativeReferenceSwitchEnabled(true);
  _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotor->setEnabled(true);
  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeEndSwitchEnabled(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveEndSwitchEnabled(), true);
  BOOST_CHECK(waitForState("idleState"));
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  BOOST_CHECK(waitForState("calibrating"));
  waitToSetPositiveTargetPos();
  _motorControlerDummy->moveTowardsTarget(1);
  waitForPositiveEndSwitchActive();
  waitToSetNegativeTargetPos();
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), 10000);
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), 10000);
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveReferenceActive(),  true);
  BOOST_CHECK_THROW(_stepperMotor->setTargetPositionInSteps(0), ChimeraTK::MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->moveRelativeInSteps(-100), ChimeraTK::MotorDriverException);
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), 10000);
  _motorControlerDummy->moveTowardsTarget(1);

  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), 20000);
  BOOST_CHECK_EQUAL(_stepperMotor->getNegativeEndReferenceInSteps(), 0);
  BOOST_CHECK_EQUAL(_stepperMotor->isNegativeReferenceActive(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), true);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), false);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == StepperMotorCalibrationMode::FULL);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), NO_ERROR);
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), 0);
  while(!_stepperMotor->isSystemIdle()){}
  _stepperMotor->setTargetPositionInSteps(100);
  _stepperMotor->start();
  waitForMoveState();
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(_stepperMotor->getCurrentPositionInSteps(), 100);
}

BOOST_AUTO_TEST_CASE(testCalibrateError){

  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotor->setEnabled(true);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  waitForCalibState();
  waitToSetPositiveTargetPos();
  _motorControlerDummy->moveTowardsTarget(1);
  waitForPositiveEndSwitchActive();
  waitToSetNegativeTargetPos();
  _motorControlerDummy->moveTowardsTarget(0.01f);
  _motorControlerDummy->simulateBlockedMotor(true);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), 10000);
  BOOST_CHECK_EQUAL(getMoveInterrupted(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  _motorControlerDummy->simulateBlockedMotor(false);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), ACTION_ERROR);
  _motorControlerDummy->resetInternalStateToDefaults();
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  waitForCalibState();
  waitToSetPositiveTargetPos();
  _motorControlerDummy->moveTowardsTarget(0.01f);
  _motorControlerDummy->simulateBlockedMotor(true);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(getMoveInterrupted(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  _motorControlerDummy->simulateBlockedMotor(false);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), ACTION_ERROR);
}

BOOST_AUTO_TEST_CASE(testCalibrateStop){

  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _motorControlerDummy->setPositiveEndSwitch(60000);
  _motorControlerDummy->setNegativeEndSwitch(-60000);
  _stepperMotor->setEnabled(true);


  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  waitForCalibState();
  waitToSetPositiveTargetPos();
  _stepperMotor->stop();
  waitForStop();
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(getMoveInterrupted(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == StepperMotorCalibrationMode::NONE);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), ACTION_ERROR);

  _motorControlerDummy->resetInternalStateToDefaults();
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_NO_THROW(_stepperMotor->calibrate());
  waitForCalibState();
  waitToSetPositiveTargetPos();
  _motorControlerDummy->moveTowardsTarget(1);
  waitToSetPositiveTargetPos(100000);
  _stepperMotor->stop();
  waitForStop();
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(getMoveInterrupted(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->isCalibrated(), false);
  BOOST_CHECK_EQUAL(getCalibrationFailed(), true);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == StepperMotorCalibrationMode::NONE);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), ACTION_ERROR);
}

BOOST_AUTO_TEST_CASE(testTranslation){

  // Reset the dummy motor, it is then in an uncalibrated state
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));

  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);

  // Make sure we are in simple calibration mode
  _stepperMotor->setActualPosition(0.f);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == StepperMotorCalibrationMode::SIMPLE);

  while(!_stepperMotor->isSystemIdle()){}

  _stepperMotor->setEnabled(true);
  BOOST_CHECK_NO_THROW(_stepperMotor->translateAxisInSteps(100));

  // Check if position limits have been shifted
  BOOST_CHECK_EQUAL(_stepperMotor->getMaxPositionLimitInSteps(), std::numeric_limits<int>::max());
  BOOST_CHECK_EQUAL(_stepperMotor->getMinPositionLimitInSteps(), std::numeric_limits<int>::min() + 100);

  // As the motor is not fully calibrated, end switches should not have been shifted
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), std::numeric_limits<int>::max());
  BOOST_CHECK_EQUAL(_stepperMotor->getNegativeEndReferenceInSteps(), std::numeric_limits<int>::min());


  // Test calibrated motor now
  _motorControlerDummy->resetInternalStateToDefaults();
  performCalibrationForTest();

  // Check that endswitches are detected at positions defined
  // by the MotorControlerDummy and calibration mode is FULL
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), 20000);
  BOOST_CHECK_EQUAL(_stepperMotor->getNegativeEndReferenceInSteps(), 0);
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == StepperMotorCalibrationMode::FULL);

  BOOST_CHECK_NO_THROW(_stepperMotor->translateAxisInSteps(100));
  BOOST_CHECK_EQUAL(_stepperMotor->getMaxPositionLimitInSteps(), std::numeric_limits<int>::max());
  BOOST_CHECK_EQUAL(_stepperMotor->getMinPositionLimitInSteps(), std::numeric_limits<int>::min() + 200);

  // In full calibration mode, also end switch positions have to be shifted
  BOOST_CHECK_EQUAL(_stepperMotor->getPositiveEndReferenceInSteps(), 20100);
  BOOST_CHECK_EQUAL(_stepperMotor->getNegativeEndReferenceInSteps(), 100);

  // Translation beyond full numeric range must fail
  // Set back to center of int limits
  BOOST_CHECK_NO_THROW(_stepperMotor->translateAxisInSteps(-200));

  BOOST_CHECK_THROW(_stepperMotor->translateAxisInSteps(std::numeric_limits<int>::max()), MotorDriverException);
  BOOST_CHECK_THROW(_stepperMotor->translateAxisInSteps(std::numeric_limits<int>::min()), MotorDriverException);
}


BOOST_AUTO_TEST_CASE(testDetermineTolerance){

  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
  _stepperMotor->setEnabled(true);

  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);
  while(!_stepperMotor->isSystemIdle()){}

  // Try determine tolerance on an uncalibrated motor, should fail
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == StepperMotorCalibrationMode::NONE);
  _stepperMotor->determineTolerance();
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(getToleranceCalculated(), false);
  BOOST_CHECK_EQUAL(getToleranceCalcFailed(),  true);


  // Bring the motor into calibrated state
  performCalibrationForTest();
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == StepperMotorCalibrationMode::FULL);

  // Determine tolerance
  _stepperMotor->determineTolerance();
  waitForDetTolerance();
  performToleranceCalculationForTest();

  // The following should result as std deviation of the 10 samples
  BOOST_CHECK_CLOSE(_stepperMotor->getTolerancePositiveEndSwitch(), 3.02765, 0.001);
  BOOST_CHECK_CLOSE(_stepperMotor->getToleranceNegativeEndSwitch(), 30.2765, 0.001);

  // Return to home position
  _stepperMotor->setTargetPositionInSteps(0);
  _stepperMotor->start();
  waitToSetPositiveTargetPos(0);
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(getMoveInterrupted(), false);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), NO_ERROR);
  BOOST_CHECK_EQUAL(getToleranceCalculated(), true);
  BOOST_CHECK_EQUAL(getToleranceCalcFailed(), false);
}

/**
 * Test the following:
 * 1. Motor gets stuck during tolerance calculation
 * 2. After determination of tolerance, an end switch\n
 *    out of tolerance is detected
 */
BOOST_AUTO_TEST_CASE(testDetermineToleranceError){

  // TODO Can be removed (?) when using the TEST_CASE macros, each case becomes its own object
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));

  // Define dummy end switches and calibrate motor
  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);

  _stepperMotor->setEnabled(true);
  performCalibrationForTest();
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == StepperMotorCalibrationMode::FULL);

  // Start tolerance calculation
  _stepperMotor->determineTolerance();
  waitForDetTolerance();

  // Interrupt on first iteration of tolerance calculation
  waitToSetPositiveTargetPos(_stepperMotor->getPositiveEndReferenceInSteps() - 1000);
  _motorControlerDummy->moveTowardsTarget(0.1f);
  _motorControlerDummy->simulateBlockedMotor(true);
  while(!_stepperMotor->isSystemIdle()){}

  // Should have an error now
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), ACTION_ERROR);
  BOOST_CHECK_EQUAL(getToleranceCalculated(), false);
  BOOST_CHECK_EQUAL(getToleranceCalcFailed(), true);

  // Fake out-of-tolerance end switches
  _motorControlerDummy->resetInternalStateToDefaults();

  performCalibrationForTest();
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == StepperMotorCalibrationMode::FULL);

  _stepperMotor->determineTolerance();
  waitForDetTolerance();
  performToleranceCalculationForTest();

  // Dummy axis> [-10000, 10000], stepperMotor axis: [0, 20000],
  // stepperMotor should hit end switch at 19000 and flag an error
  _motorControlerDummy->setPositiveEndSwitch(9000);
  _stepperMotor->setTargetPositionInSteps(20000);

  _stepperMotor->start();
  waitToSetPositiveTargetPos(20000);
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotor->isSystemIdle()){}
  BOOST_CHECK_EQUAL(_stepperMotor->isPositiveReferenceActive(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), CALIBRATION_LOST);
}

BOOST_AUTO_TEST_CASE(testDetermineToleranceStop){

  // TODO Can be removed (?) when using the TEST_CASE macros, each case becomes its own object
  _motorControlerDummy->resetInternalStateToDefaults();
  _stepperMotor.reset(new ChimeraTK::StepperMotorWithReference(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));

  // Define dummy end switches and calibrate motor
  _motorControlerDummy->setPositiveEndSwitch(10000);
  _motorControlerDummy->setNegativeEndSwitch(-10000);
  _stepperMotor->setEnabled(true);

  performCalibrationForTest();
  BOOST_CHECK(_stepperMotor->getCalibrationMode() == StepperMotorCalibrationMode::FULL);

  _stepperMotor->determineTolerance();

  //Trigger stop during tolerance calc
  waitToSetPositiveTargetPos(_stepperMotor->getPositiveEndReferenceInSteps() - 1000);
  _stepperMotor->stop();
  waitForStop();
  _motorControlerDummy->moveTowardsTarget(1);
  waitToSetPositiveTargetPos(_stepperMotor->getPositiveEndReferenceInSteps() + 1000);
  _motorControlerDummy->moveTowardsTarget(1);
  while(!_stepperMotor->isSystemIdle()){}

  // Stop should have led to an error, no tolerance calculation
  BOOST_CHECK_EQUAL(getToleranceCalculated(), false);
  BOOST_CHECK_EQUAL(getToleranceCalcFailed(), true);
  BOOST_CHECK_EQUAL(_stepperMotor->getError(), ACTION_ERROR);
}


BOOST_AUTO_TEST_SUITE_END()

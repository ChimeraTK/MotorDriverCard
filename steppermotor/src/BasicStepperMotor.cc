// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "BasicStepperMotor.h"

#include "DebugLog.h"
#include "MotorControler.h"
#include "MotorDriverCard.h"
#include "MotorDriverCardFactory.h"

#include <ChimeraTK/Exception.h>

#include <cmath>

using LockGuard = boost::lock_guard<boost::mutex>;

namespace ChimeraTK::MotorDriver {

  namespace utility {
    bool checkIfOverflow(int termA, int termB) {
      int signTermA = (termA > 0) - (termA < 0);
      int signTermB = (termB > 0) - (termB < 0);
      if(signTermA * signTermB <= 0) {
        return false;
      }

      if(signTermA > 0) {
        return (termB > std::numeric_limits<int>::max() - termA);
      }
      return (termB < std::numeric_limits<int>::min() - termA);
    }
  } // namespace utility

  /********************************************************************************************************************/

  BasicStepperMotor::BasicStepperMotor(const StepperMotorParameters& parameters)
  : _motorDriverCard(mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(
        parameters.deviceName, parameters.moduleName, parameters.configFileName)),
    _motorController(_motorDriverCard->getMotorControler(parameters.driverId)),
    _stepperMotorUnitsConverter(parameters.motorUnitsConverter),
    _encoderUnitsConverter(parameters.encoderUnitsConverter),
    _targetPositionInSteps(_motorController->getTargetPosition()) {
    _stateMachine = std::make_shared<StateMachine>(*this);
    initStateMachine();
  }

  /********************************************************************************************************************/

  BasicStepperMotor::BasicStepperMotor()
  : _stepperMotorUnitsConverter(std::make_shared<utility::MotorStepsConverterTrivia>()),
    _encoderUnitsConverter(std::make_shared<utility::EncoderStepsConverterTrivia>()) {}

  /********************************************************************************************************************/

  BasicStepperMotor::~BasicStepperMotor() = default;

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::checkNewPosition(int newPositionInSteps) {
    if(_calibrationMode.load() == CalibrationMode::NONE) {
      DEBUG_LOG("BasicStepperMotor: refusing to move to ", newPositionInSteps,
          ": the motor is not calibrated (calibration mode NONE)");
      return ExitStatus::ERR_SYSTEM_NOT_CALIBRATED;
    }
    if(!limitsOK(newPositionInSteps)) {
      DEBUG_LOG(
          "BasicStepperMotor: refusing to move to ", newPositionInSteps, ": the position was rejected by limitsOK");
      return ExitStatus::ERR_INVALID_PARAMETER;
    }
    return ExitStatus::SUCCESS;
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::limitsOK(int newPositionInSteps) {
    if(!_softwareLimitsEnabled) {
      DEBUG_LOG("BasicStepperMotor: position ", newPositionInSteps, " accepted although it may be outside the limits [",
          _minPositionLimitInSteps, ", ", _maxPositionLimitInSteps, "]: software limits are disabled");
      return true;
    }
    if(newPositionInSteps < _minPositionLimitInSteps || newPositionInSteps > _maxPositionLimitInSteps) {
      DEBUG_LOG("BasicStepperMotor: position ", newPositionInSteps, " rejected: outside the enabled software limits [",
          _minPositionLimitInSteps, ", ", _maxPositionLimitInSteps, "]");
      return false;
    }
    DEBUG_LOG("BasicStepperMotor: position ", newPositionInSteps, " accepted: within the enabled software limits [",
        _minPositionLimitInSteps, ", ", _maxPositionLimitInSteps, "]");
    return true;
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::moveRelative(float delta) {
    return moveRelativeInSteps(_stepperMotorUnitsConverter->unitsToSteps(delta));
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::moveRelativeInSteps(int delta) {
    LockGuard guard(_mutex);

    if(motorActive()) {
      DEBUG_LOG("BasicStepperMotor: refusing relative move by ", delta, " steps: the system is in action (state '",
          _stateMachine->getCurrentState()->getName(), "')");
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }

    int newPosition = _motorController->getActualPosition() + delta;
    auto checkResult = checkNewPosition(newPosition);
    if(checkResult != ExitStatus::SUCCESS) {
      return checkResult;
    }

    _targetPositionInSteps = newPosition;

    DEBUG_LOG("BasicStepperMotor: moveRelativeInSteps delta ", delta, " -> target position ", newPosition,
        " (actual position ", _motorController->getActualPosition(), ")");
    _stateMachine->setAndProcessUserEvent(StateMachine::moveEvent);
    return ExitStatus::SUCCESS;
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setTargetPosition(float newPosition) {
    return setTargetPositionInSteps(_stepperMotorUnitsConverter->unitsToSteps(newPosition));
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setTargetPositionInSteps(int newPositionInSteps) {
    LockGuard guard(_mutex);
    auto checkResult = checkNewPosition(newPositionInSteps);
    if(checkResult != ExitStatus::SUCCESS) {
      return checkResult;
    }
    _targetPositionInSteps = newPositionInSteps;
    DEBUG_LOG("BasicStepperMotor: target position set to ", newPositionInSteps, " (actual position ",
        _motorController->getActualPosition(), ")");

    if(_stateMachine->getCurrentState()->getName() == "moving") {
      DEBUG_LOG("BasicStepperMotor: already moving, redirecting the ongoing movement to ", _targetPositionInSteps);
      _motorController->setTargetPosition(_targetPositionInSteps);
    }
    else if(_autostart) {
      DEBUG_LOG("BasicStepperMotor: autostart enabled, starting movement to ", _targetPositionInSteps);
      _stateMachine->setAndProcessUserEvent(StateMachine::moveEvent);
    }
    else {
      DEBUG_LOG("BasicStepperMotor: autostart disabled, target position ", _targetPositionInSteps,
          " stored but no movement started (the motor stays in state '", _stateMachine->getCurrentState()->getName(),
          "')");
    }
    return ExitStatus::SUCCESS;
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::start() {
    LockGuard guard(_mutex);
    _stateMachine->setAndProcessUserEvent(StateMachine::moveEvent);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::stop() {
    LockGuard guard(_mutex);
    _stateMachine->setAndProcessUserEvent(StateMachine::stopEvent);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::emergencyStop() {
    LockGuard guard(_mutex);
    _stateMachine->setAndProcessUserEvent(StateMachine::emergencyStopEvent);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::resetError() {
    LockGuard guard(_mutex);

    DEBUG_LOG("BasicStepperMotor: resetting error mode (was ", toString(_errorMode.load()), "), motor current ",
        _motorController->isMotorCurrentEnabled() ? "enabled" : "disabled");
    _errorMode.exchange(Error::NO_ERROR);

    if(!_motorController->isMotorCurrentEnabled()) {
      _stateMachine->setAndProcessUserEvent(StateMachine::resetToDisableEvent);
    }
    else {
      _stateMachine->setAndProcessUserEvent(StateMachine::resetToIdleEvent);
    }
  }

  /********************************************************************************************************************/

  int BasicStepperMotor::recalculateUnitsInSteps(float units) {
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->unitsToSteps(units);
  }

  /********************************************************************************************************************/

  float BasicStepperMotor::recalculateStepsInUnits(int steps) {
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(steps);
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setSoftwareLimitsEnabled(bool enabled) {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    DEBUG_LOG("BasicStepperMotor: software limits ", enabled ? "enabled" : "disabled");
    _softwareLimitsEnabled = enabled;
    return ExitStatus::SUCCESS;
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::getSoftwareLimitsEnabled() {
    LockGuard guard(_mutex);
    return _softwareLimitsEnabled;
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setMaxPositionLimitInSteps(int maxPosInSteps) {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }

    if(maxPosInSteps <= _minPositionLimitInSteps) {
      return ExitStatus::ERR_INVALID_PARAMETER;
    }
    DEBUG_LOG("BasicStepperMotor: maximum position limit set to ", maxPosInSteps);
    _maxPositionLimitInSteps = maxPosInSteps;
    return ExitStatus::SUCCESS;
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setMaxPositionLimit(float maxPosInUnits) {
    return setMaxPositionLimitInSteps(_stepperMotorUnitsConverter->unitsToSteps(maxPosInUnits));
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setMinPositionLimitInSteps(int minPosInSteps) {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }

    if(minPosInSteps >= _maxPositionLimitInSteps) {
      return ExitStatus::ERR_INVALID_PARAMETER;
    }

    DEBUG_LOG("BasicStepperMotor: minimum position limit set to ", minPosInSteps);
    _minPositionLimitInSteps = minPosInSteps;
    return ExitStatus::SUCCESS;
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setMinPositionLimit(float minPosInUnits) {
    return setMinPositionLimitInSteps(_stepperMotorUnitsConverter->unitsToSteps(minPosInUnits));
  }

  /********************************************************************************************************************/

  int BasicStepperMotor::getMaxPositionLimitInSteps() {
    LockGuard guard(_mutex);
    return _maxPositionLimitInSteps;
  }

  /********************************************************************************************************************/

  float BasicStepperMotor::getMaxPositionLimit() {
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_maxPositionLimitInSteps);
  }

  /********************************************************************************************************************/

  int BasicStepperMotor::getMinPositionLimitInSteps() {
    LockGuard guard(_mutex);
    return _minPositionLimitInSteps;
  }

  /********************************************************************************************************************/

  float BasicStepperMotor::getMinPositionLimit() {
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_minPositionLimitInSteps);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::resetMotorControllerPositions(int actualPositionInSteps) {
    DEBUG_LOG("BasicStepperMotor: resetting motor controller positions to ", actualPositionInSteps);
    bool enable = _motorController->isEnabled();
    _motorController->setEnabled(false);
    _motorController->setActualPosition(actualPositionInSteps);
    _motorController->setTargetPosition(actualPositionInSteps);
    _motorController->setEnabled(enable);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::setActualPositionActions(int actualPositionInSteps) {
    resetMotorControllerPositions(actualPositionInSteps);

    _motorController->setPositiveReferenceSwitchCalibration(std::numeric_limits<int>::max());
    _motorController->setNegativeReferenceSwitchCalibration(std::numeric_limits<int>::min());
    DEBUG_LOG(
        "BasicStepperMotor: actual position set to ", actualPositionInSteps, " by user, calibration mode -> SIMPLE");
    _calibrationMode.exchange(CalibrationMode::SIMPLE);
    _motorController->setCalibrationTime(static_cast<uint32_t>(time(nullptr)));
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setActualPositionInSteps(int actualPositionInSteps) {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    setActualPositionActions(actualPositionInSteps);
    return ExitStatus::SUCCESS;
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setActualPosition(float actualPosition) {
    return setActualPositionInSteps(_stepperMotorUnitsConverter->unitsToSteps(actualPosition));
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::translateAxisActions(int translationInSteps) {
    int actualPosition = _motorController->getActualPosition();

    resetMotorControllerPositions(actualPosition + translationInSteps);
    translateLimits(translationInSteps);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::translateLimits(int translationInSteps) {
    if(utility::checkIfOverflow(_maxPositionLimitInSteps, translationInSteps)) {
      _maxPositionLimitInSteps = std::numeric_limits<int>::max();
    }
    else {
      _maxPositionLimitInSteps = _maxPositionLimitInSteps + translationInSteps;
    }
    if(utility::checkIfOverflow(_minPositionLimitInSteps, translationInSteps)) {
      _minPositionLimitInSteps = std::numeric_limits<int>::min();
    }
    else {
      _minPositionLimitInSteps = _minPositionLimitInSteps + translationInSteps;
    }
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::translateAxisInSteps(int translationInSteps) {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    translateAxisActions(translationInSteps);
    return ExitStatus::SUCCESS;
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::translateAxis(float translationInUnits) {
    return translateAxisInSteps(_stepperMotorUnitsConverter->unitsToSteps(translationInUnits));
  }

  /********************************************************************************************************************/

  int BasicStepperMotor::getCurrentPositionInSteps() {
    LockGuard guard(_mutex);
    return (_motorController->getActualPosition());
  }

  /********************************************************************************************************************/

  float BasicStepperMotor::getCurrentPosition() {
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_motorController->getActualPosition());
  }

  /********************************************************************************************************************/

  double BasicStepperMotor::getEncoderPosition() {
    LockGuard guard(_mutex);
    return _encoderUnitsConverter->stepsToUnits(
        static_cast<int>(_motorController->getDecoderPosition()) + _encoderPositionOffset);
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setActualEncoderPosition(double referencePosition) {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }

    _encoderPositionOffset = _encoderUnitsConverter->unitsToSteps(referencePosition) -
        static_cast<int>(_motorController->getDecoderPosition());
    DEBUG_LOG("BasicStepperMotor: encoder position offset set to ", _encoderPositionOffset);
    return ExitStatus::SUCCESS;
  }

  /********************************************************************************************************************/

  int BasicStepperMotor::getTargetPositionInSteps() {
    LockGuard guard(_mutex);
    return _motorController->getTargetPosition();
  }

  /********************************************************************************************************************/

  float BasicStepperMotor::getTargetPosition() {
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_motorController->getTargetPosition());
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setStepperMotorUnitsConverter(
      std::shared_ptr<utility::MotorStepsConverter> stepperMotorUnitsConverter) {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }

    if(stepperMotorUnitsConverter == nullptr) {
      return ExitStatus::ERR_INVALID_PARAMETER;
    }
    _stepperMotorUnitsConverter = stepperMotorUnitsConverter;
    DEBUG_LOG("BasicStepperMotor: custom units converter installed");
    return ExitStatus::SUCCESS;
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setStepperMotorUnitsConverterToDefault() {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    DEBUG_LOG("BasicStepperMotor: units converter reset to default (trivial)");
    _stepperMotorUnitsConverter = std::make_unique<utility::MotorStepsConverterTrivia>();
    return ExitStatus::SUCCESS;
  }

  /********************************************************************************************************************/

  std::string BasicStepperMotor::getState() {
    LockGuard guard(_mutex);
    return _stateMachine->getCurrentState()->getName();
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::isSystemIdle() {
    LockGuard guard(_mutex);
    // return !motorActive();
    // FIXME Return to this
    std::string state = _stateMachine->getCurrentState()->getName();
    return state == "idle" || state == "disabled";
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::waitForIdle() {
    while(true) {
      usleep(100);
      if(isSystemIdle()) {
        break;
      }
    }
  }

  /********************************************************************************************************************/

  Error BasicStepperMotor::getError() {
    LockGuard guard(_mutex);
    return _errorMode.load();
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::isCalibrated() {
    return _calibrationMode.load() != CalibrationMode::NONE;
  }

  /********************************************************************************************************************/

  uint32_t BasicStepperMotor::getCalibrationTime() {
    LockGuard guard(_mutex);
    return _motorController->getCalibrationTime();
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::setEnabled(bool enable) {
    LockGuard guard(_mutex);
    if(enable) {
      _stateMachine->setAndProcessUserEvent(StateMachine::enableEvent);
    }
    else {
      _stateMachine->setAndProcessUserEvent(StateMachine::disableEvent);
    }
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::getEnabled() {
    LockGuard guard(_mutex);
    // Note:  For the old firmware version isEndSwitchPowerEnabled always returns
    //       false (End switch power is not applicable in this scenario).
    return (_motorController->isEndSwitchPowerEnabled() || _motorController->isMotorCurrentEnabled());
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::setAutostart(bool autostart) {
    LockGuard guard(_mutex);
    DEBUG_LOG("BasicStepperMotor: autostart ", autostart ? "enabled" : "disabled");
    _autostart = autostart;
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::getAutostart() {
    LockGuard guard(_mutex);
    return _autostart;
  }

  /********************************************************************************************************************/

  double BasicStepperMotor::getMaxSpeedCapability() {
    LockGuard guard(_mutex);
    return _motorController->getMaxSpeedCapability();
  }

  /********************************************************************************************************************/

  double BasicStepperMotor::getSafeCurrentLimit() {
    LockGuard guard(_mutex);

    return _motorController->getMaxCurrentLimit();
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setUserCurrentLimit(double currentInAmps) {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    _motorController->setUserCurrentLimit(currentInAmps);
    return ExitStatus::SUCCESS;
  }

  /********************************************************************************************************************/

  double BasicStepperMotor::getUserCurrentLimit() {
    LockGuard guard(_mutex);
    return _motorController->getUserCurrentLimit();
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::setUserSpeedLimit(double speedInUstepsPerSec) {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    _motorController->setUserSpeedLimit(speedInUstepsPerSec);
    return ExitStatus::SUCCESS;
  }

  /********************************************************************************************************************/

  double BasicStepperMotor::getUserSpeedLimit() {
    LockGuard guard(_mutex);
    return _motorController->getUserSpeedLimit();
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::isMoving() {
    LockGuard guard(_mutex);
    return _motorController->isMotorMoving();
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::enableFullStepping(bool enable) {
    LockGuard guard(_mutex);
    DEBUG_LOG("BasicStepperMotor: full stepping ", enable ? "enabled" : "disabled");
    _motorController->enableFullStepping(enable);
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::isFullStepping() {
    LockGuard guard(_mutex);
    return _motorController->isFullStepping();
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::motorActive() {
    return _stateMachine->getCurrentState()->getName() == "moving";
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::verifyMoveAction() {
    return _motorController->getTargetPosition() == _motorController->getActualPosition();
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::initStateMachine() {
    LockGuard guard(_mutex);
    if(_stateMachine->getCurrentState()->getName() == "initState") {
      _stateMachine->setAndProcessUserEvent(StateMachine::initialEvent);
    }
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::hasHWReferenceSwitches() {
    return false;
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::calibrate() {
    throw ChimeraTK::logic_error("This routine is not available for the BasicStepperMotor (current state '" +
        _stateMachine->getCurrentState()->getName() + "')");
  }

  /********************************************************************************************************************/

  ExitStatus BasicStepperMotor::determineTolerance() {
    throw ChimeraTK::logic_error("This routine is not available for the BasicStepperMotor (current state '" +
        _stateMachine->getCurrentState()->getName() + "')");
  }

  /********************************************************************************************************************/

  float BasicStepperMotor::getPositiveEndReference() {
    throw ChimeraTK::logic_error("This routine is not available for the BasicStepperMotor (current state '" +
        _stateMachine->getCurrentState()->getName() + "')");
  }

  /********************************************************************************************************************/

  int BasicStepperMotor::getPositiveEndReferenceInSteps() {
    throw ChimeraTK::logic_error("This routine is not available for the BasicStepperMotor (current state '" +
        _stateMachine->getCurrentState()->getName() + "')");
  }

  /********************************************************************************************************************/

  float BasicStepperMotor::getNegativeEndReference() {
    throw ChimeraTK::logic_error("This routine is not available for the BasicStepperMotor (current state '" +
        _stateMachine->getCurrentState()->getName() + "')");
  }

  /********************************************************************************************************************/

  int BasicStepperMotor::getNegativeEndReferenceInSteps() {
    throw ChimeraTK::logic_error("This routine is not available for the BasicStepperMotor (current state '" +
        _stateMachine->getCurrentState()->getName() + "')");
  }

  /********************************************************************************************************************/

  float BasicStepperMotor::getTolerancePositiveEndSwitch() {
    throw ChimeraTK::logic_error("This routine is not available for the BasicStepperMotor (current state '" +
        _stateMachine->getCurrentState()->getName() + "')");
  }

  /********************************************************************************************************************/

  float BasicStepperMotor::getToleranceNegativeEndSwitch() {
    throw ChimeraTK::logic_error("This routine is not available for the BasicStepperMotor (current state '" +
        _stateMachine->getCurrentState()->getName() + "')");
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::isPositiveReferenceActive() {
    throw ChimeraTK::logic_error("This routine is not available for the BasicStepperMotor (current state '" +
        _stateMachine->getCurrentState()->getName() + "')");
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::isNegativeReferenceActive() {
    throw ChimeraTK::logic_error("This routine is not available for the BasicStepperMotor (current state '" +
        _stateMachine->getCurrentState()->getName() + "')");
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::isPositiveEndSwitchEnabled() {
    throw ChimeraTK::logic_error("This routine is not available for the BasicStepperMotor (current state '" +
        _stateMachine->getCurrentState()->getName() + "')");
  }

  /********************************************************************************************************************/

  bool BasicStepperMotor::isNegativeEndSwitchEnabled() {
    throw ChimeraTK::logic_error("This routine is not available for the BasicStepperMotor (current state '" +
        _stateMachine->getCurrentState()->getName() + "')");
  }

  /********************************************************************************************************************/

  CalibrationMode BasicStepperMotor::getCalibrationMode() {
    return _calibrationMode.load();
  }

  /********************************************************************************************************************/

  unsigned int BasicStepperMotor::getEncoderReadoutMode() {
    return _motorController->getDecoderReadoutMode();
  }

} // namespace ChimeraTK::MotorDriver

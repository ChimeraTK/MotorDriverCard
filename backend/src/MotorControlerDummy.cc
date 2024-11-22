#include "MotorControlerDummy.h"

#include <ChimeraTK/Exception.h>

#include <iostream>

using LockGuard = std::lock_guard<std::mutex>;
using UniqueLog = std::unique_lock<std::mutex>;

namespace mtca4u {

  MotorControlerDummy::MotorControlerDummy(unsigned int id) : MotorControler(), _id(id) {
    std::cerr << "Creating MotorControlerDummy for id " << id << ": " << std::hex << std::intptr_t(this) << std::endl;
  }

  void MotorControlerDummy::setPositiveEndSwitch(int endSwitchPos) {
    LockGuard guard(_motorControllerDummyMutex);
    _positiveEndSwitchPosition = endSwitchPos;
  }

  int MotorControlerDummy::getPositiveEndSwitch() {
    LockGuard guard(_motorControllerDummyMutex);
    return _positiveEndSwitchPosition;
  }

  void MotorControlerDummy::setNegativeEndSwitch(int endSwitchNeg) {
    LockGuard guard(_motorControllerDummyMutex);
    _negativeEndSwitchPosition = endSwitchNeg;
  }

  int MotorControlerDummy::getNegativeEndSwitch() {
    LockGuard guard(_motorControllerDummyMutex);
    return _negativeEndSwitchPosition;
  }

  unsigned int MotorControlerDummy::getID() {
    LockGuard guard(_motorControllerDummyMutex);
    return _id;
  }

  int MotorControlerDummy::getActualPosition() {
    LockGuard guard(_motorControllerDummyMutex);
    return _currentPosition;
  }

  int MotorControlerDummy::getActualVelocity() {
    LockGuard guard(_motorControllerDummyMutex);
    // the velocity is only not zero if the motor is actually moving
    // FIXME: or if the motor is stepping?
    bool motorMoving = (_motorCurrentEnabled && isStepping());
    if(motorMoving) {
      return (_targetPosition < _currentPosition ? -25 : 25);
    }

    return 0;
  }

  void MotorControlerDummy::enableFullStepping(bool enable) {
    _isFullStepping = enable;
  }

  bool MotorControlerDummy::isFullStepping() {
    return _isFullStepping;
  }

  bool MotorControlerDummy::isMotorMoving() {
    LockGuard guard(_motorControllerDummyMutex);
    return (_motorCurrentEnabled && isStepping());
  }

  bool MotorControlerDummy::isStepping() {
    // if the motor is at the target position the motor is not stepping
    if(_targetPosition == _currentPosition) {
      return false;
    }

    if(_blockMotor) {
      return false;
    }
    // FIXME: check for errors. To be implemented....

    // Just checking the end switches is not enough. One has to be
    // able to move away into one direction.
    if(_targetPosition > _currentPosition) {
      // The motor should be moving in positive position
      if(isPositiveEndSwitchActive()) {
        return false;
      }
    }
    else { // negative direction
      if(isNegativeEndSwitchActive()) {
        return false;
      }
    }

    return true;
  }

  unsigned int MotorControlerDummy::getActualAcceleration() {
    // This motor accelerates instantaneously. Either it is moving or not.
    // (not very realistic, but ok for a test)
    return 0;
  }

  unsigned int MotorControlerDummy::getMicroStepCount() {
    LockGuard guard(_motorControllerDummyMutex);
    return ((_currentPosition * _userMicroStepSize) - 1) & 0x3FF;
  }

  DriverStatusData MotorControlerDummy::getStatus() {
    LockGuard guard(_motorControllerDummyMutex);
    DriverStatusData statusData;
    bool motorMoving = (_motorCurrentEnabled && isStepping());
    statusData.setStandstillIndicator(!motorMoving);
    return statusData;
  }

  unsigned int MotorControlerDummy::getDecoderReadoutMode() {
    LockGuard guard(_motorControllerDummyMutex);
    // I think this is absolute postion
    return DecoderReadoutMode::HEIDENHAIN;
  }

  unsigned int MotorControlerDummy::getDecoderPosition() {
    LockGuard guard(_motorControllerDummyMutex);
    // make the negative end switch "decoder 0"
    return _absolutePosition - _negativeEndSwitchPosition;
  }

  void MotorControlerDummy::setActualVelocity(int /*stepsPerFIXME*/) {
    throw ChimeraTK::logic_error("MotorControlerDummy::setActualVelocity() is not implemented yet!");
  }

  void MotorControlerDummy::setActualAcceleration(unsigned int /*stepsPerSquareFIXME*/) {
    throw ChimeraTK::logic_error("MotorControlerDummy::setActualAcceleration() is not implemented yet!");
  }

  void MotorControlerDummy::setMicroStepCount(unsigned int /*microStepCount*/) {
    throw ChimeraTK::logic_error("MotorControlerDummy::setMicroStepCount() is not implemented yet!");
  }

  void MotorControlerDummy::setEnabled(bool enable) {
    LockGuard guard(_motorControllerDummyMutex);
    _motorCurrentEnabled = enable;
  }

  void MotorControlerDummy::setDecoderReadoutMode(unsigned int /*decoderReadoutMode*/) {
    throw ChimeraTK::logic_error("MotorControlerDummy::setDecoderReadoutMode(): The Decoder readout mode "
                                 "cannot be changed for the dummy motor!");
  }

  bool MotorControlerDummy::isEnabled() {
    LockGuard guard(_motorControllerDummyMutex);
    return _motorCurrentEnabled;
  }

  void MotorControlerDummy::setCalibrationTime(uint32_t calibrationTime) {
    LockGuard guard(_motorControllerDummyMutex);
    _calibrationTime = calibrationTime;
  }

  uint32_t MotorControlerDummy::getCalibrationTime() {
    LockGuard guard(_motorControllerDummyMutex);
    return _calibrationTime;
  }

  void MotorControlerDummy::setPositiveReferenceSwitchCalibration(int calibratedPosition) {
    LockGuard guard(_motorControllerDummyMutex);
    _positiveEndSwitchPosition = calibratedPosition;
  }

  int MotorControlerDummy::getPositiveReferenceSwitchCalibration() {
    LockGuard guard(_motorControllerDummyMutex);
    return _positiveEndSwitchPosition;
  }

  void MotorControlerDummy::setNegativeReferenceSwitchCalibration(int calibratedPosition) {
    LockGuard guard(_motorControllerDummyMutex);
    _negativeEndSwitchPosition = calibratedPosition;
  }

  int MotorControlerDummy::getNegativeReferenceSwitchCalibration() {
    LockGuard guard(_motorControllerDummyMutex);
    return _negativeEndSwitchPosition;
  }

  MotorReferenceSwitchData MotorControlerDummy::getReferenceSwitchData() {
    LockGuard guard(_motorControllerDummyMutex);
    MotorReferenceSwitchData motorReferenceSwitchData;
    motorReferenceSwitchData.setPositiveSwitchEnabled(_positiveEndSwitchEnabled);
    motorReferenceSwitchData.setNegativeSwitchEnabled(_negativeEndSwitchEnabled);

    motorReferenceSwitchData.setPositiveSwitchActive(isPositiveEndSwitchActive());
    motorReferenceSwitchData.setNegativeSwitchActive(isNegativeEndSwitchActive());

    return motorReferenceSwitchData;
  }

  void MotorControlerDummy::setPositiveReferenceSwitchEnabled(bool enableStatus) {
    LockGuard guard(_motorControllerDummyMutex);
    _positiveEndSwitchEnabled = enableStatus;
  }

  void MotorControlerDummy::setNegativeReferenceSwitchEnabled(bool enableStatus) {
    LockGuard guard(_motorControllerDummyMutex);
    _negativeEndSwitchEnabled = enableStatus;
  }

  void MotorControlerDummy::setActualPosition(int steps) {
    LockGuard guard(_motorControllerDummyMutex);
    _currentPosition = steps;
  }

  void MotorControlerDummy::setTargetPosition(int steps) {
    LockGuard guard(_motorControllerDummyMutex);
    if(_isFullStepping) {
      roundToNextFullStep(steps);
    }
    _targetPosition = steps;
  }

  void MotorControlerDummy::roundToNextFullStep(int& targetPosition) {
    int delta = targetPosition - _currentPosition;
    int deltaMicroStep = delta * _userMicroStepSize;
    unsigned int actualMicroStepCount = ((_currentPosition * _userMicroStepSize) - 1) & 0x3FF;
    unsigned int newActualMicroStepCount = (actualMicroStepCount + deltaMicroStep) & 0x3FF;
    unsigned int distanceToPreviousFullStep = (newActualMicroStepCount + 1) % 256;
    if(distanceToPreviousFullStep < 128) {
      targetPosition = targetPosition - distanceToPreviousFullStep / _userMicroStepSize;
    }
    else {
      targetPosition = targetPosition + (256 - distanceToPreviousFullStep) / _userMicroStepSize;
    }
  }

  int MotorControlerDummy::getTargetPosition() {
    LockGuard guard(_motorControllerDummyMutex);
    return _targetPosition;
  }

  void MotorControlerDummy::setMinimumVelocity(unsigned int /*stepsPerFIXME*/) {
    throw ChimeraTK::logic_error("MotorControlerDummy::setMinimumVelocity() is not implemented yet!");
  }

  unsigned int MotorControlerDummy::getMinimumVelocity() {
    throw ChimeraTK::logic_error("MotorControlerDummy::getMinimumVelocity() is not implemented yet!");
  }

  void MotorControlerDummy::setMaximumVelocity(unsigned int /*stepsPerFIXME*/) {
    throw ChimeraTK::logic_error("MotorControlerDummy::setMaximumVelocity() is not implemented yet!");
  }

  unsigned int MotorControlerDummy::getMaximumVelocity() {
    throw ChimeraTK::logic_error("MotorControlerDummy::getMaximumVelocity() is not implemented yet!");
  }

  void MotorControlerDummy::setTargetVelocity(int /*stepsPerFIXME*/) {
    throw ChimeraTK::logic_error("MotorControlerDummy::setTargetVelocity() is not implemented yet!");
  }

  int MotorControlerDummy::getTargetVelocity() {
    throw ChimeraTK::logic_error("MotorControlerDummy::getTargetVelocity() is not implemented yet!");
  }

  void MotorControlerDummy::setMaximumAcceleration(unsigned int /*stepsPerSquareFIXME*/) {
    throw ChimeraTK::logic_error("MotorControlerDummy::setMaximumAcceleration() is not implemented yet!");
  }

  unsigned int MotorControlerDummy::getMaximumAcceleration() {
    throw ChimeraTK::logic_error("MotorControlerDummy::getMaximumAcceleration() is not implemented yet!");
  }

  void MotorControlerDummy::setPositionTolerance(unsigned int /*steps*/) {
    throw ChimeraTK::logic_error("MotorControlerDummy::setPositionTolerance() is not implemented yet!");
  }

  unsigned int MotorControlerDummy::getPositionTolerance() {
    throw ChimeraTK::logic_error("MotorControlerDummy::getPositionTolerance() is not implemented yet!");
  }

  void MotorControlerDummy::setPositionLatched(unsigned int /*steps*/) {
    throw ChimeraTK::logic_error("MotorControlerDummy::setPositionLatched() is not implemented yet!");
  }

  unsigned int MotorControlerDummy::getPositionLatched() {
    throw ChimeraTK::logic_error("MotorControlerDummy:getPositionLatched() is not implemented yet!");
  }

  bool MotorControlerDummy::targetPositionReached() {
    LockGuard guard(_motorControllerDummyMutex);
    return (_currentPosition == _targetPosition);
  }

  unsigned int MotorControlerDummy::getReferenceSwitchBit() {
    LockGuard guard(_motorControllerDummyMutex);
    return static_cast<unsigned int>(isPositiveEndSwitchActive() || isNegativeEndSwitchActive());
  }

  bool MotorControlerDummy::isPositiveEndSwitchActive() {
    if(_bothEndSwitchesAlwaysOn) return true;

    return _positiveEndSwitchEnabled && (_absolutePosition >= _positiveEndSwitchPosition);
  }

  double MotorControlerDummy::getMaxSpeedCapability() {
    throw ChimeraTK::logic_error("MotorControlerDummy::getMaxSpeedCapability() is not implemented yet!");
  }

  double MotorControlerDummy::setUserSpeedLimit(double microStepsPerSecond) {
    LockGuard guard(_motorControllerDummyMutex);
    _userSpeedLimit = microStepsPerSecond;
    return _userSpeedLimit;
  }

  double MotorControlerDummy::getUserSpeedLimit() {
    return _userSpeedLimit;
  }

  double MotorControlerDummy::setUserCurrentLimit(double currentLimit) {
    (void)currentLimit; // To suppress parameter unused warning.
    throw ChimeraTK::logic_error("MotorControlerDummy::setUserCurrentLimit() is not implemented yet!");
  }
  double MotorControlerDummy::getUserCurrentLimit() {
    throw ChimeraTK::logic_error("MotorControlerDummy::getUserCurrentLimit() is not implemented yet!");
  }
  double MotorControlerDummy::getMaxCurrentLimit() {
    throw ChimeraTK::logic_error("MotorControlerDummy::getMaxCurrentLimit() is not implemented yet!");
  }

  void MotorControlerDummy::resetInternalStateToDefaults() {
    LockGuard guard(_motorControllerDummyMutex);
    _absolutePosition = 0;
    _targetPosition = 0;
    _currentPosition = 0;
    _calibrationTime = 0;
    _positiveEndSwitchEnabled = true;
    _negativeEndSwitchEnabled = true;
    _bothEndSwitchesAlwaysOn = false;
    _blockMotor = false;
  }

  void MotorControlerDummy::setMotorCurrentEnabled(bool enable) {
    LockGuard guard(_motorControllerDummyMutex);
    _motorCurrentEnabled = enable;
  }

  bool MotorControlerDummy::isMotorCurrentEnabled() {
    LockGuard guard(_motorControllerDummyMutex);
    return _motorCurrentEnabled;
  }

  void MotorControlerDummy::setEndSwitchPowerEnabled(bool enable) {
    LockGuard guard(_motorControllerDummyMutex);
    _endSwitchPowerEnabled = enable;
  }
  bool MotorControlerDummy::isEndSwitchPowerEnabled() {
    LockGuard guard(_motorControllerDummyMutex);
    return _endSwitchPowerEnabled;
  }

  bool MotorControlerDummy::isNegativeEndSwitchActive() {
    if(_bothEndSwitchesAlwaysOn) return true;

    return _negativeEndSwitchEnabled && (_absolutePosition <= _negativeEndSwitchPosition);
  }

  void MotorControlerDummy::moveTowardsTarget(
      float fraction, bool block, bool bothEndSwitchesAlwaysOn, bool zeroPositions) {
    LockGuard guard(_motorControllerDummyMutex);
    if(zeroPositions) {
      _absolutePosition = 0;
      _targetPosition = 0;
      _currentPosition = 0;
      _positiveEndSwitchEnabled = true;
      _negativeEndSwitchEnabled = true;
    }

    _bothEndSwitchesAlwaysOn = bothEndSwitchesAlwaysOn;

    _blockMotor = block;
    if(_blockMotor) {
      return;
    }

    // check if the motor is should to be moving if it was enabled
    // otherwise return immediately
    if(!isStepping()) {
      return;
    }

    // range check for the input parameter 'fraction', just force it to 0..1
    if(fraction <= 0) {
      return;
    }
    if(fraction > 1) {
      fraction = 1.;
    }

    // calculate the new abolute target position for this move operation
    int relativeSteps = (_targetPosition - _currentPosition) * fraction;
    int absoluteTargetInThisMove = _absolutePosition + relativeSteps;
    int targetInThisMove = _currentPosition + relativeSteps;

    if(!_motorCurrentEnabled) {
      // the motor is stepping to the target position, but the actual positon
      // does not change. Thus no end switch will be hit.
      _currentPosition = targetInThisMove;
      return;
    }

    // ok, so the motor is really moving.

    // check whether an end switch would be hit
    if(_positiveEndSwitchEnabled && (absoluteTargetInThisMove > _positiveEndSwitchPosition)) {
      absoluteTargetInThisMove = _positiveEndSwitchPosition;
      // also the current positon stops if an end switch is reached
      int stepsToEndSwitch = _positiveEndSwitchPosition - _absolutePosition;
      targetInThisMove = _currentPosition + stepsToEndSwitch;
    }
    if(_negativeEndSwitchEnabled && (absoluteTargetInThisMove < _negativeEndSwitchPosition)) {
      absoluteTargetInThisMove = _negativeEndSwitchPosition;
      // also the current positon stops if an end switch is reached
      int stepsToEndSwitch = _negativeEndSwitchPosition - _absolutePosition;
      targetInThisMove = _currentPosition + stepsToEndSwitch;
    }

    // finally 'move' the motor
    _absolutePosition = absoluteTargetInThisMove;
    _currentPosition = targetInThisMove;
  }

  void MotorControlerDummy::simulateBlockedMotor(bool state) {
    LockGuard guard(_motorControllerDummyMutex);
    _blockMotor = state;
  }
} // namespace mtca4u

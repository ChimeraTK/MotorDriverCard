// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "MotorControllerDummy.h"

#include <ChimeraTK/Exception.h>

#include <iostream>

using LockGuard = std::lock_guard<std::mutex>;
using UniqueLog = std::unique_lock<std::mutex>;

namespace ChimeraTK::MotorDriver {

  MotorControllerDummy::MotorControllerDummy(unsigned int id) : MotorController(), _id(id) {}

  void MotorControllerDummy::setPositiveEndSwitch(int endSwitchPos) {
    LockGuard guard(_motorControllerDummyMutex);
    _positiveEndSwitchHardwarePosition = endSwitchPos;
  }

  int MotorControllerDummy::getPositiveEndSwitch() {
    LockGuard guard(_motorControllerDummyMutex);
    return _positiveEndSwitchHardwarePosition;
  }

  void MotorControllerDummy::setNegativeEndSwitch(int endSwitchNeg) {
    LockGuard guard(_motorControllerDummyMutex);
    _negativeEndSwitchHardwarePosition = endSwitchNeg;
  }

  int MotorControllerDummy::getNegativeEndSwitch() {
    LockGuard guard(_motorControllerDummyMutex);
    return _negativeEndSwitchHardwarePosition;
  }

  unsigned int MotorControllerDummy::getID() {
    LockGuard guard(_motorControllerDummyMutex);
    return _id;
  }

  int MotorControllerDummy::getActualPosition() {
    LockGuard guard(_motorControllerDummyMutex);
    return _currentPosition;
  }

  int MotorControllerDummy::getActualVelocity() {
    LockGuard guard(_motorControllerDummyMutex);
    // the velocity is only not zero if the motor is actually moving
    // FIXME: or if the motor is stepping?
    bool motorMoving = (_motorCurrentEnabled && isStepping());
    if(motorMoving) {
      return (_targetPosition < _currentPosition ? -25 : 25);
    }

    return 0;
  }

  void MotorControllerDummy::enableFullStepping(bool enable) {
    _isFullStepping = enable;
  }

  bool MotorControllerDummy::isFullStepping() {
    return _isFullStepping;
  }

  bool MotorControllerDummy::isMotorMoving() {
    LockGuard guard(_motorControllerDummyMutex);
    return (_motorCurrentEnabled && isStepping());
  }

  bool MotorControllerDummy::isStepping() {
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

  unsigned int MotorControllerDummy::getActualAcceleration() {
    // This motor accelerates instantaneously. Either it is moving or not.
    // (not very realistic, but ok for a test)
    return 0;
  }

  unsigned int MotorControllerDummy::getMicroStepCount() {
    LockGuard guard(_motorControllerDummyMutex);
    return static_cast<unsigned int>((_currentPosition * _userMicroStepSize) - 1) & 0x3FFU;
  }

  DriverStatusData MotorControllerDummy::getStatus() {
    LockGuard guard(_motorControllerDummyMutex);
    DriverStatusData statusData;
    bool motorMoving = (_motorCurrentEnabled && isStepping());
    statusData.setStandstillIndicator(!motorMoving);
    return statusData;
  }

  unsigned int MotorControllerDummy::getDecoderReadoutMode() {
    LockGuard guard(_motorControllerDummyMutex);
    // I think this is absolute postion
    return DecoderReadoutMode::HEIDENHAIN;
  }

  unsigned int MotorControllerDummy::getDecoderPosition() {
    LockGuard guard(_motorControllerDummyMutex);
    // make the negative end switch "decoder 0"
    return _hardwarePosition - _negativeEndSwitchHardwarePosition;
  }

  void MotorControllerDummy::setActualVelocity(int /*stepsPerFIXME*/) {
    throw ChimeraTK::logic_error("MotorControllerDummy::setActualVelocity() is not implemented yet!");
  }

  void MotorControllerDummy::setActualAcceleration(unsigned int /*stepsPerSquareFIXME*/) {
    throw ChimeraTK::logic_error("MotorControllerDummy::setActualAcceleration() is not implemented yet!");
  }

  void MotorControllerDummy::setMicroStepCount(unsigned int /*microStepCount*/) {
    throw ChimeraTK::logic_error("MotorControllerDummy::setMicroStepCount() is not implemented yet!");
  }

  void MotorControllerDummy::setEnabled(bool enable) {
    LockGuard guard(_motorControllerDummyMutex);
    _motorCurrentEnabled = enable;
  }

  void MotorControllerDummy::setDecoderReadoutMode(unsigned int /*decoderReadoutMode*/) {
    throw ChimeraTK::logic_error("MotorControllerDummy::setDecoderReadoutMode(): The Decoder readout mode "
                                 "cannot be changed for the dummy motor!");
  }

  bool MotorControllerDummy::isEnabled() {
    LockGuard guard(_motorControllerDummyMutex);
    return _motorCurrentEnabled;
  }

  void MotorControllerDummy::setCalibrationTime(uint32_t calibrationTime) {
    LockGuard guard(_motorControllerDummyMutex);
    _calibrationTime = calibrationTime;
  }

  uint32_t MotorControllerDummy::getCalibrationTime() {
    LockGuard guard(_motorControllerDummyMutex);
    return _calibrationTime;
  }

  void MotorControllerDummy::setPositiveReferenceSwitchCalibration(int calibratedPosition) {
    LockGuard guard(_motorControllerDummyMutex);
    _positiveEndSwitchPosition = calibratedPosition;
  }

  int MotorControllerDummy::getPositiveReferenceSwitchCalibration() {
    LockGuard guard(_motorControllerDummyMutex);
    return _positiveEndSwitchPosition;
  }

  void MotorControllerDummy::setNegativeReferenceSwitchCalibration(int calibratedPosition) {
    LockGuard guard(_motorControllerDummyMutex);
    _negativeEndSwitchPosition = calibratedPosition;
  }

  int MotorControllerDummy::getNegativeReferenceSwitchCalibration() {
    LockGuard guard(_motorControllerDummyMutex);
    return _negativeEndSwitchPosition;
  }

  MotorReferenceSwitchData MotorControllerDummy::getReferenceSwitchData() {
    LockGuard guard(_motorControllerDummyMutex);
    MotorReferenceSwitchData motorReferenceSwitchData;
    motorReferenceSwitchData.setPositiveSwitchEnabled(_positiveEndSwitchEnabled);
    motorReferenceSwitchData.setNegativeSwitchEnabled(_negativeEndSwitchEnabled);

    motorReferenceSwitchData.setPositiveSwitchActive(isPositiveEndSwitchActive());
    motorReferenceSwitchData.setNegativeSwitchActive(isNegativeEndSwitchActive());

    return motorReferenceSwitchData;
  }

  void MotorControllerDummy::setPositiveReferenceSwitchEnabled(bool enableStatus) {
    LockGuard guard(_motorControllerDummyMutex);
    _positiveEndSwitchEnabled = enableStatus;
  }

  void MotorControllerDummy::setNegativeReferenceSwitchEnabled(bool enableStatus) {
    LockGuard guard(_motorControllerDummyMutex);
    _negativeEndSwitchEnabled = enableStatus;
  }

  void MotorControllerDummy::setActualPosition(int steps) {
    LockGuard guard(_motorControllerDummyMutex);
    _currentPosition = steps;
  }

  void MotorControllerDummy::setTargetPosition(int steps) {
    LockGuard guard(_motorControllerDummyMutex);
    if(_isFullStepping) {
      roundToNextFullStep(steps);
    }
    _targetPosition = steps;
  }

  void MotorControllerDummy::roundToNextFullStep(int& targetPosition) const {
    int delta = targetPosition - _currentPosition;
    int deltaMicroStep = static_cast<int>(delta * _userMicroStepSize);
    unsigned int actualMicroStepCount = static_cast<unsigned int>((_currentPosition * _userMicroStepSize) - 1) & 0x3FFU;
    unsigned int newActualMicroStepCount = static_cast<unsigned int>(actualMicroStepCount + deltaMicroStep) & 0x3FFU;
    unsigned int distanceToPreviousFullStep = (newActualMicroStepCount + 1) % 256;
    if(distanceToPreviousFullStep < 128) {
      targetPosition = targetPosition - static_cast<int>(distanceToPreviousFullStep / _userMicroStepSize);
    }
    else {
      targetPosition = targetPosition + static_cast<int>((256 - distanceToPreviousFullStep) / _userMicroStepSize);
    }
  }

  int MotorControllerDummy::getTargetPosition() {
    LockGuard guard(_motorControllerDummyMutex);
    return _targetPosition;
  }

  void MotorControllerDummy::setMinimumVelocity(unsigned int /*stepsPerFIXME*/) {
    throw ChimeraTK::logic_error("MotorControllerDummy::setMinimumVelocity() is not implemented yet!");
  }

  unsigned int MotorControllerDummy::getMinimumVelocity() {
    throw ChimeraTK::logic_error("MotorControllerDummy::getMinimumVelocity() is not implemented yet!");
  }

  void MotorControllerDummy::setMaximumVelocity(unsigned int /*stepsPerFIXME*/) {
    throw ChimeraTK::logic_error("MotorControllerDummy::setMaximumVelocity() is not implemented yet!");
  }

  unsigned int MotorControllerDummy::getMaximumVelocity() {
    throw ChimeraTK::logic_error("MotorControllerDummy::getMaximumVelocity() is not implemented yet!");
  }

  void MotorControllerDummy::setTargetVelocity(int /*stepsPerFIXME*/) {
    throw ChimeraTK::logic_error("MotorControllerDummy::setTargetVelocity() is not implemented yet!");
  }

  int MotorControllerDummy::getTargetVelocity() {
    throw ChimeraTK::logic_error("MotorControllerDummy::getTargetVelocity() is not implemented yet!");
  }

  void MotorControllerDummy::setMaximumAcceleration(unsigned int /*stepsPerSquareFIXME*/) {
    throw ChimeraTK::logic_error("MotorControllerDummy::setMaximumAcceleration() is not implemented yet!");
  }

  unsigned int MotorControllerDummy::getMaximumAcceleration() {
    throw ChimeraTK::logic_error("MotorControllerDummy::getMaximumAcceleration() is not implemented yet!");
  }

  void MotorControllerDummy::setPositionTolerance(unsigned int /*steps*/) {
    throw ChimeraTK::logic_error("MotorControllerDummy::setPositionTolerance() is not implemented yet!");
  }

  unsigned int MotorControllerDummy::getPositionTolerance() {
    throw ChimeraTK::logic_error("MotorControllerDummy::getPositionTolerance() is not implemented yet!");
  }

  void MotorControllerDummy::setPositionLatched(unsigned int /*steps*/) {
    throw ChimeraTK::logic_error("MotorControllerDummy::setPositionLatched() is not implemented yet!");
  }

  unsigned int MotorControllerDummy::getPositionLatched() {
    throw ChimeraTK::logic_error("MotorControllerDummy:getPositionLatched() is not implemented yet!");
  }

  bool MotorControllerDummy::targetPositionReached() {
    LockGuard guard(_motorControllerDummyMutex);
    return (_currentPosition == _targetPosition);
  }

  unsigned int MotorControllerDummy::getReferenceSwitchBit() {
    LockGuard guard(_motorControllerDummyMutex);
    return static_cast<unsigned int>(isPositiveEndSwitchActive() || isNegativeEndSwitchActive());
  }

  bool MotorControllerDummy::isPositiveEndSwitchActive() const {
    if(_bothEndSwitchesAlwaysOn) {
      return true;
    }

    return _positiveEndSwitchEnabled && (_hardwarePosition >= _positiveEndSwitchHardwarePosition);
  }

  double MotorControllerDummy::getMaxSpeedCapability() {
    throw ChimeraTK::logic_error("MotorControllerDummy::getMaxSpeedCapability() is not implemented yet!");
  }

  double MotorControllerDummy::setUserSpeedLimit(double microStepsPerSecond) {
    LockGuard guard(_motorControllerDummyMutex);
    _userSpeedLimit = microStepsPerSecond;
    return _userSpeedLimit;
  }

  double MotorControllerDummy::getUserSpeedLimit() {
    return _userSpeedLimit;
  }

  double MotorControllerDummy::setUserCurrentLimit(double currentLimit) {
    (void)currentLimit; // To suppress parameter unused warning.
    throw ChimeraTK::logic_error("MotorControllerDummy::setUserCurrentLimit() is not implemented yet!");
  }
  double MotorControllerDummy::getUserCurrentLimit() {
    throw ChimeraTK::logic_error("MotorControllerDummy::getUserCurrentLimit() is not implemented yet!");
  }
  double MotorControllerDummy::getMaxCurrentLimit() {
    throw ChimeraTK::logic_error("MotorControllerDummy::getMaxCurrentLimit() is not implemented yet!");
  }

  void MotorControllerDummy::resetInternalStateToDefaults() {
    LockGuard guard(_motorControllerDummyMutex);
    _hardwarePosition = 0;
    _targetPosition = 0;
    _currentPosition = 0;
    _calibrationTime = 0;
    _positiveEndSwitchEnabled = true;
    _negativeEndSwitchEnabled = true;
    _bothEndSwitchesAlwaysOn = false;
    _blockMotor = false;
  }

  void MotorControllerDummy::setMotorCurrentEnabled(bool enable) {
    LockGuard guard(_motorControllerDummyMutex);
    _motorCurrentEnabled = enable;
  }

  bool MotorControllerDummy::isMotorCurrentEnabled() {
    LockGuard guard(_motorControllerDummyMutex);
    return _motorCurrentEnabled;
  }

  void MotorControllerDummy::setEndSwitchPowerEnabled(bool enable) {
    LockGuard guard(_motorControllerDummyMutex);
    _endSwitchPowerEnabled = enable;
  }
  bool MotorControllerDummy::isEndSwitchPowerEnabled() {
    LockGuard guard(_motorControllerDummyMutex);
    return _endSwitchPowerEnabled;
  }

  bool MotorControllerDummy::isNegativeEndSwitchActive() const {
    if(_bothEndSwitchesAlwaysOn) {
      return true;
    }

    return _negativeEndSwitchEnabled && (_hardwarePosition <= _negativeEndSwitchHardwarePosition);
  }

  void MotorControllerDummy::moveTowardsTarget(
      float fraction, bool block, bool bothEndSwitchesAlwaysOn, bool zeroPositions) {
    LockGuard guard(_motorControllerDummyMutex);
    if(zeroPositions) {
      _hardwarePosition = 0;
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
    int relativeSteps = static_cast<int>(static_cast<float>(_targetPosition - _currentPosition) * fraction);
    int absoluteTargetInThisMove = _hardwarePosition + relativeSteps;
    int targetInThisMove = _currentPosition + relativeSteps;

    if(!_motorCurrentEnabled) {
      // the motor is stepping to the target position, but the actual position
      // does not change. Thus no end switch will be hit.
      _currentPosition = targetInThisMove;
      return;
    }

    // ok, so the motor is really moving.

    // check whether an end switch would be hit
    if(_positiveEndSwitchEnabled && (absoluteTargetInThisMove > _positiveEndSwitchHardwarePosition)) {
      absoluteTargetInThisMove = _positiveEndSwitchHardwarePosition;
      // also the current position stops if an end switch is reached
      int stepsToEndSwitch = _positiveEndSwitchHardwarePosition - _hardwarePosition;
      targetInThisMove = _currentPosition + stepsToEndSwitch;
    }
    if(_negativeEndSwitchEnabled && (absoluteTargetInThisMove < _negativeEndSwitchHardwarePosition)) {
      absoluteTargetInThisMove = _negativeEndSwitchHardwarePosition;
      // also the current position stops if an end switch is reached
      int stepsToEndSwitch = _negativeEndSwitchHardwarePosition - _hardwarePosition;
      targetInThisMove = _currentPosition + stepsToEndSwitch;
    }

    // finally 'move' the motor
    _hardwarePosition = absoluteTargetInThisMove;
    _currentPosition = targetInThisMove;
  }

  void MotorControllerDummy::simulateBlockedMotor(bool state) {
    LockGuard guard(_motorControllerDummyMutex);
    _blockMotor = state;
  }
} // namespace ChimeraTK::MotorDriver

#include "MotorControlerDummy.h"
#include "MotorDriverException.h"

#include <iostream>

typedef std::lock_guard<std::mutex> lock_guard;
typedef std::unique_lock<std::mutex> unique_lock;

namespace mtca4u {

    int MotorControlerDummy::_positiveEndSwitchPosition = 10000;
    int MotorControlerDummy::_negativeEndSwitchPosition = -10000;

    MotorControlerDummy::MotorControlerDummy(unsigned int id)
      : _motorControllerDummyMutex(),
	_absolutePosition(0), _targetPosition(0), _currentPosition(0), _calibrationTime(0),
	_positiveEndSwitchEnabled(true), _negativeEndSwitchEnabled(true),
	_motorCurrentEnabled(false), _endSwitchPowerEnabled(false), _id(id), _blockMotor(false), 
	_bothEndSwitchesAlwaysOn(false),
	_userMicroStepSize(4),
	_isFullStepping(false){}

    unsigned int MotorControlerDummy::getID() {
      lock_guard guard(_motorControllerDummyMutex);
      return _id;
    }

    int MotorControlerDummy::getActualPosition() {
      lock_guard guard(_motorControllerDummyMutex);
      return _currentPosition;
    }

    int MotorControlerDummy::getActualVelocity() {
      lock_guard guard(_motorControllerDummyMutex);
      // the velocity is only not zero if the motor is actually moving
      // FIXME: or if the motor is stepping?
      bool motorMoving = (_motorCurrentEnabled && isStepping());
      if (motorMoving) {
        return (_targetPosition < _currentPosition ? -25 : 25);
      } else {
        return 0;
      }
    }

    void MotorControlerDummy::enableFullStepping(bool enable){
      _isFullStepping = enable;
    }

    bool MotorControlerDummy::isFullStepping(){
      return _isFullStepping;
    }

    bool MotorControlerDummy::isMotorMoving() {
      lock_guard guard(_motorControllerDummyMutex);
      return (_motorCurrentEnabled && isStepping());
    }

    bool MotorControlerDummy::isStepping() {
        // if the motor is at the target position the motor is not stepping
        if (_targetPosition == _currentPosition) {
            return false;
        }

        if (_blockMotor) {
            return false;
        }
        // FIXME: check for errors. To be implemented....

        // Just checking the end switches is not enough. One has to be 
        // able to move away into one direction.
        if (_targetPosition > _currentPosition) {
            // The motor should be moving in positive position
            if (isPositiveEndSwitchActive()) {
                return false;
            }
        } else { //negative direction
            if (isNegativeEndSwitchActive()) {
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
      lock_guard guard(_motorControllerDummyMutex);
      return ((_currentPosition * _userMicroStepSize) - 1 ) & 0x3FF;
    }

    DriverStatusData MotorControlerDummy::getStatus() {
      lock_guard guard(_motorControllerDummyMutex);
      DriverStatusData statusData;
      bool motorMoving = (_motorCurrentEnabled && isStepping());
      statusData.setStandstillIndicator(!motorMoving);
      return statusData;
    }

    unsigned int MotorControlerDummy::getDecoderReadoutMode() {
      lock_guard guard(_motorControllerDummyMutex);
      // I think this is absolute postion
      return DecoderReadoutMode::HEIDENHAIN;
    }

    unsigned int MotorControlerDummy::getDecoderPosition() {
      lock_guard guard(_motorControllerDummyMutex);
      // make the negative end switch "decoder 0"
      return _absolutePosition - _negativeEndSwitchPosition;
    }

    void MotorControlerDummy::setActualVelocity(int /*stepsPerFIXME*/) {
        throw MotorDriverException("MotorControlerDummy::setActualVelocity() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    void MotorControlerDummy::setActualAcceleration(unsigned int /*stepsPerSquareFIXME*/) {
        throw MotorDriverException("MotorControlerDummy::setActualAcceleration() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    void MotorControlerDummy::setMicroStepCount(unsigned int /*microStepCount*/) {
        throw MotorDriverException("MotorControlerDummy::setMicroStepCount() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    void MotorControlerDummy::setEnabled(bool enable) {
      lock_guard guard(_motorControllerDummyMutex);
      _motorCurrentEnabled = enable;
    }

    void MotorControlerDummy::setDecoderReadoutMode(unsigned int /*decoderReadoutMode*/) {
        throw MotorDriverException("MotorControlerDummy::setDecoderReadoutMode(): The Decoder readout mode cannot be changed for the dummy motor!", MotorDriverException::NOT_IMPLEMENTED);
    }

    bool MotorControlerDummy::isEnabled() {
      lock_guard guard(_motorControllerDummyMutex);
      return _motorCurrentEnabled;
    }

    void MotorControlerDummy::setCalibrationTime(uint32_t calibrationTime){
      lock_guard guard(_motorControllerDummyMutex);
      _calibrationTime = calibrationTime;
    }

    uint32_t MotorControlerDummy::getCalibrationTime(){
      lock_guard guard(_motorControllerDummyMutex);
      return static_cast<time_t>(_calibrationTime);
    }

    MotorReferenceSwitchData MotorControlerDummy::getReferenceSwitchData() {
      lock_guard guard(_motorControllerDummyMutex);
      MotorReferenceSwitchData motorReferenceSwitchData;
      motorReferenceSwitchData.setPositiveSwitchEnabled(_positiveEndSwitchEnabled);
      motorReferenceSwitchData.setNegativeSwitchEnabled(_negativeEndSwitchEnabled);


      motorReferenceSwitchData.setPositiveSwitchActive(isPositiveEndSwitchActive());
      motorReferenceSwitchData.setNegativeSwitchActive(isNegativeEndSwitchActive());

      return motorReferenceSwitchData;
    }

    void MotorControlerDummy::setPositiveReferenceSwitchEnabled(bool enableStatus) {
      lock_guard guard(_motorControllerDummyMutex);
      _positiveEndSwitchEnabled = enableStatus;
    }

    void MotorControlerDummy::setNegativeReferenceSwitchEnabled(bool enableStatus) {
      lock_guard guard(_motorControllerDummyMutex);
      _negativeEndSwitchEnabled = enableStatus;
    }

    void MotorControlerDummy::setActualPosition(int steps) {
      lock_guard guard(_motorControllerDummyMutex);
      _currentPosition = steps;
    }

    void MotorControlerDummy::setTargetPosition(int steps) {
      lock_guard guard(_motorControllerDummyMutex);
      if (_isFullStepping){
          roundToNextFullStep(steps);
        }
      _targetPosition = steps;
    }

    void MotorControlerDummy::roundToNextFullStep(int &targetPosition){
        int delta = targetPosition - _currentPosition;
        int deltaMicroStep = delta * _userMicroStepSize;
        unsigned int actualMicroStepCount = ((_currentPosition * _userMicroStepSize) - 1) & 0x3FF;
        unsigned int newActualMicroStepCount = (actualMicroStepCount + deltaMicroStep) & 0x3FF;
        unsigned int distanceToPreviousFullStep =  (newActualMicroStepCount + 1) % 256;
        if (distanceToPreviousFullStep < 128){
          targetPosition = targetPosition - distanceToPreviousFullStep/_userMicroStepSize;
        }else{
          targetPosition = targetPosition + (256 - distanceToPreviousFullStep)/_userMicroStepSize;
        }
      }

    int MotorControlerDummy::getTargetPosition() {
      lock_guard guard(_motorControllerDummyMutex);
      return _targetPosition;
    }

    void MotorControlerDummy::setMinimumVelocity(unsigned int /*stepsPerFIXME*/) {
        throw MotorDriverException("MotorControlerDummy::setMinimumVelocity() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    unsigned int MotorControlerDummy::getMinimumVelocity() {
        throw MotorDriverException("MotorControlerDummy::getMinimumVelocity() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    void MotorControlerDummy::setMaximumVelocity(unsigned int /*stepsPerFIXME*/) {
        throw MotorDriverException("MotorControlerDummy::setMaximumVelocity() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    unsigned int MotorControlerDummy::getMaximumVelocity() {
        throw MotorDriverException("MotorControlerDummy::getMaximumVelocity() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    void MotorControlerDummy::setTargetVelocity(int /*stepsPerFIXME*/) {
        throw MotorDriverException("MotorControlerDummy::setTargetVelocity() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    int MotorControlerDummy::getTargetVelocity() {
        throw MotorDriverException("MotorControlerDummy::getTargetVelocity() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    void MotorControlerDummy::setMaximumAcceleration(unsigned int /*stepsPerSquareFIXME*/) {
        throw MotorDriverException("MotorControlerDummy::setMaximumAcceleration() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    unsigned int MotorControlerDummy::getMaximumAcceleration() {
        throw MotorDriverException("MotorControlerDummy::getMaximumAcceleration() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    void MotorControlerDummy::setPositionTolerance(unsigned int /*steps*/) {
        throw MotorDriverException("MotorControlerDummy::setPositionTolerance() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    unsigned int MotorControlerDummy::getPositionTolerance() {
        throw MotorDriverException("MotorControlerDummy::getPositionTolerance() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    void MotorControlerDummy::setPositionLatched(unsigned int /*steps*/) {
        throw MotorDriverException("MotorControlerDummy::setPositionLatched() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    unsigned int MotorControlerDummy::getPositionLatched() {
        throw MotorDriverException("MotorControlerDummy:getPositionLatched() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    bool MotorControlerDummy::targetPositionReached() {
      lock_guard guard(_motorControllerDummyMutex);
      return (_currentPosition == _targetPosition);
    }

    unsigned int MotorControlerDummy::getReferenceSwitchBit() {
      lock_guard guard(_motorControllerDummyMutex);
      return static_cast<unsigned int>(isPositiveEndSwitchActive() ||
                                       isNegativeEndSwitchActive());
    }

    bool MotorControlerDummy::isPositiveEndSwitchActive() {
        if (_bothEndSwitchesAlwaysOn)
            return true;
        
        return _positiveEndSwitchEnabled && (_absolutePosition >= _positiveEndSwitchPosition);
    }

    double MotorControlerDummy::getMaxSpeedCapability() {
      throw MotorDriverException("MotorControlerDummy::getMaxSpeedCapability() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    double MotorControlerDummy::setUserSpeedLimit(double microStepsPerSecond) {
      (void) microStepsPerSecond; // To suppress parameter unused warning.
      throw MotorDriverException("MotorControlerDummy::setUserSpeedLimit() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    double MotorControlerDummy::getUserSpeedLimit() {
      throw MotorDriverException("MotorControlerDummy::getUserSpeedLimit() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    double MotorControlerDummy::setUserCurrentLimit(double currentLimit){
      (void) currentLimit; // To suppress parameter unused warning.
      throw MotorDriverException("MotorControlerDummy::setUserCurrentLimit() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }
    double MotorControlerDummy::getUserCurrentLimit(){
      throw MotorDriverException("MotorControlerDummy::getUserCurrentLimit() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }
    double MotorControlerDummy::getMaxCurrentLimit(){
      throw MotorDriverException("MotorControlerDummy::getMaxCurrentLimit() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    void MotorControlerDummy::resetInternalStateToDefaults() {
      lock_guard guard(_motorControllerDummyMutex);
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
      lock_guard guard(_motorControllerDummyMutex);
      _motorCurrentEnabled = enable;
  }

  bool MotorControlerDummy::isMotorCurrentEnabled() {
    lock_guard guard(_motorControllerDummyMutex);
    return _motorCurrentEnabled;
  }

  void MotorControlerDummy::setEndSwitchPowerEnabled(bool enable) {
      lock_guard guard(_motorControllerDummyMutex);
      _endSwitchPowerEnabled = enable;
  }
  bool MotorControlerDummy::isEndSwitchPowerEnabled() {
    lock_guard guard(_motorControllerDummyMutex);
    return _endSwitchPowerEnabled;
  }

  bool MotorControlerDummy::isNegativeEndSwitchActive() {
        if (_bothEndSwitchesAlwaysOn)
            return true;
        
        return _negativeEndSwitchEnabled && (_absolutePosition <= _negativeEndSwitchPosition);
    }

    void MotorControlerDummy::moveTowardsTarget(float fraction, bool block, bool bothEndSwitchesAlwaysOn, bool zeroPositions) {
      lock_guard guard(_motorControllerDummyMutex);
        if (zeroPositions) {
            _absolutePosition = 0;
            _targetPosition = 0;
            _currentPosition = 0;
            _positiveEndSwitchEnabled = true;
            _negativeEndSwitchEnabled = true;
        }
        
        _bothEndSwitchesAlwaysOn = bothEndSwitchesAlwaysOn;
        
        _blockMotor = block;
        if (_blockMotor) {
            return;
        }
        
        // check if the motor is should to be moving if it was enabled
        // otherwise return immediately
        if (!isStepping()) {
            return;
        }

        // range check for the input parameter 'fraction', just force it to 0..1
        if (fraction <= 0) {
            return;
        }
        if (fraction > 1) {
            fraction = 1.;
        }

        // calculate the new abolute target position for this move operation
        int relativeSteps = (_targetPosition - _currentPosition) * fraction;
        int absoluteTargetInThisMove = _absolutePosition + relativeSteps;
        int targetInThisMove = _currentPosition + relativeSteps;

        if (!_motorCurrentEnabled) {
            // the motor is stepping to the target position, but the actual positon 
            // does not change. Thus no end switch will be hit.
            _currentPosition = targetInThisMove;
            return;
        }

        // ok, so the motor is really moving.

        // check whether an end switch would be hit
        if (_positiveEndSwitchEnabled &&
                (absoluteTargetInThisMove > _positiveEndSwitchPosition)) {
            absoluteTargetInThisMove = _positiveEndSwitchPosition;
            // also the current positon stops if an end switch is reached
            int stepsToEndSwitch = _positiveEndSwitchPosition - _absolutePosition;
            targetInThisMove = _currentPosition + stepsToEndSwitch;
        }
        if (_negativeEndSwitchEnabled &&
                (absoluteTargetInThisMove < _negativeEndSwitchPosition)) {
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
      lock_guard guard(_motorControllerDummyMutex);
      _blockMotor = state;
    }
    } // namespace mtca4u


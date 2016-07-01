#include "MotorControlerDummy.h"
#include "MotorDriverException.h"

#include <iostream>

namespace mtca4u {

    int const MotorControlerDummy::_positiveEndSwitchPosition = 10000;
    int const MotorControlerDummy::_negativeEndSwitchPosition = -10000;

    MotorControlerDummy::MotorControlerDummy(unsigned int id)
    : _absolutePosition(0), _targetPosition(0), _currentPosition(0),
    _positiveEndSwitchEnabled(true), _negativeEndSwitchEnabled(true),
    _enabled(false), _id(id), _blockMotor(false), _bothEndSwitchesAlwaysOn(false) {
    }

    unsigned int MotorControlerDummy::getID() {
        return _id;
    }

    int MotorControlerDummy::getActualPosition() {
        return _currentPosition;
    }

    int MotorControlerDummy::getActualVelocity() {
        // the velocity is only not zero if the motor is actually moving
        // FIXME: or if the motor is stepping?
        if (isMotorMoving()) {
            return ( _targetPosition < _currentPosition ? -25 : 25);
        } else {
            return 0;
        }
    }

    bool MotorControlerDummy::isMotorMoving() {
        return (_enabled && isStepping());
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
        // I actually don't know what to return. I use the current position....
        return _currentPosition;
    }

    DriverStatusData MotorControlerDummy::getStatus() {
        DriverStatusData statusData;
        statusData.setStandstillIndicator(!isMotorMoving());
        return statusData;
    }

    unsigned int MotorControlerDummy::getDecoderReadoutMode() {
        // I think this is absolute postion
        return DecoderReadoutMode::HEIDENHAIN;
    }

    unsigned int MotorControlerDummy::getDecoderPosition() {
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
        _enabled = enable;
    }

    void MotorControlerDummy::setDecoderReadoutMode(unsigned int /*decoderReadoutMode*/) {
        throw MotorDriverException("MotorControlerDummy::setDecoderReadoutMode(): The Decoder readout mode cannot be changed for the dummy motor!", MotorDriverException::NOT_IMPLEMENTED);
    }

    bool MotorControlerDummy::isEnabled() {
        return _enabled;
    }

    MotorReferenceSwitchData MotorControlerDummy::getReferenceSwitchData() {
        MotorReferenceSwitchData motorReferenceSwitchData;
        motorReferenceSwitchData.setPositiveSwitchEnabled(_positiveEndSwitchEnabled);
        motorReferenceSwitchData.setNegativeSwitchEnabled(_negativeEndSwitchEnabled);


        motorReferenceSwitchData.setPositiveSwitchActive(isPositiveEndSwitchActive());
        motorReferenceSwitchData.setNegativeSwitchActive(isNegativeEndSwitchActive());

        return motorReferenceSwitchData;
    }

    void MotorControlerDummy::setPositiveReferenceSwitchEnabled(bool enableStatus) {
        _positiveEndSwitchEnabled = enableStatus;
    }

    void MotorControlerDummy::setNegativeReferenceSwitchEnabled(bool enableStatus) {
        _negativeEndSwitchEnabled = enableStatus;
    }

    void MotorControlerDummy::setActualPosition(int steps) {
        _currentPosition = steps;
    }

    void MotorControlerDummy::setTargetPosition(int steps) {
        _targetPosition = steps;
    }

    int MotorControlerDummy::getTargetPosition() {
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
        return (_currentPosition == _targetPosition);
    }

    unsigned int MotorControlerDummy::getReferenceSwitchBit() {
        return static_cast<unsigned int> (isPositiveEndSwitchActive() ||
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
      throw MotorDriverException("MotorControlerDummy::setUserSpeedLimit() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    double MotorControlerDummy::getUserSpeedLimit() {
      throw MotorDriverException("MotorControlerDummy::getUserSpeedLimit() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    double MotorControlerDummy::setUserCurrentLimit(double currentLimit){
      throw MotorDriverException("MotorControlerDummy::setUserCurrentLimit() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }
    double MotorControlerDummy::getUserCurrentLimit(){
      throw MotorDriverException("MotorControlerDummy::getUserCurrentLimit() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }
    double MotorControlerDummy::getMaxCurrentLimit(){
      throw MotorDriverException("MotorControlerDummy::getMaxCurrentLimit() is not implemented yet!", MotorDriverException::NOT_IMPLEMENTED);
    }

    void MotorControlerDummy::resetInternalStateToDefaults() {
      _absolutePosition = 0;
      _targetPosition = 0;
      _currentPosition = 0;
      _positiveEndSwitchEnabled = true;
      _negativeEndSwitchEnabled = true;
      _bothEndSwitchesAlwaysOn = false;
      _blockMotor = false;
  }


    bool MotorControlerDummy::isNegativeEndSwitchActive() {
        if (_bothEndSwitchesAlwaysOn)
            return true;
        
        return _negativeEndSwitchEnabled && (_absolutePosition <= _negativeEndSwitchPosition);
    }

    void MotorControlerDummy::moveTowardsTarget(float fraction, bool block, bool bothEndSwitchesAlwaysOn, bool zeroPositions) {
       
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

        if (!_enabled) {
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
      _blockMotor = state;
    }
    } // namespace mtca4u


#include "MotorControlerDummy.h"
#include <MtcaMappedDevice/NotImplementedException.h>

namespace mtca4u{

  int const MotorControlerDummy::_positiveEndSwitchPosition = 10000;
  int const MotorControlerDummy::_negativeEndSwitchPosition =-10000;

  MotorControlerDummy::MotorControlerDummy(unsigned int id)
    : _absolutePosition(0), _targetPosition(0), _currentPosition(0),
      _positiveEndSwitchEnabled(true), _negativeEndSwitchEnabled(true),
      _enabled(false), _id(id)
  {}

  unsigned int MotorControlerDummy::getID(){
    return _id;
  }

  int MotorControlerDummy::getActualPosition(){
    return _currentPosition;
  }

  int MotorControlerDummy::getActualVelocity(){
    if (isMoving()){
      return ( _targetPosition < _currentPosition ? 25 : -25 );
    }
    else{
      return 0;
    }
  }

  bool MotorControlerDummy::isMoving(){
    // if the motor is at the end position the motor is not moving
    if (_targetPosition == _currentPosition){return false;}

    // FIXME: check for errors. To be implemented....

    // Just checking the end switches is not enough. One has to be 
    // able to move away into one direction.
    if (_targetPosition > _currentPosition){
      // The motor should be moving in positive position
      if (isPositiveEndSwitchActive()){return false;}
    }else{ //negative direction
      if (isNegativeEndSwitchActive()){return false;}
    }
    
    return true;
  }

  unsigned int MotorControlerDummy::getActualAcceleration(){
    // This motor accelerates instantaneously. Either it is moving or not.
    // (not very realistic, but ok for a test)
    return 0;
  }

  unsigned int MotorControlerDummy::getMicroStepCount(){
    // I actually don't know what to return. I use the current position....
    return _currentPosition;
  }

  DriverStatusData MotorControlerDummy::getStatus(){
    throw NotImplementedException("MotorControlerDummy::getStatus() is not implemented yet!");
  }

  unsigned int MotorControlerDummy::getDecoderReadoutMode(){
    // I think this is absolute postion
    return DecoderReadoutMode::HEIDENHAIN;
  }

  unsigned int MotorControlerDummy::getDecoderPosition(){
    //FIXME: this returns unsigned
    return _absolutePosition;
  }
    
  void MotorControlerDummy::setActualVelocity(int /*stepsPerFIXME*/){
    throw NotImplementedException("MotorControlerDummy::setActualVelocity() is not implemented yet!");
  }

  void MotorControlerDummy::setActualAcceleration(unsigned int /*stepsPerSquareFIXME*/){
    throw NotImplementedException("MotorControlerDummy::setActualAcceleration() is not implemented yet!");
  }
  
  void MotorControlerDummy::setMicroStepCount(unsigned int /*microStepCount*/){
     throw NotImplementedException("MotorControlerDummy::setMicroStepCount() is not implemented yet!");   
  }

  void MotorControlerDummy::setEnabled(bool enable){
    _enabled=enable;
  }

  void MotorControlerDummy::setDecoderReadoutMode(unsigned int /*decoderReadoutMode*/){
      throw NotImplementedException("MotorControlerDummy::setDecoderReadoutMode(): The Decoder readout mode cannot be changed for the dummy motor!");   
 }

  bool MotorControlerDummy::isEnabled(){
    return _enabled;
  }
    
  MotorReferenceSwitchData MotorControlerDummy::getReferenceSwitchData(){
    MotorReferenceSwitchData motorReferenceSwitchData;
    motorReferenceSwitchData.setPositiveSwitchEnabled( _positiveEndSwitchEnabled );
    motorReferenceSwitchData.setNegativeSwitchEnabled( _negativeEndSwitchEnabled );
    motorReferenceSwitchData.setPositiveSwitchActive( isPositiveEndSwitchActive() );
    motorReferenceSwitchData.setNegativeSwitchActive( isNegativeEndSwitchActive() );

    return motorReferenceSwitchData;
  }

  void MotorControlerDummy::setPositiveReferenceSwitchEnabled(bool enableStatus){
    _positiveEndSwitchEnabled=enableStatus;
  }

  void MotorControlerDummy::setNegativeReferenceSwitchEnabled(bool enableStatus){
    _negativeEndSwitchEnabled=enableStatus;    
  }

  void MotorControlerDummy::setActualPosition(int steps){
    _currentPosition=steps;
  }

  void MotorControlerDummy::setTargetPosition(int steps){
    _targetPosition=steps;
  }
  int MotorControlerDummy::getTargetPosition(){
    return _targetPosition;
  }

  void MotorControlerDummy::setMinimumVelocity(unsigned int /*stepsPerFIXME*/){
     throw NotImplementedException("MotorControlerDummy::setMinimumVelocity() is not implemented yet!");       
  }
  unsigned int MotorControlerDummy::getMinimumVelocity(){
     throw NotImplementedException("MotorControlerDummy::getMinimumVelocity() is not implemented yet!");       
  }

  void MotorControlerDummy::setMaximumVelocity(unsigned int /*stepsPerFIXME*/){
     throw NotImplementedException("MotorControlerDummy::setMaximumVelocity() is not implemented yet!");       
  }
  unsigned int MotorControlerDummy::getMaximumVelocity(){
     throw NotImplementedException("MotorControlerDummy::getMaximumVelocity() is not implemented yet!");       
  }

  void MotorControlerDummy::setTargetVelocity(int /*stepsPerFIXME*/){
     throw NotImplementedException("MotorControlerDummy::setTargetVelocity() is not implemented yet!");           
  }
  int MotorControlerDummy::getTargetVelocity(){
     throw NotImplementedException("MotorControlerDummy::getTargetVelocity() is not implemented yet!");           
  }
  
  void MotorControlerDummy::setMaximumAcceleration(unsigned int /*stepsPerSquareFIXME*/){
      throw NotImplementedException("MotorControlerDummy::setMaximumAcceleration() is not implemented yet!");              
  }
  unsigned int MotorControlerDummy::getMaximumAcceleration(){
      throw NotImplementedException("MotorControlerDummy::getMaximumAcceleration() is not implemented yet!");              
  }

  void MotorControlerDummy::setPositionTolerance(unsigned int /*steps*/){
      throw NotImplementedException("MotorControlerDummy::setPositionTolerance() is not implemented yet!");              
  }
  unsigned int MotorControlerDummy::getPositionTolerance(){
      throw NotImplementedException("MotorControlerDummy::getPositionTolerance() is not implemented yet!");              
  }

  void MotorControlerDummy::setPositionLatched(unsigned int /*steps*/){
      throw NotImplementedException("MotorControlerDummy::setPositionLatched() is not implemented yet!");              
  }
  unsigned int MotorControlerDummy::getPositionLatched(){
      throw NotImplementedException("MotorControlerDummy:gsetPositionLatched() is not implemented yet!");              
  }

  bool MotorControlerDummy::targetPositionReached(){
    return (_currentPosition==_targetPosition);
  }

  unsigned int MotorControlerDummy::getReferenceSwitchBit(){
    return static_cast<unsigned int>(isPositiveEndSwitchActive() ||
				     isNegativeEndSwitchActive());
  }

  bool MotorControlerDummy::isPositiveEndSwitchActive(){
    return _positiveEndSwitchEnabled && (_absolutePosition==_positiveEndSwitchPosition);
  }

  bool MotorControlerDummy::isNegativeEndSwitchActive(){
    return _negativeEndSwitchEnabled && (_absolutePosition==_negativeEndSwitchPosition);
  }

  void MotorControlerDummy::moveTowardsTarget(float fraction){
    // check if the motor is considered to be moving, otherwise return immediately
    if (!isMoving()){return;}
    
    // range check for the input parameter 'fraction', just force it to 0..1
    if (fraction <= 0){return;}
    if (fraction >1){fraction = 1.;}

    // calculate the new abolute target position for this move operation
    int relativeSteps = (_targetPosition - _currentPosition)*fraction;
    int absoluteTargetInThisMove = _absolutePosition + relativeSteps;

    // check whether an end switch would be hit
    if (absoluteTargetInThisMove > _positiveEndSwitchPosition){
      absoluteTargetInThisMove =  _positiveEndSwitchPosition;
    }
    if (absoluteTargetInThisMove < _negativeEndSwitchPosition){
      absoluteTargetInThisMove =  _negativeEndSwitchPosition;
    }
    
    // finally 'move' the motor
    _absolutePosition = absoluteTargetInThisMove;
  }

}// namespace mtca4u

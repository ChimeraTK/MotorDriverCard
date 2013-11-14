#include "MotorControler.h"

#include "DFMC_MD22Constants.h"
using namespace mtca4u::dfmc_md22;

#include "MotorDriverCardImpl.h"


// just save some typing...
#define REG_OBJECT_FROM_SUFFIX( SUFFIX )\
  _driverCard._mappedDevice->getRegObject( createMotorRegisterName( _id, SUFFIX ) )

namespace mtca4u
{

  MotorControler::MotorControler( unsigned int ID, MotorDriverCardImpl & driverCard ) 
    : _driverCard(driverCard) , _id(ID), 
      _actualPosition( REG_OBJECT_FROM_SUFFIX(  ACTUAL_POSITION_SUFFIX ) ),
      _actualVelocity( REG_OBJECT_FROM_SUFFIX( ACTUAL_VELOCITY_SUFFIX ) ),
      _actualAcceleration( REG_OBJECT_FROM_SUFFIX( ACTUAL_ACCELETATION_SUFFIX ) ),
      //_accelerationThreshold( REG_OBJECT_FROM_SUFFIX( ACCELERATION_THRESHOLD_SUFFIX ) ),
      _microStepCount( REG_OBJECT_FROM_SUFFIX( MICRO_STEP_COUNT_SUFFIX ) )
  {}

  unsigned int MotorControler::getID(){
    return _id;
  }
   
  unsigned int MotorControler::getActualPosition(){
    return readRegObject( _actualPosition );
  }

  unsigned int MotorControler::readRegObject( devMap<devBase>::regObject const & registerAccessor){
    int readValue;
    registerAccessor.readReg( &readValue );
    return static_cast<unsigned int>(readValue);
  }

  void MotorControler::setActualPosition(unsigned int position){
    _actualPosition.writeReg(reinterpret_cast<int32_t *>(&position));
  }

  unsigned int MotorControler::getActualVelocity(){
    return readRegObject( _actualVelocity );
  }

  void MotorControler::setActualVelocity(unsigned int velocity){
    _actualVelocity.writeReg(reinterpret_cast<int32_t *>(&velocity));
  }

  unsigned int MotorControler::getActualAcceleration(){
    return readRegObject( _actualAcceleration );
  }

  void MotorControler::setActualAcceleration(unsigned int acceleration){
    _actualAcceleration.writeReg(reinterpret_cast<int32_t *>(&acceleration));
  }

//  unsigned int MotorControler::getAccelerationThreshold(){
//    return readRegObject( _accelerationThreshold );
//  }
//
//  void MotorControler::setAccelerationThreshold(unsigned int accelerationThreshold){
//    _accelerationThreshold.writeReg(reinterpret_cast<int32_t *>(&accelerationThreshold));
//  }

  unsigned int MotorControler::getMicroStepCount(){
    return readRegObject( _microStepCount );
  }

  void MotorControler::setMicroStepCount(unsigned int microStepCount){
    _microStepCount.writeReg(reinterpret_cast<int32_t *>(&microStepCount));
  }

}// namespace mtca4u

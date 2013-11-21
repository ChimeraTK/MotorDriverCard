#include "MotorControler.h"

#include "DFMC_MD22Constants.h"
using namespace mtca4u::dfmc_md22;
#include "TMC429Constants.h"
using namespace mtca4u::tmc429;

#include "MotorDriverCardImpl.h"


// just save some typing...
#define REG_OBJECT_FROM_SUFFIX( SUFFIX )\
  _driverCard._mappedDevice->getRegObject( createMotorRegisterName( _id, SUFFIX ) )
#define REG_OBJECT_FROM_REG_NAME( NAME )\
  _driverCard._mappedDevice->getRegObject( NAME )

namespace mtca4u
{

  MotorControler::MotorControler( unsigned int ID, MotorDriverCardImpl & driverCard ) 
    : _driverCard(driverCard) , _id(ID), 
      _actualPosition( REG_OBJECT_FROM_SUFFIX(  ACTUAL_POSITION_SUFFIX ) ),
      _actualVelocity( REG_OBJECT_FROM_SUFFIX( ACTUAL_VELOCITY_SUFFIX ) ),
      _actualAcceleration( REG_OBJECT_FROM_SUFFIX( ACTUAL_ACCELETATION_SUFFIX ) ),
      //_accelerationThreshold( REG_OBJECT_FROM_SUFFIX( ACCELERATION_THRESHOLD_SUFFIX ) ),
      _microStepCount( REG_OBJECT_FROM_SUFFIX( MICRO_STEP_COUNT_SUFFIX ) ),
      _stallGuardValue( REG_OBJECT_FROM_SUFFIX( STALL_GUARD_VALUE_SUFFIX ) ),
      _coolStepValue( REG_OBJECT_FROM_SUFFIX( COOL_STEP_VALUE_SUFFIX ) ),
      _status( REG_OBJECT_FROM_SUFFIX( STATUS_SUFFIX ) ),
      _enabled( REG_OBJECT_FROM_SUFFIX( ENABLE_SUFFIX ) ),
      _decoderReadoutMode( REG_OBJECT_FROM_SUFFIX( DECODER_READOUT_MODE_SUFFIX ) ),
      _decoderPosition( REG_OBJECT_FROM_SUFFIX( DECODER_POSITION_SUFFIX ) )
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
    _driverCard.controlerSpiWrite( _id, IDX_ACTUAL_POSITION, position );
  }

  unsigned int MotorControler::getActualVelocity(){
    return readRegObject( _actualVelocity );
  }

  void MotorControler::setActualVelocity(unsigned int velocity){
    _driverCard.controlerSpiWrite( _id, IDX_ACTUAL_VELOCITY, velocity );
  }

  unsigned int MotorControler::getActualAcceleration(){
    return readRegObject( _actualAcceleration );
  }

  void MotorControler::setActualAcceleration(unsigned int acceleration){
    _driverCard.controlerSpiWrite( _id, IDX_ACTUAL_ACCELERATION, acceleration );
  }

  unsigned int MotorControler::getMicroStepCount(){
    return readRegObject( _microStepCount );
  }

  void MotorControler::setMicroStepCount(unsigned int microStepCount){
    _driverCard.controlerSpiWrite( _id, IDX_MICRO_STEP_COUNT, microStepCount );
  }

  unsigned int MotorControler::getCoolStepValue(){
    return readRegObject( _coolStepValue );
  }
 
  unsigned int MotorControler::getStallGuardValue(){
    return readRegObject( _stallGuardValue );
  }
 
  unsigned int MotorControler::getStatus(){
    return readRegObject( _status );
  }
 
  void MotorControler::setDecoderReadoutMode(unsigned int readoutMode){
    int32_t temporaryWriteWord = static_cast<int32_t>(readoutMode);
    _decoderReadoutMode.writeReg( &temporaryWriteWord );
  }

  unsigned int MotorControler::getDecoderReadoutMode(){
     return readRegObject( _decoderReadoutMode );
  }

  unsigned int MotorControler::getDecoderPosition(){
     return readRegObject( _decoderPosition );
  }
 
  void MotorControler::setEnabled(bool enable){
    int32_t enableWord = ( enable ? 1 : 0 );
    _enabled.writeReg( &enableWord );
  }

  bool MotorControler::isEnabled(){
     return readRegObject( _enabled );
  }

}// namespace mtca4u

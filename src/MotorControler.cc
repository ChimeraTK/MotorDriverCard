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
      _controlerSpiWrite( REG_OBJECT_FROM_REG_NAME(CONTROLER_SPI_WRITE_ADDRESS_STRING )),
      _controlerSpiReadback( REG_OBJECT_FROM_REG_NAME(CONTROLER_SPI_READBACK_ADDRESS_STRING )),
      _actualPosition( REG_OBJECT_FROM_SUFFIX(  ACTUAL_POSITION_SUFFIX ) ),
      _actualVelocity( REG_OBJECT_FROM_SUFFIX( ACTUAL_VELOCITY_SUFFIX ) ),
      _actualAcceleration( REG_OBJECT_FROM_SUFFIX( ACTUAL_ACCELETATION_SUFFIX ) ),
      //_accelerationThreshold( REG_OBJECT_FROM_SUFFIX( ACCELERATION_THRESHOLD_SUFFIX ) ),
      _microStepCount( REG_OBJECT_FROM_SUFFIX( MICRO_STEP_COUNT_SUFFIX ) ),
      _stallGuardValue( REG_OBJECT_FROM_SUFFIX( STALL_GUARD_VALUE_SUFFIX ) ),
      _coolStepValue( REG_OBJECT_FROM_SUFFIX( COOL_STEP_VALUE_SUFFIX ) )
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
    writeControlerSpi( position, IDX_ACTUAL_POSITION );
  }

  void MotorControler::writeControlerSpi(unsigned int data, 
					 unsigned int IDX ){
    TMC429InputWord spiWord;
    spiWord.setDATA( data );
    spiWord.setSMDA(_id);
    spiWord.setIDX_JDX( IDX );
    _controlerSpiWrite.writeReg(reinterpret_cast<int32_t *>(&spiWord) );
  } 

  unsigned int MotorControler::getActualVelocity(){
    return readRegObject( _actualVelocity );
  }

  void MotorControler::setActualVelocity(unsigned int velocity){
    writeControlerSpi( velocity, IDX_ACTUAL_VELOCITY );
  }

  unsigned int MotorControler::getActualAcceleration(){
    return readRegObject( _actualAcceleration );
  }

  void MotorControler::setActualAcceleration(unsigned int acceleration){
    writeControlerSpi( acceleration, IDX_ACTUAL_ACCELERATION );
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
    writeControlerSpi( microStepCount, IDX_MICRO_STEP_COUNT );
  }

   unsigned int MotorControler::getCoolStepValue(){
    return readRegObject( _coolStepValue );
  }
 
   unsigned int MotorControler::getStallGuardValue(){
    return readRegObject( _stallGuardValue );
  }
 

}// namespace mtca4u

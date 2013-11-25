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

// Macros which cover more than one line are not good for the code coverage test, as only the 
// macro call is checked. In this case it is probably ok because we gain a lot of code because
// we have to define many variables, AND there is only one line per function without braching,
// which is covered by the funcion coverage test (not the line coverage test).
// Otherwise we would need a complete test which checks the macro.
#define DEFINE_GET_SET_VALUE( NAME, IDX )\
  unsigned int MotorControler::get ## NAME (){\
    return _driverCard.controlerSpiRead( _id, IDX ).getDATA();}\
  void MotorControler::set ## NAME (unsigned int value){\
  _driverCard.controlerSpiWrite( _id, IDX, value );}

#define DEFINE_WRITE_READ_TYPED_REGISTER( NAME, TYPE )\
  TYPE MotorControler::read ## NAME (){\
    return readTypedRegister< TYPE >();}\
  void MotorControler::write ## NAME ( TYPE inputWord ){\
    writeTypedRegister( inputWord );}

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
      _decoderPosition( REG_OBJECT_FROM_SUFFIX( DECODER_POSITION_SUFFIX ) ),
      _driverSpiWrite( REG_OBJECT_FROM_SUFFIX( SPI_WRITE_SUFFIX ) )
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

  DEFINE_GET_SET_VALUE( TargetPosition, IDX_TARGET_POSITION )
  DEFINE_GET_SET_VALUE( MinimumVelocity, IDX_MINIMUM_VELOCITY )
  DEFINE_GET_SET_VALUE( MaximumVelocity, IDX_MAXIMUM_VELOCITY )
  DEFINE_GET_SET_VALUE( TargetVelocity, IDX_TARGET_VELOCITY )
  DEFINE_GET_SET_VALUE( MaximumAcceleration, IDX_MAXIMUM_ACCELERATION )
  DEFINE_GET_SET_VALUE( PositionTolerance, IDX_DELTA_X_REFERENCE_TOLERANCE )
  DEFINE_GET_SET_VALUE( PositionLatched, IDX_POSITION_LATCHED )

  template<class T>
  T  MotorControler::readTypedRegister(){
    T typedWord;
    typedWord.setSMDA( _id );
    TMC429OutputWord readbackWord =  _driverCard.controlerSpiRead( _id, typedWord.getIDX_JDX());
    typedWord.setDATA( readbackWord.getDATA() );
    return typedWord;
  }

  void MotorControler::writeTypedRegister(TMC429InputWord inputWord){
    // set/overwrite the id with this motors id
    inputWord.setSMDA( _id );
    _driverCard.controlerSpiWrite( inputWord );
 
  }

  DEFINE_WRITE_READ_TYPED_REGISTER( AccelerationThresholdRegister, AccelerationThresholdData )
  DEFINE_WRITE_READ_TYPED_REGISTER( ProportionalityFactorRegister, ProportionalityFactorData )
  DEFINE_WRITE_READ_TYPED_REGISTER( ReferenceConfigAndRampModeRegister, ReferenceConfigAndRampModeData )
  DEFINE_WRITE_READ_TYPED_REGISTER( InterruptRegister, InterruptData )
  DEFINE_WRITE_READ_TYPED_REGISTER( DividersAndMicroStepResolutionRegister, DividersAndMicroStepResolutionData )

  DriverControlData const & MotorControler::getDriverControlData() const{
    return _driverControlData;
  }

  ChopperControlData const & MotorControler::getChopperControlData() const{
    return _chopperControlData;
  }

  CoolStepControlData const & MotorControler::getCoolStepControlData() const{
    return _coolStepControlData;
  }

  StallGuardControlData const & MotorControler::getStallGuardControlData() const{
    return _stallGuardControlData;
  }

  DriverConfigData const & MotorControler::getDriverConfigData() const{
    return _driverConfigData;
  }
           
  void MotorControler::setDriverControlRegister(DriverControlData driverControlData){
    int32_t temporaryWriteWord = static_cast<int32_t>(driverControlData.getDataWord());
     _driverSpiWrite.writeReg( &temporaryWriteWord );
     // Remember the written word for readback.
     _driverControlData = driverControlData;
  }

}// namespace mtca4u

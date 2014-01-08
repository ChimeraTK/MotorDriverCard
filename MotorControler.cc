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

#define DEFINE_SET_GET_TYPED_CONTROLER_REGISTER( NAME )\
  NAME MotorControler::get ## NAME (){\
    return readTypedRegister< NAME >();}\
  void MotorControler::set ## NAME ( NAME const & inputWord ){\
    writeTypedControlerRegister( inputWord );}

#define DEFINE_SET_GET_TYPED_DRIVER_DATA( NAME, LOCAL_DATA_INSTANCE )\
  NAME const & MotorControler::get ## NAME () const{\
    return LOCAL_DATA_INSTANCE;}\
  void MotorControler::set ## NAME ( NAME const & driverData){\
    setTypedDriverData< NAME >( driverData, LOCAL_DATA_INSTANCE );}

namespace mtca4u
{

  MotorControler::MotorControler( unsigned int ID, MotorDriverCardImpl & driverCard,
				   MotorControlerConfig const & motorControlerConfig ) 
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
      _driverSpiWrite( REG_OBJECT_FROM_SUFFIX( SPI_WRITE_SUFFIX ) ),
      _driverSpiWaitingTime( motorControlerConfig.driverSpiWaitingTime )
  {
    setAccelerationThresholdData( motorControlerConfig.accelerationThresholdData );
    setActualPosition( motorControlerConfig.actualPosition );
    setChopperControlData( motorControlerConfig.chopperControlData );
    setCoolStepControlData( motorControlerConfig.coolStepControlData );
    setDecoderReadoutMode( motorControlerConfig.decoderReadoutMode );
    setDividersAndMicroStepResolutionData( motorControlerConfig.dividersAndMicroStepResolutionData );
    setDriverConfigData( motorControlerConfig.driverConfigData );
    setDriverControlData( motorControlerConfig.driverControlData );
    setInterruptData( motorControlerConfig.interruptData );
    setMaximumAcceleration( motorControlerConfig.maximumAccelleration );
    setMaximumVelocity( motorControlerConfig.maximumVelocity );
    setMicroStepCount( motorControlerConfig.microStepCount );
    setMinimumVelocity( motorControlerConfig.minimumVelocity );
    setPositionTolerance( motorControlerConfig.positionTolerance );
    setProportionalityFactorData( motorControlerConfig.proportionalityFactorData );
    setReferenceConfigAndRampModeData( motorControlerConfig.referenceConfigAndRampModeData );
    setStallGuardControlData( motorControlerConfig.stallGuardControlData );
    setTargetPosition( motorControlerConfig.targetPosition );
    setTargetVelocity( motorControlerConfig.targetVelocity );

    // enabling the motor is the last step after setting all registers
    setEnabled( motorControlerConfig.enabled );
  }

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

  void MotorControler::writeTypedControlerRegister(TMC429InputWord inputWord){
    // set/overwrite the id with this motors id
    inputWord.setSMDA( _id );
    _driverCard.controlerSpiWrite( inputWord );
 
  }

  DEFINE_SET_GET_TYPED_CONTROLER_REGISTER( AccelerationThresholdData )
  DEFINE_SET_GET_TYPED_CONTROLER_REGISTER( ProportionalityFactorData )
  DEFINE_SET_GET_TYPED_CONTROLER_REGISTER( ReferenceConfigAndRampModeData )
  DEFINE_SET_GET_TYPED_CONTROLER_REGISTER( InterruptData )
  DEFINE_SET_GET_TYPED_CONTROLER_REGISTER( DividersAndMicroStepResolutionData )

  DEFINE_SET_GET_TYPED_DRIVER_DATA( DriverControlData, _driverControlData)
  DEFINE_SET_GET_TYPED_DRIVER_DATA( ChopperControlData, _chopperControlData)
  DEFINE_SET_GET_TYPED_DRIVER_DATA( CoolStepControlData, _coolStepControlData)
  DEFINE_SET_GET_TYPED_DRIVER_DATA( StallGuardControlData, _stallGuardControlData)
  DEFINE_SET_GET_TYPED_DRIVER_DATA( DriverConfigData, _driverConfigData)

  template <class T>
  void MotorControler::setTypedDriverData(T const & driverData, T & localDataInstance){
    int32_t temporaryWriteWord = static_cast<int32_t>(driverData.getDataWord());

    // FIXME: protect the following section by mutex
     _driverSpiWrite.writeReg( &temporaryWriteWord );
     // Remember the written word for readback.
     localDataInstance  = driverData;
     
     // FIXME: Implement a handshake to wait until the word is written
     // Current workaround: introduce a timeout (will also be needed later in the polling
     // loop for the synchronisation)
     _driverCard.sleepMicroSeconds( _driverSpiWaitingTime );

     // FIXME: End of mutex protected section
  }

}// namespace mtca4u

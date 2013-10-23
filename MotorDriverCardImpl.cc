#include <boost/shared_ptr.hpp>

#include "MotorDriverCardImpl.h"

namespace mtca4u{

  MotorDriverCardImpl::MotorDriverCardImpl(boost::shared_ptr< devMap<devBase> > & mappedDevice,
				   MotorDriverConfiguration & motorDriverConfiguration)
    : _mappedDevice(mappedDevice),
      _spiControlWriteRegister( 
           mappedDevice->getRegObject( SPI_CONTROL_WRITE_ADDRESS_STRING )) ,
      _spiControlReadbackRegister(
	   mappedDevice->getRegObject( SPI_CONTROL_READBACK_ADDRESS_STRING )),
      _powerMonitor(new PowerMonitor)
  {
    
  }

  MotorControler & MotorDriverCardImpl::getMotorControler(unsigned int motorControlerID){
    if (motorControlerID > 2){
      throw MotorDriverException("motorControlerID too large", 
				 MotorDriverException::WRONG_MOTOR_ID);
    }
    
    return *(_motorControlers[motorControlerID]);
  }

  PowerMonitor & MotorDriverCardImpl::getPowerMonitor(){
    return *_powerMonitor;
  }

  unsigned int MotorDriverCardImpl::getControlerChipVersion(){
    return spiRead( SMDA_COMMON, JDX_CHIP_VERSION ).getDATA();
  }

  TMC429Word MotorDriverCardImpl::spiRead( unsigned int smda, unsigned int idx_jdx )
  {
    // Although the first half is almost identical to the spiWrite,
    // the preparation of the TMC429Word is different. So we accept a little 
    // code duplication here.
    TMC429Word readRequest;
    readRequest.setSMDA(smda);
    readRequest.setIDX_JDX(idx_jdx);
    readRequest.setRW(RW_READ);

    // we need this intermediate variable because the write function needs a pointer
    // to int.
    int writeMe = static_cast<int>(readRequest.getDataWord());
    _spiControlWriteRegister.writeReg(&writeMe, sizeof(int));
    //FIXME: synchronise so readback value is valid
    
    int readbackValue;
    _spiControlReadbackRegister.readReg( & readbackValue, sizeof(int));

    return TMC429Word(static_cast<unsigned int>(readbackValue));
  }

  void MotorDriverCardImpl::spiWrite( unsigned int smda, unsigned int idx_jdx, 
				      unsigned int data )
  {
    TMC429Word writeMe;
    writeMe.setSMDA(smda);
    writeMe.setIDX_JDX(idx_jdx);
    writeMe.setRW(RW_WRITE);
    writeMe.setDATA(data);

    int temporaryWriteInt = static_cast<int>(writeMe.getDataWord());
    _spiControlWriteRegister.writeReg( &temporaryWriteInt, sizeof(int));
  }

  void MotorDriverCardImpl::setDatagramLowWord(unsigned int datagramLowWord){
  }

  void MotorDriverCardImpl::setDatagramHighWord(unsigned int datagramHighWord){
  }
   
  void MotorDriverCardImpl::setCoverPositionAndLength(
		CoverPositionAndLength const & coverPositionAndLength){
  }

  void MotorDriverCardImpl::setCoverDatagram(unsigned int coverDatagram){
  }

  
  void MotorDriverCardImpl::setStepperMotorGlobalParametersRegister(
	   StepperMotorGlobalParameters const & stepperMotorGlobalParameters){
  }

  void MotorDriverCardImpl::setInterfaceConfiguration(
	   InterfaceConfiguration const & interfaceConfiguration){
  }

  void MotorDriverCardImpl::setPositionCompareRegister(
           unsigned int positionCompareWord){
  }

  void MotorDriverCardImpl::setPositionCompareInterruptRegister(
	   PositionCompareInterruptData const & positionCompareInterruptData){
  }

  void MotorDriverCardImpl::powerDown(){
  }

}// namespace mtca4u

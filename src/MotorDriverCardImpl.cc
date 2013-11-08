#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <sstream>

#include "NotImplementedException.h"
#include "MotorDriverCardImpl.h"

#define INVALID_SPI_READBACK_VALUE 0xFFFFFFFF

namespace mtca4u{

  MotorDriverCardImpl::MotorDriverCardImpl(boost::shared_ptr< devMap<devBase> > const & mappedDevice,
					   MotorDriverConfiguration const & motorDriverConfiguration)
    : _mappedDevice(mappedDevice),
      _spiControlWriteRegister( 
           mappedDevice->getRegObject( SPI_CONTROL_WRITE_ADDRESS_STRING )) ,
      _spiControlReadbackRegister(
	   mappedDevice->getRegObject( SPI_CONTROL_READBACK_ADDRESS_STRING )),
      _powerMonitor(new PowerMonitor)
  {
    
  }

  MotorControler & MotorDriverCardImpl::getMotorControler(unsigned int motorControlerID){
    try{
      return *(_motorControlers.at(motorControlerID));
    }
    catch(std::out_of_range &e){
      std::stringstream errorMessage;
      errorMessage << "motorControlerID " << motorControlerID << " is too large. "
		   << e.what();
      throw MotorDriverException(errorMessage.str(),
				 MotorDriverException::WRONG_MOTOR_ID);
    }
  }

  PowerMonitor & MotorDriverCardImpl::getPowerMonitor(){
    return *_powerMonitor;
  }

  unsigned int MotorDriverCardImpl::getControlerChipVersion(){
    return spiRead( SMDA_COMMON, JDX_CHIP_VERSION ).getDATA();
  }

  TMC429OutputWord MotorDriverCardImpl::spiRead( unsigned int smda, unsigned int idx_jdx )
  {
    // Although the first half is almost identical to the spiWrite,
    // the preparation of the TMC429InputWord is different. So we accept a little 
    // code duplication here.
    TMC429InputWord readRequest;
    readRequest.setSMDA(smda);
    readRequest.setIDX_JDX(idx_jdx);
    readRequest.setRW(RW_READ);

    spiWrite(readRequest);

    int readbackValue;
      _spiControlReadbackRegister.readReg( & readbackValue, sizeof(int));
    // Wait until the correct data word appears in the readback register.
    // Until the word has been transmitted from the controller to the FPGA whith the 
    // PCIe end point, this word reads 0xFFFFFFFF.
    // Limit the loop to 10 attempts with idle time in between
    // FIXME: Hard coded number of attempts, time and invalid word
    
    size_t readbackLoopCounter;
    for (readbackLoopCounter=0; 
	 (static_cast<unsigned int>(readbackValue)!=INVALID_SPI_READBACK_VALUE) && (readbackLoopCounter < 10);
	 ++readbackLoopCounter){
      // sleep somehow
      _spiControlReadbackRegister.readReg( & readbackValue, sizeof(int));
    }

    if (static_cast<unsigned int>(readbackValue)==INVALID_SPI_READBACK_VALUE){
      throw( MotorDriverException("spi read timed out", MotorDriverException::SPI_READ_TIMEOUT) ); 
    }

    return TMC429OutputWord(static_cast<unsigned int>(readbackValue));
  }

  void MotorDriverCardImpl::spiWrite( unsigned int smda, unsigned int idx_jdx, 
				      unsigned int data )
  {
    TMC429InputWord writeMe;
    writeMe.setSMDA(smda);
    writeMe.setIDX_JDX(idx_jdx);
    writeMe.setRW(RW_WRITE);
    writeMe.setDATA(data);
    
    spiWrite(writeMe);
  }

  void MotorDriverCardImpl::spiWrite( TMC429InputWord const & writeWord ){
    // we need this intermediate variable because the write function needs a pointer
    // to int.
    int temporaryWriteInt = static_cast<int>(writeWord.getDataWord());
    _spiControlWriteRegister.writeReg( &temporaryWriteInt, sizeof(int));
  }

  void MotorDriverCardImpl::setDatagramLowWord(unsigned int datagramLowWord){
    spiWrite( SMDA_COMMON, JDX_DATAGRAM_LOW_WORD, datagramLowWord );
  }

  unsigned int MotorDriverCardImpl::getDatagramLowWord(){
    return spiRead( SMDA_COMMON, JDX_DATAGRAM_LOW_WORD ).getDATA();
  }

  void MotorDriverCardImpl::setDatagramHighWord(unsigned int datagramHighWord){
    spiWrite( SMDA_COMMON, JDX_DATAGRAM_HIGH_WORD, datagramHighWord );
  }
   
  unsigned int MotorDriverCardImpl::getDatagramHighWord(){
    return spiRead( SMDA_COMMON, JDX_DATAGRAM_HIGH_WORD ).getDATA();
  }

  void MotorDriverCardImpl::setCoverPositionAndLength(
		CoverPositionAndLength const & coverPositionAndLength){
    throw NotImplementedException("setCoverPositionAndLength not implemented yet");
  }

  CoverPositionAndLength MotorDriverCardImpl::getCoverPositionAndLength(){
    throw NotImplementedException("getCoverPositionAndLength not implemented yet");
  }

  void MotorDriverCardImpl::setCoverDatagram(unsigned int coverDatagram){
    throw NotImplementedException("setCoverDatagram not implemented yet");
  }
    
  unsigned int MotorDriverCardImpl::getCoverDatagram(){
    throw NotImplementedException("getCoverDatagram not implemented yet");
  }
 
  void MotorDriverCardImpl::setStepperMotorGlobalParametersRegister(
	   StepperMotorGlobalParameters const & stepperMotorGlobalParameters){
    throw NotImplementedException("setStepperMotorGlobalParametersRegister not implemented yet");
  }

  StepperMotorGlobalParameters MotorDriverCardImpl::getStepperMotorGlobalParametersRegister(){
    throw NotImplementedException("getStepperMotorGlobalParametersRegister not implemented yet");
  }

  void MotorDriverCardImpl::setInterfaceConfiguration(
	   InterfaceConfiguration const & interfaceConfiguration){
    throw NotImplementedException("setInterfaceConfiguration not implemented yet");
  }

  InterfaceConfiguration MotorDriverCardImpl::getInterfaceConfiguration(){
    throw NotImplementedException("getInterfaceConfiguration not implemented yet");
  }

  void MotorDriverCardImpl::setPositionCompareRegister(
           unsigned int positionCompareWord){
    throw NotImplementedException("setPositionCompareRegister not implemented yet");
  }

  unsigned int MotorDriverCardImpl::getPositionCompareRegister(){
    throw NotImplementedException("getPositionCompareRegister not implemented yet");
  }

  void MotorDriverCardImpl::setPositionCompareInterruptRegister(
	PositionCompareInterruptData const & positionCompareInterruptData ){
    throw NotImplementedException("setPositionCompareInterruptRegister not implemented yet"); 
 }

  PositionCompareInterruptData MotorDriverCardImpl::getPositionCompareInterruptRegister(){
    throw NotImplementedException("setPositionCompareInterruptRegister not implemented yet"); 
 }

  void MotorDriverCardImpl::powerDown(){
    throw NotImplementedException("powerDown not implemented yet");
  }

  ReferenceSwitchData MotorDriverCardImpl::getReferenceSwitchRegister(){
    return  ReferenceSwitchData(spiRead( SMDA_COMMON, JDX_REFERENCE_SWITCH ).getDATA());
  }

}// namespace mtca4u

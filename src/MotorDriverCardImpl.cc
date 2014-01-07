#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <sstream>
#include <ctime>
#include <cerrno>

#include "NotImplementedException.h"
#include "MotorDriverCardImpl.h"
using namespace mtca4u::tmc429;

#include "DFMC_MD22Constants.h"
using namespace mtca4u::dfmc_md22;

namespace mtca4u{
  MotorDriverCardImpl::MotorDriverCardImpl(boost::shared_ptr< devMap<devBase> > const & mappedDevice,
					   MotorDriverCardConfig const & cardConfiguration)
    : _mappedDevice(mappedDevice),
      _controlerSpiWriteRegister( 
           mappedDevice->getRegObject( CONTROLER_SPI_WRITE_ADDRESS_STRING )) ,
      _controlerSpiReadbackRegister(
	   mappedDevice->getRegObject( CONTROLER_SPI_READBACK_ADDRESS_STRING )),
      _powerMonitor(new PowerMonitor),
      _spiCommunicationSleepTime(SPI_COMMUNICATION_DEFAULT_SLEEP_TIME)
  {
    // initialise common registers
    setCoverDatagram( cardConfiguration.coverDatagram );
    setCoverPositionAndLength( cardConfiguration.coverPositionAndLength );
    setDatagramHighWord( cardConfiguration.datagramHighWord );
    setDatagramLowWord( cardConfiguration.datagramLowWord );
    setInterfaceConfiguration( cardConfiguration.interfaceConfiguration );
    setPositionCompareInterruptData( cardConfiguration.positionCompareInterruptData );
    setPositionCompareWord( cardConfiguration.positionCompareWord );
    setStepperMotorGlobalParameters( cardConfiguration.stepperMotorGlobalParameters );

    // initialise motors
    _motorControlers.resize( N_MOTORS_MAX );
    for (unsigned int i = 0; i < _motorControlers.size() ; ++i){
      _motorControlers[i].reset( new MotorControler( i, *this,
						     cardConfiguration.motorControlerConfigurations[i]) );
    }
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
    return controlerSpiRead( SMDA_COMMON, JDX_CHIP_VERSION ).getDATA();
  }

  TMC429OutputWord MotorDriverCardImpl::controlerSpiRead( unsigned int smda, unsigned int idx_jdx )
  {
    // Although the first half is almost identical to the controlerSpiWrite,
    // the preparation of the TMC429InputWord is different. So we accept a little 
    // code duplication here.
    TMC429InputWord readRequest;
    readRequest.setSMDA(smda);
    readRequest.setIDX_JDX(idx_jdx);
    readRequest.setRW(RW_READ);

    controlerSpiWrite(readRequest);

    int readbackValue;
      _controlerSpiReadbackRegister.readReg( & readbackValue, sizeof(int));
    // Wait until the correct data word appears in the readback register.
    // Until the word has been transmitted from the controller to the FPGA whith the 
    // PCIe end point, this word reads 0xFFFFFFFF.
    // Limit the loop to 10 attempts with idle time in between
    // FIXME: Hard coded number of attempts, time and invalid word
    
    size_t readbackLoopCounter;
    for (readbackLoopCounter=0; 
	 (static_cast<unsigned int>(readbackValue)==INVALID_SPI_READBACK_VALUE) && (readbackLoopCounter < 10);
	 ++readbackLoopCounter){
      sleepMicroSeconds(_spiCommunicationSleepTime);
      _controlerSpiReadbackRegister.readReg( & readbackValue, sizeof(int));
    }

    if (static_cast<unsigned int>(readbackValue)==INVALID_SPI_READBACK_VALUE){
      throw( MotorDriverException("spi read timed out", MotorDriverException::SPI_READ_TIMEOUT) ); 
    }

    return TMC429OutputWord(static_cast<unsigned int>(readbackValue));
  }

  void MotorDriverCardImpl::sleepMicroSeconds(unsigned int microSeconds){
    timespec sleepTime;
    sleepTime.tv_sec = microSeconds / 1000000;
    sleepTime.tv_nsec = (microSeconds%1000000)*1000;

    timespec remainingTime;
    remainingTime.tv_sec = 0;
    remainingTime.tv_nsec = 0;
    if ( nanosleep(&sleepTime, &remainingTime) ){
      if (errno == EFAULT){
	std::stringstream errorMessage;
	errorMessage << "Error sleeping " << microSeconds << " micro seconds!";
	throw( MotorDriverException(errorMessage.str(),
				    MotorDriverException::SPI_READ_TIMEOUT));
      }
    }
  }

  void MotorDriverCardImpl::controlerSpiWrite( unsigned int smda, unsigned int idx_jdx, 
				      unsigned int data )
  {
    TMC429InputWord writeMe;
    writeMe.setSMDA(smda);
    writeMe.setIDX_JDX(idx_jdx);
    writeMe.setRW(RW_WRITE);
    writeMe.setDATA(data);
    
    controlerSpiWrite(writeMe);
  }

  void MotorDriverCardImpl::controlerSpiWrite( TMC429InputWord const & writeWord ){
    // FIXME:
    // lock the controler SPI register using a separate mutex. Access to other registers
    // of the device is allowed, but SPI has to wait until the word has been processed.

    // we need this intermediate variable because the write function needs a pointer
    // to int.
    int temporaryWriteInt = static_cast<int>(writeWord.getDataWord());
    _controlerSpiWriteRegister.writeReg( &temporaryWriteInt, sizeof(int));

    // sleep to make (reasonably) sure the word has been written
    // FIXME: Make absolutely sure the word has been written to the controler by introducing an SPI
    // Sync register
    sleepMicroSeconds(_spiCommunicationSleepTime);
    
  }

  void MotorDriverCardImpl::setDatagramLowWord(unsigned int datagramLowWord){
    controlerSpiWrite( SMDA_COMMON, JDX_DATAGRAM_LOW_WORD, datagramLowWord );
  }

  unsigned int MotorDriverCardImpl::getDatagramLowWord(){
    return controlerSpiRead( SMDA_COMMON, JDX_DATAGRAM_LOW_WORD ).getDATA();
  }

  void MotorDriverCardImpl::setDatagramHighWord(unsigned int datagramHighWord){
    controlerSpiWrite( SMDA_COMMON, JDX_DATAGRAM_HIGH_WORD, datagramHighWord );
  }
   
  unsigned int MotorDriverCardImpl::getDatagramHighWord(){
    return controlerSpiRead( SMDA_COMMON, JDX_DATAGRAM_HIGH_WORD ).getDATA();
  }

  void MotorDriverCardImpl::setCoverPositionAndLength(
		CoverPositionAndLength const & coverPositionAndLength){
    controlerSpiWrite( coverPositionAndLength );
  }

  CoverPositionAndLength MotorDriverCardImpl::getCoverPositionAndLength(){
    return CoverPositionAndLength( controlerSpiRead(SMDA_COMMON, JDX_COVER_POSITION_AND_LENGTH).getDATA());
  }

  void MotorDriverCardImpl::setCoverDatagram(unsigned int coverDatagram){
    controlerSpiWrite( SMDA_COMMON, JDX_COVER_DATAGRAM, coverDatagram );
  }
    
  unsigned int MotorDriverCardImpl::getCoverDatagram(){
    return controlerSpiRead( SMDA_COMMON, JDX_COVER_DATAGRAM ).getDATA();
  }
 
  void MotorDriverCardImpl::setStepperMotorGlobalParameters(
	   StepperMotorGlobalParameters const & stepperMotorGlobalParameters){
    controlerSpiWrite( stepperMotorGlobalParameters );
  }

  StepperMotorGlobalParameters MotorDriverCardImpl::getStepperMotorGlobalParameters(){
    return StepperMotorGlobalParameters( controlerSpiRead(SMDA_COMMON, JDX_STEPPER_MOTOR_GLOBAL_PARAMETERS).getDATA());    
  }

  void MotorDriverCardImpl::setInterfaceConfiguration(
	   InterfaceConfiguration const & interfaceConfiguration){
    controlerSpiWrite( interfaceConfiguration );
  }

  InterfaceConfiguration MotorDriverCardImpl::getInterfaceConfiguration(){
    return InterfaceConfiguration( controlerSpiRead(SMDA_COMMON, JDX_INTERFACE_CONFIGURATION).getDATA());    
  }

  void MotorDriverCardImpl::setPositionCompareWord(
           unsigned int positionCompareWord){
    controlerSpiWrite( SMDA_COMMON, JDX_POSITION_COMPARE, positionCompareWord );
  }

  unsigned int MotorDriverCardImpl::getPositionCompareWord(){
    return controlerSpiRead( SMDA_COMMON, JDX_POSITION_COMPARE ).getDATA();
  }

  void MotorDriverCardImpl::setPositionCompareInterruptData(
	PositionCompareInterruptData const & positionCompareInterruptData ){
    return controlerSpiWrite( positionCompareInterruptData );
 }

  PositionCompareInterruptData MotorDriverCardImpl::getPositionCompareInterruptData(){
    return PositionCompareInterruptData( controlerSpiRead(SMDA_COMMON, JDX_POSITION_COMPARE_INTERRUPT).getDATA());    
 }

  void MotorDriverCardImpl::powerDown(){
    controlerSpiWrite( SMDA_COMMON, JDX_POWER_DOWN, 1 );
  }

  ReferenceSwitchData MotorDriverCardImpl::getReferenceSwitchData(){
    return  ReferenceSwitchData(controlerSpiRead( SMDA_COMMON, JDX_REFERENCE_SWITCH ).getDATA());
  }

  void MotorDriverCardImpl::setSpiCommunicationSleepTime(unsigned int microSeconds){
    _spiCommunicationSleepTime = microSeconds;
  }
    
  unsigned int MotorDriverCardImpl::getSpiCommunicationSleepTime() const{
    return _spiCommunicationSleepTime;
  }

}// namespace mtca4u

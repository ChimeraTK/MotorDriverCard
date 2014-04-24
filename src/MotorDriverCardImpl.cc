#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <sstream>
#include <ctime>
#include <cerrno>

#include <MtcaMappedDevice/NotImplementedException.h>
#include "MotorDriverException.h" 
#include "MotorDriverCardImpl.h"
using namespace mtca4u::tmc429;

#include "DFMC_MD22Constants.h"
using namespace mtca4u::dfmc_md22;

namespace mtca4u{
  MotorDriverCardImpl::MotorDriverCardImpl(boost::shared_ptr< devMap<devBase> > const & mappedDevice,
					   MotorDriverCardConfig const & cardConfiguration)
    : _mappedDevice(mappedDevice),
      _powerMonitor(new PowerMonitor),
      _controlerSPIviaPCIe( mappedDevice, CONTROLER_SPI_WRITE_ADDRESS_STRING,
			    CONTROLER_SPI_SYNC_ADDRESS_STRING,  CONTROLER_SPI_READBACK_ADDRESS_STRING,
			    cardConfiguration.controlerSpiWaitingTime )
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

    return TMC429OutputWord( _controlerSPIviaPCIe.read( readRequest.getDataWord() ) );
  }

  void MotorDriverCardImpl::controlerSpiWrite( unsigned int smda, unsigned int idx_jdx, 
				      unsigned int data )
  {
    TMC429InputWord writeMe;
    writeMe.setSMDA(smda);
    writeMe.setIDX_JDX(idx_jdx);
    writeMe.setRW(RW_WRITE);
    writeMe.setDATA(data);
    
    controlerSpiWrite( writeMe );
  }

  void MotorDriverCardImpl::controlerSpiWrite( TMC429InputWord const & writeWord ){
    _controlerSPIviaPCIe.write( writeWord.getDataWord() );
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

}// namespace mtca4u

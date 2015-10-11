#include "DFMC_MD22Dummy.h"
#include "DFMC_MD22Constants.h"
using namespace mtca4u::dfmc_md22;
#include <mtca4u/NotImplementedException.h>
#include "MotorDriverException.h"

#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>

#include "TMC429Words.h"
#include "TMC260Words.h"
#include "TMC429DummyConstants.h"
#include "TMC260DummyConstants.h"
using namespace mtca4u::tmc429;

#define DEFINE_ADDRESS_RANGE( NAME , ADDRESS_STRING , MOD_NAME )\
    _registerMapping->getRegisterInfo( ADDRESS_STRING, registerInformation, MOD_NAME );\
    AddressRange NAME( registerInformation.reg_bar, \
                       registerInformation.reg_address, \
		       registerInformation.reg_size)

//#define MODULE_NAME_0 "MD220"


namespace mtca4u{

 DFMC_MD22Dummy::DFMC_MD22Dummy(std::string const & mapFileName, std::string const & moduleName)
 	 	 	 	 	 : DummyBackend(mapFileName),
 				   _powerIsUp(true), _causeSpiTimeouts(false),
 				   _causeSpiErrors(false),
 				   _microsecondsControllerSpiDelay(tmc429::DEFAULT_DUMMY_SPI_DELAY),
 				   _microsecondsDriverSpiDelay(tmc260::DEFAULT_DUMMY_SPI_DELAY),
 	 	 	 	   _moduleName(moduleName)
           {


  }


  void DFMC_MD22Dummy::open(){
std::cout<<"open dev"<<std::endl;
      DummyBackend::open();
    setPCIeRegistersForTesting(); // some of them will be overwriten if a 
    // defined behaviour exists for them
    
    _controlerSpiAddressSpace.resize( tmc429::SIZE_OF_SPI_ADDRESS_SPACE , 0 );

    setControlerSpiRegistersForOperation();

    RegisterInfoMap::RegisterInfo registerInformation;
    DEFINE_ADDRESS_RANGE( controlerSpiWriteAddressRange, CONTROLER_SPI_WRITE_ADDRESS_STRING, _moduleName );
    _controlerSpiWriteAddress = registerInformation.reg_address;
    _controlerSpiBar = registerInformation.reg_bar;
    
    _registerMapping->getRegisterInfo( CONTROLER_SPI_READBACK_ADDRESS_STRING, registerInformation, _moduleName );
    if ( _controlerSpiBar != registerInformation.reg_bar ){
      throw MotorDriverException("SPI write and readback address must be in the same bar",
				 MotorDriverException::SPI_ERROR );
    }
    _controlerSpiReadbackAddress = registerInformation.reg_address;
    std::cout<<"registerInformation.reg_size:"<<registerInformation.reg_size<<std::endl;
    setReadOnly( registerInformation.reg_bar, registerInformation.reg_address, registerInformation.reg_size);

    _registerMapping->getRegisterInfo( CONTROLER_SPI_SYNC_ADDRESS_STRING, registerInformation, _moduleName );
    if ( _controlerSpiBar != registerInformation.reg_bar ){
      throw MotorDriverException("SPI write and sync address must be in the same bar",
				 MotorDriverException::SPI_ERROR );
    }
    _controlerSpiSyncAddress = registerInformation.reg_address;
    
    setWriteCallbackFunction( controlerSpiWriteAddressRange, boost::bind( &DFMC_MD22Dummy::handleControlerSpiWrite, this ) );


    _driverSPIs.resize( N_MOTORS_MAX );
    for (unsigned int id = 0; id < N_MOTORS_MAX ; ++id){
      DEFINE_ADDRESS_RANGE( actualPositionAddressRange,
			    createMotorRegisterName( id, ACTUAL_POSITION_SUFFIX ), _moduleName );
      setReadOnly( actualPositionAddressRange );
      DEFINE_ADDRESS_RANGE( actualVelocityAddressRange,
			    createMotorRegisterName( id, ACTUAL_VELOCITY_SUFFIX ), _moduleName );
      setReadOnly( actualVelocityAddressRange );
      DEFINE_ADDRESS_RANGE( actualAccelerationAddressRange,
			    createMotorRegisterName( id, ACTUAL_ACCELETATION_SUFFIX ), _moduleName );
      setReadOnly( actualAccelerationAddressRange );
      DEFINE_ADDRESS_RANGE( microStepCountAddressRange,
			    createMotorRegisterName( id, MICRO_STEP_COUNT_SUFFIX ), _moduleName );
      setReadOnly( microStepCountAddressRange );

      _driverSPIs[id].addressSpace.resize(tmc260::SIZE_OF_SPI_ADDRESS_SPACE, 0);
      setDriverSpiRegistersForTesting(id);

      DEFINE_ADDRESS_RANGE( driverSpiWriteAddressRange,
			    createMotorRegisterName( id, SPI_WRITE_SUFFIX ), _moduleName );
      DEFINE_ADDRESS_RANGE( driverSpiSyncAddressRange,
			    createMotorRegisterName( id, SPI_SYNC_SUFFIX ), _moduleName );
      if ( driverSpiWriteAddressRange.bar != driverSpiSyncAddressRange.bar ){
	throw MotorDriverException("SPI write and sync address must be in the same bar",
				   MotorDriverException::SPI_ERROR );
      }

      _driverSPIs[id].bar = static_cast<unsigned int>(driverSpiWriteAddressRange.bar);
      _driverSPIs[id].pcieWriteAddress = driverSpiWriteAddressRange.address;
      _driverSPIs[id].pcieSyncAddress = driverSpiSyncAddressRange.address;
      setWriteCallbackFunction( driverSpiWriteAddressRange, 
				boost::bind( &DFMC_MD22Dummy::handleDriverSpiWrite, this, id) );
    }
  }

  void DFMC_MD22Dummy::setPCIeRegistersForTesting(){
    for (std::map< uint8_t, std::vector<int32_t> >::iterator barIter = _barContents.begin();
	 barIter != _barContents.end(); ++barIter ){
      for (unsigned int i = 0; i < barIter->second.size(); ++i){
	unsigned int address = sizeof(uint32_t)*i;
	barIter->second[i] = 3*address*address+17;
      }
    }
    
    // now reset the known standard registers to their normal valued
    setStandardRegisters();
  }

  void DFMC_MD22Dummy::setStandardRegisters(){
    setConstantPCIeRegister(PROJECT_MAGIC_ADDRESS_STRING,  PROJECT_MAGIC_NUMBER);
    setConstantPCIeRegister(PROJECT_ID_ADDRESS_STRING, PROJECT_ID);
    // the dummy firmware has patch level 0xFFFF
    resetFirmwareVersion(); // call this special function to avoid code duplication in the firmware calculation
    // date and reserved are left at the debug values
    setConstantPCIeRegister(PROJECT_RESET_ADDRESS_STRING, 0); // project is not in reset. FIXME: react on user setting to 1
   setConstantPCIeRegister(PROJECT_NEXT_ADDRESS_STRING, 0); // There is no next project
  }

  void DFMC_MD22Dummy::setFirmwareVersion(uint32_t version){
    setConstantPCIeRegister(PROJECT_VERSION_ADDRESS_STRING, version);
  }

  void DFMC_MD22Dummy::resetFirmwareVersion(){
    setConstantPCIeRegister(PROJECT_VERSION_ADDRESS_STRING, MINIMAL_FIRMWARE_VERSION | 0xFFFF);
  }

  void DFMC_MD22Dummy::setConstantPCIeRegister( std::string registerName, int32_t content){
    RegisterInfoMap::RegisterInfo registerInformation; // variable neede in the DEFINE_ADDRESS_RANGE macro
    DEFINE_ADDRESS_RANGE( addressRange, registerName, _moduleName);
    size_t addressIndex = addressRange.address/sizeof(uint32_t);
    _barContents[addressRange.bar][addressIndex] = content;
    // only set the one word we modified as constant. The address range might be larger
    std::cout<<"setting PCIe readonly_moduleName:"<<_moduleName<<std::endl;
    std::cout<<"registerName:"<<registerName<<std::endl;
    std::cout<<"addressRange.bar:"<<unsigned(addressRange.bar)<<std::endl;
    std::cout<<"addressRange.address:"<<addressRange.address<<std::endl;
    setReadOnly( addressRange.bar, addressRange.address, 1);
    std::cout<<"--return"<<std::endl;
  }

  void DFMC_MD22Dummy::setDriverSpiRegistersForTesting(unsigned int motorID){
    for(unsigned int address = 0; address < _driverSPIs[motorID].addressSpace.size(); ++address){
      _driverSPIs[motorID].addressSpace[address]= tmc260::testWordFromSpiAddress(address, motorID);
    }
  }

  void DFMC_MD22Dummy::setControlerSpiRegistersForOperation(){
    std::cout<<"setControlerSpiRegistersForOperation"<<std::endl;
	  for_each( _controlerSpiAddressSpace.begin(), _controlerSpiAddressSpace.end(), boost::lambda::_1=0 );

    // Information from the data sheet:
    size_t controlerSpiAddress =  spiAddressFromSmdaIdxJdx( SMDA_COMMON, JDX_CHIP_VERSION );
    _controlerSpiAddressSpace[controlerSpiAddress] = CONTROLER_CHIP_VERSION;

    // the CLK2_DIV subword is 15, all other bits stay 0
    controlerSpiAddress = spiAddressFromSmdaIdxJdx( SMDA_COMMON, JDX_STEPPER_MOTOR_GLOBAL_PARAMETERS );
    StepperMotorGlobalParameters globalParameters;
    globalParameters.setClk2_div(15);
    _controlerSpiAddressSpace.at(controlerSpiAddress) = globalParameters.getDATA();

    synchroniseFpgaWithControlerSpiRegisters();
    std::cout<<"setControlerSpiRegistersForOperation return"<<std::endl;
  }

  void DFMC_MD22Dummy::setControlerSpiRegistersForTesting(){
    for( size_t i=0; i< _controlerSpiAddressSpace.size(); ++i ){
      _controlerSpiAddressSpace[i]=i*i+13;
    }

    synchroniseFpgaWithControlerSpiRegisters();
  }

  void DFMC_MD22Dummy::handleControlerSpiWrite(){
    //debug functionality: cause timeouts by ignoring spi writes
    if (_causeSpiTimeouts){
      return;
    }
    int32_t value = SPI_SYNC_ERROR;
    //debug functionality: cause errors
    if (_causeSpiErrors){
      write(_controlerSpiBar, _controlerSpiSyncAddress, &value,  4);
      return;
    }

    int32_t controlerSpiSyncWord;
    read(_controlerSpiBar, _controlerSpiSyncAddress, &controlerSpiSyncWord,  4);

    if (controlerSpiSyncWord != SPI_SYNC_REQUESTED){
      write(_controlerSpiBar, _controlerSpiSyncAddress, &value,  4);
      return;
    }

    int32_t controlerSpiWriteWord;
    read(_controlerSpiBar, _controlerSpiWriteAddress, &controlerSpiWriteWord,  4);

    TMC429InputWord inputWord(controlerSpiWriteWord);
    uint32_t controlerSpiAddress = inputWord.getADDRESS();
    if (inputWord.getRW() == RW_READ){
      writeControlerSpiContentToReadbackRegister(controlerSpiAddress);
      // it takes some time to perform this in firmware. Implement a short delay,
      // which proabaly will cause rescheduling, twice for write and read
      boost::this_thread::sleep(boost::posix_time::microseconds(2*_microsecondsControllerSpiDelay));
    }
    else{
      writeContentToControlerSpiRegister(inputWord.getDATA(), controlerSpiAddress);
      // it takes some time to perform this in firmware. Implement a short delay,
      // which proabaly will cause rescheduling.
      boost::this_thread::sleep(boost::posix_time::microseconds(_microsecondsControllerSpiDelay));
    }

    triggerActionsOnControlerSpiWrite(inputWord);
    value = SPI_SYNC_OK;
    // SPI action done, write sync ok to finish the handshake
    write(_controlerSpiBar, _controlerSpiSyncAddress, &value, 4);
  }

  void DFMC_MD22Dummy::handleDriverSpiWrite(unsigned int ID){
    //debug functionality: cause timeouts by ignoring spi writes
    if (_causeSpiTimeouts){
      return;
    }

    unsigned int bar =  _driverSPIs[ID].bar;
    unsigned int writeIndex =  _driverSPIs[ID].pcieWriteAddress/sizeof(int32_t);
    unsigned int syncIndex =  _driverSPIs[ID].pcieSyncAddress/sizeof(int32_t);

    //debug functionality: cause errors
    if (_causeSpiErrors){
      _barContents[bar].at( syncIndex ) = SPI_SYNC_ERROR;
      return;
    }

    // check the sync register
    if ( _barContents[bar].at( syncIndex ) != SPI_SYNC_REQUESTED){
      _barContents[bar].at( syncIndex ) = SPI_SYNC_ERROR;
      return;
    }

    // perform the SPI transfer
    unsigned int writtenSpiWord = _barContents[bar].at(writeIndex);
    unsigned int spiAddress = tmc260::spiAddressFromDataWord(writtenSpiWord);
    unsigned int payloadDataMask = tmc260::dataMaskFromSpiAddress(spiAddress);

    boost::this_thread::sleep(boost::posix_time::microseconds(_microsecondsDriverSpiDelay));

    _driverSPIs[ID].addressSpace.at(spiAddress) = writtenSpiWord & payloadDataMask;
    
    driverSpiActions(ID, spiAddress, writtenSpiWord & payloadDataMask);

    // SPI transfer ready, set the sync register to ok
    _barContents[bar].at( syncIndex ) = SPI_SYNC_OK;    
  }

  void DFMC_MD22Dummy::driverSpiActions(unsigned int ID, unsigned int spiAddress, unsigned int payloadData){
    switch( spiAddress ){
    case tmc260::ADDRESS_STALL_GUARD_CONFIG:
	// according to the data sheet the cool step value (read response) is between 1/4 or 1/2 
	// (depending on the SEMIN bit in the coolStep control register) and 1/1 of the current scaling value.
	// Just setting it to 1/1 (which seems to be happenening during my tests) should be fine
	StallGuardControlData stallGuardControlData(payloadData);
	setConstantPCIeRegister( createMotorRegisterName( ID, COOL_STEP_VALUE_SUFFIX),
				 stallGuardControlData.getCurrentScale() );
    }
  }

  void DFMC_MD22Dummy::writeContentToControlerSpiRegister(unsigned int content,
						 unsigned int controlerSpiAddress){
    if (_powerIsUp){
      _controlerSpiAddressSpace.at(controlerSpiAddress)=content;
    }
  }

  void DFMC_MD22Dummy::writeControlerSpiContentToReadbackRegister(unsigned int controlerSpiAddress){
    TMC429OutputWord outputWord;
    // FIXME: set the status bits correctly. We currently leave them at 0;
    outputWord.setDATA( _controlerSpiAddressSpace.at(controlerSpiAddress) );
     writeRegisterWithoutCallback( _controlerSpiReadbackAddress,
				   outputWord.getDataWord(),
				   _controlerSpiBar );
  }

  void DFMC_MD22Dummy::triggerActionsOnControlerSpiWrite(TMC429InputWord const & inputWord){
    switch(inputWord.getSMDA()){
      case SMDA_COMMON: // common registers are addressed by jdx
	performJdxActions( inputWord.getIDX_JDX() );
	break;
      default: // motor specifix registers are addressed by idx
	performIdxActions( inputWord.getIDX_JDX() );
    };

    synchroniseFpgaWithControlerSpiRegisters();
  }

  void DFMC_MD22Dummy::performJdxActions( unsigned int jdx ){
    switch(jdx){
      case( JDX_POWER_DOWN ):
	_powerIsUp = false;
	// stop internal cock thread when implemented
	break;
      default:
	;// do nothing
    }
  }

  void DFMC_MD22Dummy::performIdxActions( unsigned int /*idx*/ ){
    // no actions defined at the moment
  }

  bool DFMC_MD22Dummy::isPowerUp(){
    return _powerIsUp;
  }

  void DFMC_MD22Dummy::powerUp(){
    unsigned int powerDownControlerSpiAddress = spiAddressFromSmdaIdxJdx( SMDA_COMMON,
								 JDX_POWER_DOWN );
    writeContentToControlerSpiRegister( 0, powerDownControlerSpiAddress );
    _powerIsUp = true;
  }

  void DFMC_MD22Dummy::synchroniseFpgaWithControlerSpiRegisters(){
    for (unsigned int i = 0; i < N_MOTORS_MAX ; ++i){
      writeControlerSpiRegisterToFpga( i, IDX_ACTUAL_POSITION , ACTUAL_POSITION_SUFFIX );
      writeControlerSpiRegisterToFpga( i, IDX_ACTUAL_VELOCITY, ACTUAL_VELOCITY_SUFFIX );
      writeControlerSpiRegisterToFpga( i, IDX_ACTUAL_ACCELERATION, ACTUAL_ACCELETATION_SUFFIX );
      //writeAccelerationThresholdToFpgs( i );
      writeControlerSpiRegisterToFpga( i, IDX_MICRO_STEP_COUNT,  MICRO_STEP_COUNT_SUFFIX);
    }
  }

  void DFMC_MD22Dummy::writeControlerSpiRegisterToFpga( unsigned int ID, unsigned int IDX, std::string suffix ){
    unsigned int controlerSpiAddress = spiAddressFromSmdaIdxJdx( ID, IDX );
    std::string registerName = createMotorRegisterName( ID, suffix );
    RegisterInfoMap::RegisterInfo registerInformation;
    _registerMapping->getRegisterInfo (registerName, registerInformation, _moduleName);
     writeRegisterWithoutCallback(  registerInformation.reg_bar , registerInformation.reg_address,
				   _controlerSpiAddressSpace[controlerSpiAddress]
				  );
  }

  unsigned int DFMC_MD22Dummy::readDriverSpiRegister( unsigned int motorID, unsigned int driverSpiAddress ){
    return _driverSPIs.at(motorID).addressSpace.at(driverSpiAddress);
  }
  //_WORD_M1_THRESHOLD_ACCEL

  void  DFMC_MD22Dummy::setRegistersForTesting(){
    // FIXME: store the current config to be able to restore it later

  setPCIeRegistersForTesting();
  setControlerSpiRegistersForTesting();

    for (unsigned int id = 0; id < N_MOTORS_MAX ; ++id){
      setDriverSpiRegistersForTesting(id);
    }
  }

  void  DFMC_MD22Dummy::causeSpiTimeouts(bool causeTimeouts){
    _causeSpiTimeouts = causeTimeouts;
  }

  void  DFMC_MD22Dummy::causeSpiErrors(bool causeErrors){
    _causeSpiErrors = causeErrors;
  }

  boost::shared_ptr<mtca4u::DeviceBackend> DFMC_MD22Dummy::createInstance(std::string /*host*/, std::string /*interface*/, std::list<std::string> parameters) {
    #ifdef _DEBUG
    	std::cout<<"DFMC_MD22Dummy createInstance"<<std::endl;
    #endif
    	//fixme create DFMC_MD22DummyException
    	if (parameters.empty()){
    	    throw DummyBackendException("No moudle name given in the parameter list.",
    	        DummyBackendException::INVALID_PARAMETER);
    	}
    	if (parameters.size() < 2){
    	  throw DummyBackendException("No map file given in the parameter list.",
    	              DummyBackendException::INVALID_PARAMETER);
    	}
    	return boost::shared_ptr<DeviceBackend> ( new DFMC_MD22Dummy(parameters.front(),parameters.back()) );
  }
}// namespace mtca4u

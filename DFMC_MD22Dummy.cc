#include "DFMC_MD22Dummy.h"
#include "DFMC_MD22Constants.h"
using namespace mtca4u::dfmc_md22;
#include "NotImplementedException.h"

#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>

#include "TMC429Words.h"
#include "TMC260Words.h"
#include "TMC429DummyConstants.h"
#include "TMC260DummyConstants.h"
using namespace mtca4u::tmc429;

#define DEFINE_ADDRESS_RANGE( NAME , ADDRESS_STRING )\
    _registerMapping->getRegisterInfo( ADDRESS_STRING, registerInformation );\
    AddressRange NAME( registerInformation.reg_address, \
                       registerInformation.reg_size, \
		       registerInformation.reg_bar)

namespace mtca4u{

 DFMC_MD22Dummy::DFMC_MD22Dummy(): DummyDevice(), 
				    _controlerSpiWriteAddress(0), _controlerSpiWriteBar(0),
				    _controlerSpiReadbackAddress(0), _controlerSpiReadbackBar(0),
				    _powerIsUp(true){
  }

  void DFMC_MD22Dummy::openDev(const std::string &mappingFileName, int perm,
			       devConfigBase* pConfig){
    DummyDevice::openDev(mappingFileName, perm, pConfig);
    
    setPCIeRegistersForTesting(); // some of them will be overwriten if a 
    // defined behaviour exists for them

    _controlerSpiAddressSpace.resize( tmc429::SIZE_OF_SPI_ADDRESS_SPACE , 0 );
    setControlerSpiRegistersForOperation();

    mapFile::mapElem registerInformation;
    DEFINE_ADDRESS_RANGE( controlerSpiWriteAddressRange, CONTROLER_SPI_WRITE_ADDRESS_STRING );
    _controlerSpiWriteAddress = registerInformation.reg_address;
    _controlerSpiWriteBar = registerInformation.reg_bar;

    _registerMapping->getRegisterInfo( CONTROLER_SPI_READBACK_ADDRESS_STRING, registerInformation );
    _controlerSpiReadbackAddress = registerInformation.reg_address;
    _controlerSpiReadbackBar = registerInformation.reg_bar;
    setReadOnly(registerInformation.reg_address, registerInformation.reg_size,
		registerInformation.reg_bar);
    
    setWriteCallbackFunction( controlerSpiWriteAddressRange, boost::bind( &DFMC_MD22Dummy::handleControlerSpiWrite, this ) );


    _driverSpiAddressSpaces.resize( N_MOTORS_MAX );
    _driverSpiWriteBarAndAddresses.resize( N_MOTORS_MAX );
    for (unsigned int id = 0; id < N_MOTORS_MAX ; ++id){
      DEFINE_ADDRESS_RANGE( actualPositionAddressRange,
			    createMotorRegisterName( id, ACTUAL_POSITION_SUFFIX ) );
      setReadOnly( actualPositionAddressRange );
      DEFINE_ADDRESS_RANGE( actualVelocityAddressRange,
			    createMotorRegisterName( id, ACTUAL_VELOCITY_SUFFIX ) );
      setReadOnly( actualVelocityAddressRange );
      DEFINE_ADDRESS_RANGE( actualAccelerationAddressRange,
			    createMotorRegisterName( id, ACTUAL_ACCELETATION_SUFFIX ) );
      setReadOnly( actualAccelerationAddressRange );
      DEFINE_ADDRESS_RANGE( microStepCountAddressRange,
			    createMotorRegisterName( id, MICRO_STEP_COUNT_SUFFIX ) );
      setReadOnly( microStepCountAddressRange );

      _driverSpiAddressSpaces[id].resize(tmc260::SIZE_OF_SPI_ADDRESS_SPACE, 0);
      setDriverSpiRegistersForTesting(id);

      DEFINE_ADDRESS_RANGE( driverSpiWriteAddressRange,
			    createMotorRegisterName( id, SPI_WRITE_SUFFIX ) );
      _driverSpiWriteBarAndAddresses[id]=std::make_pair( static_cast<unsigned int>(driverSpiWriteAddressRange.bar),
							 driverSpiWriteAddressRange.offset );
      setWriteCallbackFunction( driverSpiWriteAddressRange, 
				boost::bind( &DFMC_MD22Dummy::handleDriverSpiWrite, this, id) );
    }

    //    setWriteCallbackFunction( 

  }

  void DFMC_MD22Dummy::setPCIeRegistersForTesting(){
    for (std::map< uint8_t, std::vector<int32_t> >::iterator barIter = _barContents.begin();
	 barIter != _barContents.end(); ++barIter ){
      for (unsigned int i = 0; i < barIter->second.size(); ++i){
	unsigned int address = sizeof(uint32_t)*i;
	barIter->second[i] = 3*address*address+17;
      }
    }
  }

  void DFMC_MD22Dummy::setDriverSpiRegistersForTesting(unsigned int motorID){
    for(unsigned int address = 0; address < _driverSpiAddressSpaces[motorID].size(); ++address){
      _driverSpiAddressSpaces[motorID][address]= tmc260::testWordFromSpiAddress(address, motorID);
    }
  }

  void DFMC_MD22Dummy::setControlerSpiRegistersForOperation(){
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
  }

  void DFMC_MD22Dummy::setControlerSpiRegistersForTesting(){
    for( size_t i=0; i< _controlerSpiAddressSpace.size(); ++i ){
      _controlerSpiAddressSpace[i]=i*i+13;
    }

    synchroniseFpgaWithControlerSpiRegisters();
  }

  void DFMC_MD22Dummy::handleControlerSpiWrite(){
    int32_t controlerSpiWriteWord;
    readReg( _controlerSpiWriteAddress, &controlerSpiWriteWord, _controlerSpiWriteBar );

    TMC429InputWord inputWord(controlerSpiWriteWord);
    uint32_t controlerSpiAddress = inputWord.getADDRESS();
    if (inputWord.getRW() == RW_READ){
      writeControlerSpiContentToReadbackRegister(controlerSpiAddress);
    }
    else{
      writeContentToControlerSpiRegister(inputWord.getDATA(), controlerSpiAddress);
    }

    triggerActionsOnControlerSpiWrite(inputWord);
  }

  void DFMC_MD22Dummy::handleDriverSpiWrite(unsigned int ID){
    unsigned int bar =  _driverSpiWriteBarAndAddresses[ID].first;
    unsigned int pcieAddress =  _driverSpiWriteBarAndAddresses[ID].second;
    unsigned int writtenSpiWord = _barContents[bar].at(pcieAddress/sizeof(int32_t));
    unsigned int spiAddress = tmc260::spiAddressFromDataWord(writtenSpiWord);

    _driverSpiAddressSpaces.at(ID).at(spiAddress) = writtenSpiWord;
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
				   _controlerSpiReadbackBar );
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
    mapFile::mapElem registerInformation;
    _registerMapping->getRegisterInfo (registerName, registerInformation);
     writeRegisterWithoutCallback( registerInformation.reg_address,
				   _controlerSpiAddressSpace[controlerSpiAddress], 
				   registerInformation.reg_bar );
  }

  unsigned int DFMC_MD22Dummy::readDriverSpiRegister( unsigned int motorID, unsigned int driverSpiAddress ){
    return _driverSpiAddressSpaces.at(motorID).at(driverSpiAddress);
  }
  //_WORD_M1_THRESHOLD_ACCEL

}// namespace mtca4u

#include "DFMC_MD22Dummy.h"
#include "DFMC_MD22Constants.h"
#include "NotImplementedException.h"

#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lexical_cast.hpp>

#include "TMC429Words.h"
#include "TMC429DummyConstants.h"
using namespace mtca4u::tmc429;

// See data sheet: the spi address has 7 bits, the lower 4 are IDX or JDX, the higher 3 are SMDA
#define SPI_ADDRESS_FROM_SMDA_IDXJDX( SMDA, IDXJDX ) ((SMDA << 4) + IDXJDX)

#define DEFINE_ADDRESS_RANGE( NAME , ADDRESS_STRING )\
    _registerMapping->getRegisterInfo( ADDRESS_STRING, registerInformation );\
    AddressRange NAME( registerInformation.reg_address, \
                       registerInformation.reg_size, \
		       registerInformation.reg_bar)

namespace mtca4u{
 uint32_t const SPI_DATA_MASK = 0xFFFFFF;

 DFMC_MD22Dummy::DFMC_MD22Dummy(): DummyDevice(), 
				    _spiWriteAddress(0), _spiWriteBar(0),
				    _spiReadbackAddress(0), _spiReadbackBar(0),
				    _powerIsUp(true){
  }

  void DFMC_MD22Dummy::openDev(const std::string &mappingFileName, int perm,
			       devConfigBase* pConfig){
    DummyDevice::openDev(mappingFileName, perm, pConfig);

    _spiAddressSpace.resize( SIZE_OF_SPI_ADDRESS_SPACE , 0 );
    setSPIRegistersForOperation();

    mapFile::mapElem registerInformation;
    DEFINE_ADDRESS_RANGE( spiWriteAddressRange, SPI_CONTROL_WRITE_ADDRESS_STRING );
    _spiWriteAddress = registerInformation.reg_address;
    _spiWriteBar = registerInformation.reg_bar;

    _registerMapping->getRegisterInfo( SPI_CONTROL_READBACK_ADDRESS_STRING, registerInformation );
    _spiReadbackAddress = registerInformation.reg_address;
    _spiReadbackBar = registerInformation.reg_bar;
    
    setWriteCallbackFunction( spiWriteAddressRange, boost::bind( &DFMC_MD22Dummy::handleSPIWrite, this ) );

    for (unsigned int i = 0; i < N_MOTORS_MAX ; ++i){
      DEFINE_ADDRESS_RANGE( actualPositionAddressRange,
			    createMotorRegisterName( i, "ACTUAL_POS" ) );
      setWriteCallbackFunction( actualPositionAddressRange, boost::bind( &DFMC_MD22Dummy::writeActualPositionToSpiRegister, this, i ) );
      DEFINE_ADDRESS_RANGE( actualVelocityAddressRange,
			    createMotorRegisterName( i, "V_ACTUAL" ) );
      setWriteCallbackFunction( actualVelocityAddressRange, boost::bind( &DFMC_MD22Dummy::writeActualVelocityToSpiRegister, this, i ) );
    }

    //    setWriteCallbackFunction( 

  }

  void DFMC_MD22Dummy::setSPIRegistersForOperation(){
    for_each( _spiAddressSpace.begin(), _spiAddressSpace.end(), boost::lambda::_1=0 );

    // Information from the data sheet:
    size_t spiAddress =  SPI_ADDRESS_FROM_SMDA_IDXJDX( SMDA_COMMON, JDX_CHIP_VERSION );
    _spiAddressSpace[spiAddress] = CONTROLER_CHIP_VERSION;

    // the CLK2_DIV subword is 15, all other bits stay 0
    spiAddress = SPI_ADDRESS_FROM_SMDA_IDXJDX( SMDA_COMMON, JDX_STEPPER_MOTOR_GLOBAL_PARAMETERS );
    StepperMotorGlobalParameters globalParameters;
    globalParameters.setClk2_div(15);
    _spiAddressSpace.at(spiAddress) = globalParameters.getDATA();

    synchroniseFpgaWithSpiRegisters();
  }

  void DFMC_MD22Dummy::setSPIRegistersForTesting(){
    for( size_t i=0; i< _spiAddressSpace.size(); ++i ){
      _spiAddressSpace[i]=i*i+13;
    }

    synchroniseFpgaWithSpiRegisters();
  }

  void DFMC_MD22Dummy::handleSPIWrite(){
    int32_t spiWriteWord;
    readReg( _spiWriteAddress, &spiWriteWord, _spiWriteBar );

    TMC429InputWord inputWord(spiWriteWord);
    uint32_t spiAddress = inputWord.getADDRESS();

    if (inputWord.getRW() == RW_READ){
      writeSpiContentToReadbackRegister(spiAddress);
    }
    else{
      writeContentToSpiRegister(inputWord.getDATA(), spiAddress);
    }

    triggerActionsOnSpiWrite(inputWord);
  }

  void DFMC_MD22Dummy::writeContentToSpiRegister(unsigned int content,
						 unsigned int spiAddress){
    if (_powerIsUp){
      _spiAddressSpace.at(spiAddress)=content;
    }
  }

  void DFMC_MD22Dummy::writeSpiContentToReadbackRegister(unsigned int spiAddress){
    TMC429OutputWord outputWord;
    // FIXME: set the status bits correctly. We currently leave them at 0;
    outputWord.setDATA( _spiAddressSpace.at(spiAddress) );
     writeRegisterWithoutCallback( _spiReadbackAddress,
				   outputWord.getDataWord(),
				   _spiReadbackBar );
  }

  void DFMC_MD22Dummy::triggerActionsOnSpiWrite(TMC429InputWord const & inputWord){
    switch(inputWord.getSMDA()){
      case SMDA_COMMON: // common registers are addressed by jdx
	performJdxActions( inputWord.getIDX_JDX() );
	break;
      default: // motor specifix registers are addressed by idx
	performIdxActions( inputWord.getIDX_JDX() );
    };

    synchroniseFpgaWithSpiRegisters();
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
    unsigned int powerDownSpiAddress = SPI_ADDRESS_FROM_SMDA_IDXJDX( SMDA_COMMON,
								     JDX_POWER_DOWN );
    writeContentToSpiRegister( 0, powerDownSpiAddress );
    _powerIsUp = true;
  }

  void DFMC_MD22Dummy::synchroniseFpgaWithSpiRegisters(){
    for (unsigned int i = 0; i < N_MOTORS_MAX ; ++i){
      writeSpiRegisterToFpga( i, IDX_ACTUAL_POSITION , "ACTUAL_POS" );
      writeSpiRegisterToFpga( i, IDX_ACTUAL_VELOCITY, "V_ACTUAL");
      writeSpiRegisterToFpga( i, IDX_ACTUAL_ACCELERATION, "ACT_ACCEL" );
      //writeAccelerationThresholdToFpgs( i );
      writeSpiRegisterToFpga( i, IDX_MICRO_STEP_COUNT, "MSTEP_VAL");
    }
  }

  void DFMC_MD22Dummy::writeSpiRegisterToFpga( unsigned int ID, unsigned int IDX, std::string suffix ){
    unsigned int spiAddress = SPI_ADDRESS_FROM_SMDA_IDXJDX( ID,
							    IDX );
    std::string registerName = createMotorRegisterName( ID, suffix );
    mapFile::mapElem registerInformation;
    _registerMapping->getRegisterInfo (registerName, registerInformation);
     writeRegisterWithoutCallback( registerInformation.reg_address,
				   _spiAddressSpace[spiAddress], 
				   registerInformation.reg_bar );
  }

  void DFMC_MD22Dummy::writeFpgaRegisterToSpi( unsigned int ID, unsigned int IDX, std::string suffix ){
    unsigned int spiAddress = SPI_ADDRESS_FROM_SMDA_IDXJDX( ID,
							    IDX );
    std::string registerName = createMotorRegisterName( ID, suffix );
    mapFile::mapElem registerInformation;
    _registerMapping->getRegisterInfo (registerName, registerInformation);
    int32_t fpgaValue;
    readReg( registerInformation.reg_address, &fpgaValue, 
	     registerInformation.reg_bar );
    _spiAddressSpace[spiAddress] = fpgaValue & SPI_DATA_MASK;
    
    // resync the masked value with the FPGA
    writeRegisterWithoutCallback( registerInformation.reg_address,
				  _spiAddressSpace[spiAddress], 
				  registerInformation.reg_bar );    
  }
  
  std::string DFMC_MD22Dummy::createMotorRegisterName( unsigned int ID, std::string suffix ){
    return std::string("WORD_M")
      + boost::lexical_cast<std::string>(ID+1) + "_"
      + suffix;
  }

  void DFMC_MD22Dummy::writeActualPositionToSpiRegister( unsigned int ID ){
    writeFpgaRegisterToSpi( ID, IDX_ACTUAL_POSITION , "ACTUAL_POS" );
  }

  void DFMC_MD22Dummy::writeActualVelocityToSpiRegister( unsigned int ID ){
    writeFpgaRegisterToSpi( ID, IDX_ACTUAL_VELOCITY , "V_ACTUAL" );
  }

  //_WORD_M1_THRESHOLD_ACCEL

}// namespace mtca4u

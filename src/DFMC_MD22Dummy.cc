#include "DFMC_MD22Dummy.h"
#include "DFMC_MD22Constants.h"
#include "NotImplementedException.h"

#include <boost/bind.hpp>

#include "TMC429Words.h"

// See data sheet: the spi address has 7 bits, the lower 4 are IDX or JDX, the higher 3 are SMDA
#define SPI_ADDRESS_FROM_SMDA_IDXJDX( SMDA, IDXJDX ) ((SMDA << 4) + IDXJDX)

namespace mtca4u{
  void DFMC_MD22Dummy::openDev(const std::string &mappingFileName, int perm,
			       devConfigBase* pConfig){
    DummyDevice::openDev(mappingFileName, perm, pConfig);

    prepareSPIRegisters();

    mapFile::mapElem registerInformation;
    _registerMapping->getRegisterInfo( SPI_CONTROL_WRITE_ADDRESS_STRING, registerInformation );
    AddressRange spiWriteAddressRange( registerInformation.reg_address,
				       registerInformation.reg_size,
				       registerInformation.reg_bar);
    _spiWriteAddress = registerInformation.reg_address;
    _spiWriteBar = registerInformation.reg_bar;

    _registerMapping->getRegisterInfo( SPI_CONTROL_READBACK_ADDRESS_STRING, registerInformation );
    _spiReadbackAddress = registerInformation.reg_address;
    _spiReadbackBar = registerInformation.reg_bar;
    
    setWriteCallbackFunction( spiWriteAddressRange, boost::bind( &DFMC_MD22Dummy::handleSPIWrite, this ) );

  }

  void DFMC_MD22Dummy::prepareSPIRegisters(){
    _spiAddressSpace.resize( SIZE_OF_SPI_ADDRESS_SPACE , 0 );

    // Information from the data sheet:
    size_t spiAddress =  SPI_ADDRESS_FROM_SMDA_IDXJDX( SMDA_COMMON, JDX_CHIP_VERSION );
    _spiAddressSpace[spiAddress] = CONTROLER_CHIP_VERSION;

    // the CLK2_DIV subword is 15, all other bits stay 0
    spiAddress = SPI_ADDRESS_FROM_SMDA_IDXJDX( SMDA_COMMON, JDX_STEPPER_MOTOR_GLOBAL_PARAMETERS );
    StepperMotorGlobalParameters globalParameters;
    globalParameters.setClk2_div(15);
    _spiAddressSpace[spiAddress] = globalParameters.getDATA();
  }

  void DFMC_MD22Dummy::handleSPIWrite(){
    int32_t spiWriteWord;
    readReg( _spiWriteAddress, &spiWriteWord, _spiWriteBar );

    TMC429InputWord inputWord(spiWriteWord);
    uint32_t spiAddress = inputWord.getADDRESS();

    if (inputWord.getRW() == RW_READ){
      TMC429OutputWord outputWord;
      // FIXME: set the status bits correctly. We currently leave them at 0;
      outputWord.setDATA( _spiAddressSpace[spiAddress] );
      writeReg( _spiReadbackAddress, outputWord.getDataWord(), _spiReadbackBar );
    }
    else{
      _spiAddressSpace[spiAddress]=inputWord.getDATA();
    }

  }

}// namespace mtca4u

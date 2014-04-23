#include "SPIviaPCIe.h"
#include "DFMC_MD22Constants.h"
#include "MotorDriverException.h"

using namespace mtca4u::dfmc_md22;

namespace mtca4u{

  SPIviaPCIe::SPIviaPCIe( boost::shared_ptr< devMap<devBase> > & mappedDevice,
			  std::string const & writeRegisterName, std::string const & syncRegisterName )
    : _writeRegister( mappedDevice->getRegObject( writeRegisterName ) ),
      _synchronisationRegister(  mappedDevice->getRegObject( syncRegisterName ) ),
      // will always return the last written word
      _readbackRegister(  mappedDevice->getRegObject( writeRegisterName ) )
  {}

  SPIviaPCIe::SPIviaPCIe( boost::shared_ptr< devMap<devBase> > & mappedDevice,
			  std::string const & writeRegisterName, std::string const & syncRegisterName,
			  std::string const & readbackRegisterName )
    : _writeRegister( mappedDevice->getRegObject( writeRegisterName ) ),
      _synchronisationRegister(  mappedDevice->getRegObject( syncRegisterName ) ),
      _readbackRegister(  mappedDevice->getRegObject(readbackRegisterName ) )
  {}

  void SPIviaPCIe::write( int32_t spiCommand ){
    // @fixme Protect this function with a (reentrant?) mutex

    // Implement the write handshake
    // 1. write 0xff to the synch register
    _synchronisationRegister.writeReg( &SPI_SYNC_REQUESTED);

    // 2. write the spi command
    _writeRegister.writeReg( &spiCommand );

    // 3. wait for the handshake
    int32_t syncValue;
    _synchronisationRegister.readReg( &syncValue );
    
    for (size_t syncCounter=0; 
	 (static_cast<unsigned int>(syncValue)==SPI_SYNC_REQUESTED) && (syncCounter < 10);
	 ++syncCounter){
      //sleepMicroSeconds(_spiWaitingTime);
      _synchronisationRegister.readReg( & syncValue, sizeof(int));
    }

    switch( syncValue ){
    case SPI_SYNC_OK:
      break;
    case SPI_SYNC_REQUESTED:
      throw MotorDriverException( "Timeout writing via SPI ", MotorDriverException::SPI_TIMEOUT );
    case SPI_SYNC_ERROR:
    default:
      throw MotorDriverException( "Error writing via SPI ", MotorDriverException::SPI_ERROR );
    }
  }

  uint32_t SPIviaPCIe::read( int32_t spiCommand ){
    // @fixme this has to be locked by a RECURSIVE mutex

    // this is easy: the write does all the sync. And after a successful sync we know that the 
    // readback word is valid.
    write( spiCommand );

    int32_t readbackValue;
    _readbackRegister.readReg( &readbackValue );

    return readbackValue;
  }

}// namespace mtca4u

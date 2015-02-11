#include "SPIviaPCIe.h"
#include "DFMC_MD22Constants.h"
#include "MotorDriverException.h"
#include <ctime>
#include <cerrno>

#include <boost/thread/locks.hpp>

using namespace mtca4u::dfmc_md22;

namespace mtca4u{

  SPIviaPCIe::SPIviaPCIe( boost::shared_ptr< devMap<devBase> > const & mappedDevice,
			  std::string const & writeRegisterName, std::string const & syncRegisterName,
			  unsigned int spiWaitingTime)
    : _writeRegister( mappedDevice->getRegObject( writeRegisterName ) ),
      _synchronisationRegister(  mappedDevice->getRegObject( syncRegisterName ) ),
      // will always return the last written word
      _readbackRegister(  mappedDevice->getRegObject( writeRegisterName ) ),
      _spiWaitingTime(spiWaitingTime),
      _writeRegisterName( writeRegisterName ),
      _syncRegisterName( syncRegisterName )
  {}

  SPIviaPCIe::SPIviaPCIe( boost::shared_ptr< devMap<devBase> > const & mappedDevice,
			  std::string const & writeRegisterName, std::string const & syncRegisterName,
			  std::string const & readbackRegisterName,
			  unsigned int spiWaitingTime )
    : _writeRegister( mappedDevice->getRegObject( writeRegisterName ) ),
      _synchronisationRegister(  mappedDevice->getRegObject( syncRegisterName ) ),
      _readbackRegister(  mappedDevice->getRegObject(readbackRegisterName ) ),
      _spiWaitingTime(spiWaitingTime),
      _writeRegisterName( writeRegisterName ),
      _syncRegisterName( syncRegisterName )
  {}

  void SPIviaPCIe::write( int32_t spiCommand ){
    boost::lock_guard<boost::recursive_mutex> guard(_spiMutex);

    // Implement the write handshake
    // 1. write 0xff to the synch register
    _synchronisationRegister.writeReg( &SPI_SYNC_REQUESTED);

    // 2. write the spi command
    _writeRegister.writeReg( &spiCommand );

    // 3. wait for the handshake
    int32_t syncValue;
    _synchronisationRegister.readReg( &syncValue );
    
    for (size_t syncCounter=0; 
	 (syncValue==SPI_SYNC_REQUESTED) && (syncCounter < 10);
	 ++syncCounter){
      sleepMicroSeconds(_spiWaitingTime);
      _synchronisationRegister.readReg( & syncValue, sizeof(int));
    }

    //It might be inefficient to always create the error message, even if not needed, but
    //it minimises code duplication.
    std::stringstream errorDetails;
    errorDetails << "PCIe register " << _writeRegisterName
		 << ", sync register " << _syncRegisterName  << "= 0x"<< std::hex << syncValue
		 << ", spi command 0x" << std::hex << spiCommand << std::dec;
    switch( syncValue ){
    case SPI_SYNC_OK:
      break;
    case SPI_SYNC_REQUESTED:
      throw MotorDriverException( std::string("Timeout writing via SPI, ") +errorDetails.str(),
				  MotorDriverException::SPI_TIMEOUT );
    case SPI_SYNC_ERROR:
    default:
      throw MotorDriverException( std::string("Error writing via SPI, ") +errorDetails.str(),
				  MotorDriverException::SPI_ERROR );
    }
  }

  uint32_t SPIviaPCIe::read( int32_t spiCommand ){
    boost::lock_guard<boost::recursive_mutex> guard(_spiMutex);

    // this is easy: the write does all the sync. And after a successful sync we know that the 
    // readback word is valid.
    write( spiCommand );

    int32_t readbackValue;
    _readbackRegister.readReg( &readbackValue );

    return readbackValue;
  }

  void SPIviaPCIe::sleepMicroSeconds(unsigned int microSeconds){
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
				    MotorDriverException::SPI_TIMEOUT));
      }
    }
  }

  void SPIviaPCIe::setSpiWaitingTime(unsigned int microSeconds){
    boost::lock_guard<boost::recursive_mutex> guard(_spiMutex);
    _spiWaitingTime = microSeconds;
  }

  unsigned int SPIviaPCIe::getSpiWaitingTime() const {
    boost::lock_guard<boost::recursive_mutex> guard(_spiMutex);
    return _spiWaitingTime;
  }

}// namespace mtca4u

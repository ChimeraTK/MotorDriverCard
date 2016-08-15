#include "SPIviaPCIe.h"
#include "DFMC_MD22Constants.h"
#include "MotorDriverException.h"
#include <ctime>
#include <cerrno>

#include <boost/thread/locks.hpp>
#include <mtca4u/Device.h>

using namespace mtca4u::dfmc_md22;

namespace mtca4u{

  SPIviaPCIe::SPIviaPCIe( boost::shared_ptr< Device > const & device,
			  std::string const & moduleName, 
                          std::string const & writeRegisterName, 
                          std::string const & syncRegisterName,
			  unsigned int spiWaitingTime)
    : _writeRegister( device->getRegisterAccessor(writeRegisterName, moduleName) ),
      _synchronisationRegister(  device->getRegisterAccessor(syncRegisterName, moduleName) ),
      // will always return the last written word
      _readbackRegister(  device->getRegisterAccessor(writeRegisterName, moduleName) ),
      _spiWaitingTime(spiWaitingTime),
      _moduleName(moduleName),
      _writeRegisterName(writeRegisterName),
      _syncRegisterName(syncRegisterName),
      _spiMutex()
  {}

  SPIviaPCIe::SPIviaPCIe( boost::shared_ptr< Device > const & device,
			  std::string const & moduleName, 
                          std::string const & writeRegisterName, 
                          std::string const & syncRegisterName,
			  std::string const & readbackRegisterName,
			  unsigned int spiWaitingTime )
    : _writeRegister( device->getRegisterAccessor(writeRegisterName, moduleName) ),
      _synchronisationRegister(  device->getRegisterAccessor(syncRegisterName, moduleName) ),
      _readbackRegister(  device->getRegisterAccessor(readbackRegisterName, moduleName) ),
      _spiWaitingTime(spiWaitingTime),
      _moduleName(moduleName),
      _writeRegisterName(writeRegisterName),
      _syncRegisterName(syncRegisterName)
      
  {}

  void SPIviaPCIe::write( int32_t spiCommand ){
    boost::lock_guard<boost::recursive_mutex> guard(_spiMutex);

    // Implement the write handshake
    // 1. write 0xff to the synch register
    _synchronisationRegister->writeRaw( &SPI_SYNC_REQUESTED);

    // 2. write the spi command
    _writeRegister->writeRaw( &spiCommand );

    // 3. wait for the handshake
    int32_t syncValue;
    _synchronisationRegister->readRaw( &syncValue );
    
    for (size_t syncCounter=0; 
	 (syncValue==SPI_SYNC_REQUESTED) && (syncCounter < 10);
	 ++syncCounter){
      sleepMicroSeconds(_spiWaitingTime);
      _synchronisationRegister->readRaw( & syncValue, sizeof(int));
    }

    //It might be inefficient to always create the error message, even if not needed, but
    //it minimises code duplication.
    std::stringstream errorDetails;
    errorDetails << "PCIe register " << _moduleName << "." << _writeRegisterName
		 << ", sync register " << _moduleName << "." << _syncRegisterName  << "= 0x"<< std::hex << syncValue
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
    _readbackRegister->readRaw( &readbackValue );

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

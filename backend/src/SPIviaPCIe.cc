#include "impl/SPIviaPCIe.h"
#include "DFMC_MD22Constants.h"
#include "MotorDriverException.h"
#include <cerrno>
#include <ctime>

#include <ChimeraTK/Device.h>
#include <boost/thread/locks.hpp>

using namespace mtca4u::dfmc_md22;

namespace mtca4u {

  SPIviaPCIe::SPIviaPCIe(boost::shared_ptr<ChimeraTK::Device> const& device,
      std::string const& moduleName,
      std::string const& writeRegisterName,
      std::string const& syncRegisterName,
      unsigned int spiWaitingTime)
  : _writeRegister(device->getScalarRegisterAccessor<int32_t>(
        moduleName + "/" + writeRegisterName, 0, {ChimeraTK::AccessMode::raw})),
    _synchronisationRegister(device->getScalarRegisterAccessor<int32_t>(
        moduleName + "/" + syncRegisterName, 0, {ChimeraTK::AccessMode::raw})),
    // will always return the last written word
    _readbackRegister(device->getScalarRegisterAccessor<int32_t>(
        moduleName + "/" + writeRegisterName, 0, {ChimeraTK::AccessMode::raw})),
    _spiWaitingTime(spiWaitingTime), _moduleName(moduleName), _writeRegisterName(writeRegisterName),
    _syncRegisterName(syncRegisterName), _spiMutex() {}

  SPIviaPCIe::SPIviaPCIe(boost::shared_ptr<ChimeraTK::Device> const& device,
      std::string const& moduleName,
      std::string const& writeRegisterName,
      std::string const& syncRegisterName,
      std::string const& readbackRegisterName,
      unsigned int spiWaitingTime)
  : _writeRegister(device->getScalarRegisterAccessor<int32_t>(
        moduleName + "/" + writeRegisterName, 0, {ChimeraTK::AccessMode::raw})),
    _synchronisationRegister(device->getScalarRegisterAccessor<int32_t>(
        moduleName + "/" + syncRegisterName, 0, {ChimeraTK::AccessMode::raw})),
    _readbackRegister(device->getScalarRegisterAccessor<int32_t>(
        moduleName + "/" + readbackRegisterName, 0, {ChimeraTK::AccessMode::raw})),
    _spiWaitingTime(spiWaitingTime), _moduleName(moduleName), _writeRegisterName(writeRegisterName),
    _syncRegisterName(syncRegisterName), _spiMutex() {}

  void SPIviaPCIe::write(int32_t spiCommand) {
    boost::lock_guard<boost::recursive_mutex> guard(_spiMutex);

    // try three times to mittigate effects of a firmware bug
    for(int i = 0; i < 3; ++i) {
      if(i > 0) {
        std::cerr << "Warning, SPI handshake timed out. Retrying..." << std::endl;
      }

      // Implement the write handshake
      // 1. write 0xff to the synch register
      _synchronisationRegister = SPI_SYNC_REQUESTED;
      _synchronisationRegister.write();

      // 2. write the spi command
      _writeRegister = spiCommand;
      _writeRegister.write();

      // 3. wait for the handshake
      _synchronisationRegister.read();

      for(size_t syncCounter = 0; (_synchronisationRegister == SPI_SYNC_REQUESTED) && (syncCounter < 10);
          ++syncCounter) {
        sleepMicroSeconds(_spiWaitingTime);

        _synchronisationRegister.read();
      }

      if(_synchronisationRegister != SPI_SYNC_REQUESTED) {
        // break on either error or ok. If still in SPI_SYNC_REQUESTED repeat (up
        // to 3 times)
        break;
      }
    }

    // It might be inefficient to always create the error message, even if not
    // needed, but it minimises code duplication.
    std::stringstream errorDetails;
    errorDetails << "PCIe register " << _moduleName << "." << _writeRegisterName << ", sync register " << _moduleName
                 << "." << _syncRegisterName << "= 0x" << std::hex << _synchronisationRegister << ", spi command 0x"
                 << std::hex << spiCommand << std::dec;
    switch(_synchronisationRegister) {
      case SPI_SYNC_OK:
        break;
      case SPI_SYNC_REQUESTED:
        throw MotorDriverException(
            std::string("Timeout writing via SPI, ") + errorDetails.str(), MotorDriverException::SPI_TIMEOUT);
      case SPI_SYNC_ERROR:
      default:
        throw MotorDriverException(
            std::string("Error writing via SPI, ") + errorDetails.str(), MotorDriverException::SPI_ERROR);
    }
  }

  uint32_t SPIviaPCIe::read(int32_t spiCommand) {
    boost::lock_guard<boost::recursive_mutex> guard(_spiMutex);

    // this is easy: the write does all the sync. And after a successful sync we
    // know that the readback word is valid.
    write(spiCommand);

    _readbackRegister.read();
    return static_cast<uint32_t>(_readbackRegister);
  }

  void SPIviaPCIe::sleepMicroSeconds(unsigned int microSeconds) {
    timespec sleepTime;
    sleepTime.tv_sec = microSeconds / 1000000;
    sleepTime.tv_nsec = (microSeconds % 1000000) * 1000;

    timespec remainingTime;
    remainingTime.tv_sec = 0;
    remainingTime.tv_nsec = 0;
    if(nanosleep(&sleepTime, &remainingTime)) {
      if(errno == EFAULT) {
        std::stringstream errorMessage;
        errorMessage << "Error sleeping " << microSeconds << " micro seconds!";
        throw(MotorDriverException(errorMessage.str(), MotorDriverException::SPI_TIMEOUT));
      }
    }
  }

  void SPIviaPCIe::setSpiWaitingTime(unsigned int microSeconds) {
    boost::lock_guard<boost::recursive_mutex> guard(_spiMutex);
    _spiWaitingTime = microSeconds;
  }

  unsigned int SPIviaPCIe::getSpiWaitingTime() const {
    boost::lock_guard<boost::recursive_mutex> guard(_spiMutex);
    return _spiWaitingTime;
  }

} // namespace mtca4u

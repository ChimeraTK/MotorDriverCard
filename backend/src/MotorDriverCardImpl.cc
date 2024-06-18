#include "impl/MotorDriverCardImpl.h"

#include "impl/MotorControlerImpl.h"

#include <boost/shared_ptr.hpp>

#include <sstream>
#include <stdexcept>
using namespace mtca4u::tmc429;

#include "DFMC_MD22Constants.h"

#include <ChimeraTK/Device.h>
using namespace mtca4u::dfmc_md22;

namespace mtca4u {
  MotorDriverCardImpl::MotorDriverCardImpl(boost::shared_ptr<ChimeraTK::Device> const& device,
      std::string const& moduleName,
      MotorDriverCardConfig const& cardConfiguration)
  : _motorControlers(),               // done later in the constructor body
    _device(device), _powerMonitor(), // done later in the constructor body
    _controlerSPI(),                  // done later in the constructor body
    _controlerStatusRegister(device->getScalarRegisterAccessor<int32_t>(
        moduleName + "/" + CONTROLER_STATUS_BITS_ADDRESS_STRING, 0, {ChimeraTK::AccessMode::raw})),
    _moduleName(moduleName) {
    checkFirmwareVersion();

    // the shared pointers can only be initialised inside the constructor because
    // execution order of the new operations is not defined in the contructor. As
    // any new can throw, it might be that the other object is allocated, but the
    // corresponding smart pointer is not initialised yet, which would lead to a
    // memory leak.
    _powerMonitor.reset(new PowerMonitor);
    _controlerSPI.reset(
        new TMC429SPI(device, moduleName, CONTROLER_SPI_WRITE_ADDRESS_STRING, CONTROLER_SPI_SYNC_ADDRESS_STRING,
            CONTROLER_SPI_READBACK_ADDRESS_STRING, cardConfiguration.controlerSpiWaitingTime));

    // initialise common registers
    setCoverDatagram(cardConfiguration.coverDatagram);
    setCoverPositionAndLength(cardConfiguration.coverPositionAndLength);
    setDatagramHighWord(cardConfiguration.datagramHighWord);
    setDatagramLowWord(cardConfiguration.datagramLowWord);
    setInterfaceConfiguration(cardConfiguration.interfaceConfiguration);
    setPositionCompareInterruptData(cardConfiguration.positionCompareInterruptData);
    setPositionCompareWord(cardConfiguration.positionCompareWord);
    setStepperMotorGlobalParameters(cardConfiguration.stepperMotorGlobalParameters);

    // initialise motors
    _motorControlers.resize(N_MOTORS_MAX);
    for(unsigned int i = 0; i < _motorControlers.size(); ++i) {
      _motorControlers[i].reset(new MotorControlerImpl(
          i, device, moduleName, _controlerSPI, cardConfiguration.motorControlerConfigurations[i]));
    }
  }

  boost::shared_ptr<MotorControler> MotorDriverCardImpl::getMotorControler(unsigned int motorControlerID) {
    try {
      return _motorControlers.at(motorControlerID);
    }
    catch(std::out_of_range& e) {
      std::stringstream errorMessage;
      errorMessage << "motorControlerID " << motorControlerID << " is too large. " << e.what();
      throw ChimeraTK::logic_error(errorMessage.str());
    }
  }

  PowerMonitor& MotorDriverCardImpl::getPowerMonitor() {
    return *_powerMonitor;
  }

  unsigned int MotorDriverCardImpl::getControlerChipVersion() {
    return _controlerSPI->read(SMDA_COMMON, JDX_CHIP_VERSION).getDATA();
  }

  void MotorDriverCardImpl::setDatagramLowWord(unsigned int datagramLowWord) {
    _controlerSPI->write(SMDA_COMMON, JDX_DATAGRAM_LOW_WORD, datagramLowWord);
  }

  unsigned int MotorDriverCardImpl::getDatagramLowWord() {
    return _controlerSPI->read(SMDA_COMMON, JDX_DATAGRAM_LOW_WORD).getDATA();
  }

  void MotorDriverCardImpl::setDatagramHighWord(unsigned int datagramHighWord) {
    _controlerSPI->write(SMDA_COMMON, JDX_DATAGRAM_HIGH_WORD, datagramHighWord);
  }

  unsigned int MotorDriverCardImpl::getDatagramHighWord() {
    return _controlerSPI->read(SMDA_COMMON, JDX_DATAGRAM_HIGH_WORD).getDATA();
  }

  void MotorDriverCardImpl::setCoverPositionAndLength(CoverPositionAndLength const& coverPositionAndLength) {
    _controlerSPI->write(coverPositionAndLength);
  }

  CoverPositionAndLength MotorDriverCardImpl::getCoverPositionAndLength() {
    return CoverPositionAndLength(_controlerSPI->read(SMDA_COMMON, JDX_COVER_POSITION_AND_LENGTH).getDATA());
  }

  void MotorDriverCardImpl::setCoverDatagram(unsigned int coverDatagram) {
    _controlerSPI->write(SMDA_COMMON, JDX_COVER_DATAGRAM, coverDatagram);
  }

  unsigned int MotorDriverCardImpl::getCoverDatagram() {
    return _controlerSPI->read(SMDA_COMMON, JDX_COVER_DATAGRAM).getDATA();
  }

  void MotorDriverCardImpl::setStepperMotorGlobalParameters(
      StepperMotorGlobalParameters const& stepperMotorGlobalParameters) {
    _controlerSPI->write(stepperMotorGlobalParameters);
  }

  StepperMotorGlobalParameters MotorDriverCardImpl::getStepperMotorGlobalParameters() {
    return StepperMotorGlobalParameters(
        _controlerSPI->read(SMDA_COMMON, JDX_STEPPER_MOTOR_GLOBAL_PARAMETERS).getDATA());
  }

  void MotorDriverCardImpl::setInterfaceConfiguration(InterfaceConfiguration const& interfaceConfiguration) {
    _controlerSPI->write(interfaceConfiguration);
  }

  InterfaceConfiguration MotorDriverCardImpl::getInterfaceConfiguration() {
    return InterfaceConfiguration(_controlerSPI->read(SMDA_COMMON, JDX_INTERFACE_CONFIGURATION).getDATA());
  }

  void MotorDriverCardImpl::setPositionCompareWord(unsigned int positionCompareWord) {
    _controlerSPI->write(SMDA_COMMON, JDX_POSITION_COMPARE, positionCompareWord);
  }

  unsigned int MotorDriverCardImpl::getPositionCompareWord() {
    return _controlerSPI->read(SMDA_COMMON, JDX_POSITION_COMPARE).getDATA();
  }

  void MotorDriverCardImpl::setPositionCompareInterruptData(
      PositionCompareInterruptData const& positionCompareInterruptData) {
    return _controlerSPI->write(positionCompareInterruptData);
  }

  PositionCompareInterruptData MotorDriverCardImpl::getPositionCompareInterruptData() {
    return PositionCompareInterruptData(_controlerSPI->read(SMDA_COMMON, JDX_POSITION_COMPARE_INTERRUPT).getDATA());
  }

  void MotorDriverCardImpl::powerDown() {
    _controlerSPI->write(SMDA_COMMON, JDX_POWER_DOWN, 1);
  }

  ReferenceSwitchData MotorDriverCardImpl::getReferenceSwitchData() {
    return ReferenceSwitchData(_controlerSPI->read(SMDA_COMMON, JDX_REFERENCE_SWITCH).getDATA());
  }

  TMC429StatusWord MotorDriverCardImpl::getStatusWord() {
    _controlerStatusRegister.read();
    return TMC429StatusWord(_controlerStatusRegister);
  }

  void MotorDriverCardImpl::checkFirmwareVersion() {
    ChimeraTK::ScalarRegisterAccessor<int32_t> firmwareVersion =
        _device->getScalarRegisterAccessor<int32_t>(_moduleName + "/" + PROJECT_VERSION_ADDRESS_STRING);
    firmwareVersion.read();

    // only allow firmware versions with the same major number
    uint32_t maxFirmwareVersion = (MINIMAL_FIRMWARE_VERSION & 0xFF000000) | 0x00FFFFFF;

    // check the version
    if((static_cast<uint32_t>(firmwareVersion) < static_cast<uint32_t>(MINIMAL_FIRMWARE_VERSION)) ||
        (static_cast<uint32_t>(firmwareVersion) > maxFirmwareVersion)) {
      std::stringstream errorMessage;
      errorMessage << "MotorDriverCardImpl detected wrong firmware version. "
                   << "Current firmware is 0x" << std::hex << firmwareVersion << ", minumum required version is 0x"
                   << MINIMAL_FIRMWARE_VERSION << ", maximum allowed version is 0x" << maxFirmwareVersion << std::dec
                   << ".";
      throw ChimeraTK::runtime_error(errorMessage.str());
    }
  }

} // namespace mtca4u

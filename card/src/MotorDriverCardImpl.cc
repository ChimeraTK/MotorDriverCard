// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "impl/MotorDriverCardImpl.h"

#include "impl/MotorControllerImpl.h"

#include <sstream>
#include <stdexcept>
using namespace ChimeraTK::MotorDriver::tmc429;

#include "DFMC_MD22Constants.h"

#include <ChimeraTK/Device.h>
using namespace ChimeraTK::MotorDriver::dfmc_md22;

namespace ChimeraTK::MotorDriver {
  MotorDriverCardImpl::MotorDriverCardImpl(std::shared_ptr<ChimeraTK::Device> const& device,
      std::string const& moduleName,
      MotorDriverCardConfig const& cardConfiguration)
  :                  // done later in the constructor body
    _device(device), // done later in the constructor body
    _controllerStatusRegister(device->getScalarRegisterAccessor<int32_t>(
        moduleName + "/" + CONTROLLER_STATUS_BITS_ADDRESS_STRING, 0, {ChimeraTK::AccessMode::raw})),
    _moduleName(moduleName) {
    checkFirmwareVersion();

    // the shared pointers can only be initialised inside the constructor because
    // execution order of the new operations is not defined in the contructor. As
    // any new can throw, it might be that the other object is allocated, but the
    // corresponding smart pointer is not initialised yet, which would lead to a
    // memory leak.
    _powerMonitor = std::make_shared<PowerMonitor>();
    _controllerSPI = std::make_shared<TMC429SPI>(device, moduleName, CONTROLLER_SPI_WRITE_ADDRESS_STRING,
        CONTROLLER_SPI_SYNC_ADDRESS_STRING, CONTROLLER_SPI_READBACK_ADDRESS_STRING,
        cardConfiguration.controllerSpiWaitingTime);

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
    _motorControllers.resize(N_MOTORS_MAX);
    for(unsigned int i = 0; i < _motorControllers.size(); ++i) {
      _motorControllers[i] = std::make_shared<MotorControllerImpl>(
          i, device, moduleName, _controllerSPI, cardConfiguration.motorControllerConfigurations[i]);
    }
  }

  std::shared_ptr<MotorController> MotorDriverCardImpl::getMotorController(unsigned int motorControllerID) {
    try {
      return _motorControllers.at(motorControllerID);
    }
    catch(std::out_of_range& e) {
      std::stringstream errorMessage;
      errorMessage << "motorControllerID " << motorControllerID << " is too large. " << e.what();
      throw ChimeraTK::logic_error(errorMessage.str());
    }
  }

  PowerMonitor& MotorDriverCardImpl::getPowerMonitor() {
    return *_powerMonitor;
  }

  unsigned int MotorDriverCardImpl::getControllerChipVersion() {
    return _controllerSPI->read(SMDA_COMMON, JDX_CHIP_VERSION).getDATA();
  }

  void MotorDriverCardImpl::setDatagramLowWord(unsigned int datagramLowWord) {
    _controllerSPI->write(SMDA_COMMON, JDX_DATAGRAM_LOW_WORD, datagramLowWord);
  }

  unsigned int MotorDriverCardImpl::getDatagramLowWord() {
    return _controllerSPI->read(SMDA_COMMON, JDX_DATAGRAM_LOW_WORD).getDATA();
  }

  void MotorDriverCardImpl::setDatagramHighWord(unsigned int datagramHighWord) {
    _controllerSPI->write(SMDA_COMMON, JDX_DATAGRAM_HIGH_WORD, datagramHighWord);
  }

  unsigned int MotorDriverCardImpl::getDatagramHighWord() {
    return _controllerSPI->read(SMDA_COMMON, JDX_DATAGRAM_HIGH_WORD).getDATA();
  }

  void MotorDriverCardImpl::setCoverPositionAndLength(CoverPositionAndLength const& coverPositionAndLength) {
    _controllerSPI->write(coverPositionAndLength);
  }

  CoverPositionAndLength MotorDriverCardImpl::getCoverPositionAndLength() {
    // NOLINTNEXTLINE(modernize-return-braced-init-list) the constructor is explicit
    return CoverPositionAndLength(_controllerSPI->read(SMDA_COMMON, JDX_COVER_POSITION_AND_LENGTH).getDATA());
  }

  void MotorDriverCardImpl::setCoverDatagram(unsigned int coverDatagram) {
    _controllerSPI->write(SMDA_COMMON, JDX_COVER_DATAGRAM, coverDatagram);
  }

  unsigned int MotorDriverCardImpl::getCoverDatagram() {
    return _controllerSPI->read(SMDA_COMMON, JDX_COVER_DATAGRAM).getDATA();
  }

  void MotorDriverCardImpl::setStepperMotorGlobalParameters(
      StepperMotorGlobalParameters const& stepperMotorGlobalParameters) {
    _controllerSPI->write(stepperMotorGlobalParameters);
  }

  StepperMotorGlobalParameters MotorDriverCardImpl::getStepperMotorGlobalParameters() {
    // NOLINTNEXTLINE(modernize-return-braced-init-list) the constructor is explicit
    return StepperMotorGlobalParameters(
        _controllerSPI->read(SMDA_COMMON, JDX_STEPPER_MOTOR_GLOBAL_PARAMETERS).getDATA());
  }

  void MotorDriverCardImpl::setInterfaceConfiguration(InterfaceConfiguration const& interfaceConfiguration) {
    _controllerSPI->write(interfaceConfiguration);
  }

  InterfaceConfiguration MotorDriverCardImpl::getInterfaceConfiguration() {
    // NOLINTNEXTLINE(modernize-return-braced-init-list) the constructor is explicit
    return InterfaceConfiguration(_controllerSPI->read(SMDA_COMMON, JDX_INTERFACE_CONFIGURATION).getDATA());
  }

  void MotorDriverCardImpl::setPositionCompareWord(unsigned int positionCompareWord) {
    _controllerSPI->write(SMDA_COMMON, JDX_POSITION_COMPARE, positionCompareWord);
  }

  unsigned int MotorDriverCardImpl::getPositionCompareWord() {
    return _controllerSPI->read(SMDA_COMMON, JDX_POSITION_COMPARE).getDATA();
  }

  void MotorDriverCardImpl::setPositionCompareInterruptData(
      PositionCompareInterruptData const& positionCompareInterruptData) {
    return _controllerSPI->write(positionCompareInterruptData);
  }

  PositionCompareInterruptData MotorDriverCardImpl::getPositionCompareInterruptData() {
    // NOLINTNEXTLINE(modernize-return-braced-init-list) the constructor is explicit
    return PositionCompareInterruptData(_controllerSPI->read(SMDA_COMMON, JDX_POSITION_COMPARE_INTERRUPT).getDATA());
  }

  void MotorDriverCardImpl::powerDown() {
    _controllerSPI->write(SMDA_COMMON, JDX_POWER_DOWN, 1);
  }

  ReferenceSwitchData MotorDriverCardImpl::getReferenceSwitchData() {
    // NOLINTNEXTLINE(modernize-return-braced-init-list) the constructor is explicit
    return ReferenceSwitchData(_controllerSPI->read(SMDA_COMMON, JDX_REFERENCE_SWITCH).getDATA());
  }

  TMC429StatusWord MotorDriverCardImpl::getStatusWord() {
    _controllerStatusRegister.read();
    // NOLINTNEXTLINE(modernize-return-braced-init-list) braced initialisation would narrow from int to unsigned int
    return TMC429StatusWord(_controllerStatusRegister);
  }

  void MotorDriverCardImpl::checkFirmwareVersion() {
    ChimeraTK::ScalarRegisterAccessor<int32_t> firmwareVersion =
        _device->getScalarRegisterAccessor<int32_t>(_moduleName + "/" + PROJECT_VERSION_ADDRESS_STRING);
    firmwareVersion.read();

    // only allow firmware versions with the same major number
    uint32_t maxFirmwareVersion = (static_cast<uint32_t>(MINIMAL_FIRMWARE_VERSION) & 0xFF000000U) | 0x00FFFFFFU;

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

} // namespace ChimeraTK::MotorDriver

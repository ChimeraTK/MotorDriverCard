// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "impl/MotorControllerImpl.h"

#include "DFMC_MD22Constants.h"
using namespace ChimeraTK::MotorDriver::dfmc_md22;
#include "TMC429Constants.h"
using namespace ChimeraTK::MotorDriver::tmc429;

#include "impl/MotorDriverCardImpl.h"

#include <ChimeraTK/Device.h>

#include <cmath>

// just save some typing...
#define RAW_ACCESSOR_FROM_SUFFIX(MODULE_NAME, SUFFIX)                                                                  \
  device->getScalarRegisterAccessor<int32_t>(                                                                          \
      (MODULE_NAME) + "/" + createMotorRegisterName(_id, SUFFIX), 0, {ChimeraTK::AccessMode::raw})

// Macros which cover more than one line are not good for the code coverage
// test, as only the macro call is checked. In this case it is probably ok
// because we gain a lot of code because we have to define many variables, AND
// there is only one line per function without branching, which is covered by
// the function coverage test (not the line coverage test). Otherwise we would
// need a complete test which checks the macro.
#define DEFINE_GET_SET_VALUE(NAME, IDX)                                                                                \
  unsigned int MotorControllerImpl::get##NAME() {                                                                      \
    lock_guard guard(_mutex);                                                                                          \
    return _controllerSPI->read(_id, IDX).getDATA();                                                                   \
  }                                                                                                                    \
  void MotorControllerImpl::set##NAME(unsigned int value) {                                                            \
    lock_guard guard(_mutex);                                                                                          \
    _controllerSPI->write(_id, IDX, value);                                                                            \
  }

#define DEFINE_SIGNED_GET_SET_VALUE(NAME, IDX, CONVERTER)                                                              \
  int MotorControllerImpl::get##NAME() {                                                                               \
    lock_guard guard(_mutex);                                                                                          \
    int readValue = static_cast<int>(_controllerSPI->read(_id, IDX).getDATA());                                        \
    return (CONVERTER).customToThirtyTwo(readValue);                                                                   \
  }                                                                                                                    \
  void MotorControllerImpl::set##NAME(int value) {                                                                     \
    lock_guard guard(_mutex);                                                                                          \
    unsigned int writeValue = static_cast<unsigned int>((CONVERTER).thirtyTwoToCustom(value));                         \
    _controllerSPI->write(_id, IDX, writeValue);                                                                       \
  }

#define DEFINE_SET_GET_TYPED_CONTROLLER_REGISTER(NAME)                                                                 \
  NAME MotorControllerImpl::get##NAME() {                                                                              \
    lock_guard guard(_mutex);                                                                                          \
    return readTypedRegister<NAME>();                                                                                  \
  }                                                                                                                    \
  void MotorControllerImpl::set##NAME(NAME const& inputWord) {                                                         \
    lock_guard guard(_mutex);                                                                                          \
    writeTypedControllerRegister(inputWord);                                                                           \
  }

#define DEFINE_SET_GET_TYPED_DRIVER_DATA(NAME, LOCAL_DATA_INSTANCE)                                                    \
  NAME const& MotorControllerImpl::get##NAME() const {                                                                 \
    lock_guard guard(_mutex);                                                                                          \
    return LOCAL_DATA_INSTANCE;                                                                                        \
  }                                                                                                                    \
  void MotorControllerImpl::set##NAME(NAME const& driverData) {                                                        \
    lock_guard guard(_mutex);                                                                                          \
    setTypedDriverData<NAME>(driverData, LOCAL_DATA_INSTANCE);                                                         \
  }

using lock_guard = std::lock_guard<std::mutex>;
using unique_lock = std::unique_lock<std::mutex>;

namespace ChimeraTK::MotorDriver {
  MotorControllerImpl::MotorControllerImpl(unsigned int ID, boost::shared_ptr<ChimeraTK::Device> const& device,
      std::string const& moduleName, boost::shared_ptr<TMC429SPI> const& controllerSPI,
      MotorControllerConfig const& motorControllerConfig)
  : _mutex(), _device(device), _id(ID), _controllerConfig(motorControllerConfig),
    _conversionFactor(calculateConversionFactor(motorControllerConfig)),
    _currentVmax(motorControllerConfig.maximumVelocity),
    _usrSetCurrentScale(_controllerConfig.stallGuardControlData.getCurrentScale()), _driverControlData(), // set later
                                                                                                          // in the
                                                                                                          // constructor
                                                                                                          // body
    _chopperControlData(),    // set later in the constructor body
    _coolStepControlData(),   // set later in the constructor body
    _stallGuardControlData(), // set later in the constructor body
    _driverConfigData(),      // set later in the constructor body
    _controllerStatus{device->getScalarRegisterAccessor<int32_t>(
        moduleName + "/" + CONTROLLER_STATUS_BITS_ADDRESS_STRING, 0, {ChimeraTK::AccessMode::raw})},
    _actualPosition{RAW_ACCESSOR_FROM_SUFFIX(moduleName, ACTUAL_POSITION_SUFFIX)},
    _actualVelocity{RAW_ACCESSOR_FROM_SUFFIX(moduleName, ACTUAL_VELOCITY_SUFFIX)},
    _actualAcceleration{RAW_ACCESSOR_FROM_SUFFIX(moduleName, ACTUAL_ACCELERATION_SUFFIX)},
    _microStepCount{RAW_ACCESSOR_FROM_SUFFIX(moduleName, MICRO_STEP_COUNT_SUFFIX)},
    _stallGuardValue{RAW_ACCESSOR_FROM_SUFFIX(moduleName, STALL_GUARD_VALUE_SUFFIX)},
    _coolStepValue{RAW_ACCESSOR_FROM_SUFFIX(moduleName, COOL_STEP_VALUE_SUFFIX)},
    _status{RAW_ACCESSOR_FROM_SUFFIX(moduleName, STATUS_SUFFIX)},
    _motorCurrentEnabled{RAW_ACCESSOR_FROM_SUFFIX(moduleName, MOTOR_CURRENT_ENABLE_SUFFIX)},
    _decoderReadoutMode{RAW_ACCESSOR_FROM_SUFFIX(moduleName, DECODER_READOUT_MODE_SUFFIX)},
    _decoderPosition{RAW_ACCESSOR_FROM_SUFFIX(moduleName, DECODER_POSITION_SUFFIX)},
    _calibrationTime{RAW_ACCESSOR_FROM_SUFFIX(moduleName, CALIBRATION_TIME_SUFFIX)},
    _endSwitchPositive{RAW_ACCESSOR_FROM_SUFFIX(moduleName, END_SWITCH_POSITIVE_SUFFIX)},
    _endSwitchNegative{RAW_ACCESSOR_FROM_SUFFIX(moduleName, END_SWITCH_NEGATIVE_SUFFIX)}, _endSwitchPowerIndicator{},
    _driverSPI(device, moduleName, createMotorRegisterName(ID, SPI_WRITE_SUFFIX),
        createMotorRegisterName(ID, SPI_SYNC_SUFFIX), motorControllerConfig.driverSpiWaitingTime),
    _controllerSPI(controllerSPI), _converter24bits(24), _converter12bits(12), _moveOnlyFullStep(false) {
    setAccelerationThresholdData(motorControllerConfig.accelerationThresholdData);
    // setActualPosition( motorControllerConfig.actualPosition );
    setChopperControlData(motorControllerConfig.chopperControlData);
    setCoolStepControlData(motorControllerConfig.coolStepControlData);
    setDecoderReadoutMode(motorControllerConfig.decoderReadoutMode);
    setDividersAndMicroStepResolutionData(motorControllerConfig.dividersAndMicroStepResolutionData);
    setDriverConfigData(motorControllerConfig.driverConfigData);
    setDriverControlData(motorControllerConfig.driverControlData);
    setInterruptData(motorControllerConfig.interruptData);
    setMaximumAcceleration(motorControllerConfig.maximumAcceleration);
    setMaximumVelocity(motorControllerConfig.maximumVelocity);
    setMicroStepCount(motorControllerConfig.microStepCount);
    setMinimumVelocity(motorControllerConfig.minimumVelocity);
    setPositionTolerance(motorControllerConfig.positionTolerance);
    setProportionalityFactorData(motorControllerConfig.proportionalityFactorData);
    setReferenceConfigAndRampModeData(motorControllerConfig.referenceConfigAndRampModeData);
    setStallGuardControlData(motorControllerConfig.stallGuardControlData);
    setTargetVelocity(motorControllerConfig.targetVelocity);

    _localTargetPosition = retrieveTargetPositonAndConvert();
    _userMicroStepSize = pow(2, motorControllerConfig.driverControlData.getMicroStepResolution());

    // enabling the motor is the last step after setting all registers
    setEnabled(motorControllerConfig.enabled);
    try {
      // this must throw on mapfile not having this register
      _endSwitchPowerIndicator.replace(RAW_ACCESSOR_FROM_SUFFIX(moduleName, ENDSWITCH_ENABLE_SUFFIX));
    }
    catch(std::exception& a) {
      // ignore exception when creating the accessor with the consequence that
      // _endSwitchPowerIndicator stays uninitialized. This happens when the
      // library is used with old firmware that does not have the
      // WORD_M1_VOLTAGE_EN register in the mapfile. Methods using this accessor
      // have to check its isInitialised() method prior to using it.
    }
  }

  unsigned int MotorControllerImpl::getID() {
    lock_guard guard(_mutex);
    return _id;
  }

  int MotorControllerImpl::getActualPosition() {
    lock_guard guard(_mutex);
    return readPositionRegisterAndConvert();
  }

  int MotorControllerImpl::readPositionRegisterAndConvert() {
    _actualPosition.read();
    return _converter24bits.customToThirtyTwo(_actualPosition);
  }

  unsigned int MotorControllerImpl::readRegisterAccessor(ChimeraTK::ScalarRegisterAccessor<int32_t>& readValue) {
    readValue.read();
    return static_cast<unsigned int>(readValue);
  }

  void MotorControllerImpl::setActualPosition(int position) {
    lock_guard guard(_mutex);
    _controllerSPI->write(
        _id, IDX_ACTUAL_POSITION, static_cast<unsigned int>(_converter24bits.thirtyTwoToCustom(position)));
  }

  int MotorControllerImpl::getActualVelocity() {
    lock_guard guard(_mutex);
    _actualVelocity.read();
    return _converter12bits.customToThirtyTwo(_actualVelocity);
  }

  void MotorControllerImpl::setActualVelocity(int velocity) {
    lock_guard guard(_mutex);
    _controllerSPI->write(
        _id, IDX_ACTUAL_VELOCITY, static_cast<unsigned int>(_converter12bits.thirtyTwoToCustom(velocity)));
  }

  unsigned int MotorControllerImpl::getActualAcceleration() {
    lock_guard guard(_mutex);
    return readRegisterAccessor(_actualAcceleration);
  }

  void MotorControllerImpl::setActualAcceleration(unsigned int acceleration) {
    lock_guard guard(_mutex);
    _controllerSPI->write(_id, IDX_ACTUAL_ACCELERATION, acceleration);
  }

  unsigned int MotorControllerImpl::getMicroStepCount() {
    lock_guard guard(_mutex);
    return readRegisterAccessor(_microStepCount);
  }

  void MotorControllerImpl::setMicroStepCount(unsigned int microStepCount) {
    lock_guard guard(_mutex);
    _controllerSPI->write(_id, IDX_MICRO_STEP_COUNT, microStepCount);
  }

  unsigned int MotorControllerImpl::getCoolStepValue() {
    lock_guard guard(_mutex);
    return readRegisterAccessor(_coolStepValue);
  }

  unsigned int MotorControllerImpl::getStallGuardValue() {
    lock_guard guard(_mutex);
    return readRegisterAccessor(_stallGuardValue);
  }

  DriverStatusData MotorControllerImpl::getStatus() {
    lock_guard guard(_mutex);
    return {readRegisterAccessor(_status)};
  }

  void MotorControllerImpl::setDecoderReadoutMode(unsigned int readoutMode) {
    lock_guard guard(_mutex);
    _decoderReadoutMode = static_cast<int32_t>(readoutMode);
    _decoderReadoutMode.write();
  }

  unsigned int MotorControllerImpl::getDecoderReadoutMode() {
    lock_guard guard(_mutex);
    return readRegisterAccessor(_decoderReadoutMode);
  }

  unsigned int MotorControllerImpl::getDecoderPosition() {
    lock_guard guard(_mutex);
    return readRegisterAccessor(_decoderPosition);
  }

  void MotorControllerImpl::setEnabled(bool enable) {
    // The mutex is locked in setMotorCurrentEnabled
    setMotorCurrentEnabled(enable);
  }

  bool MotorControllerImpl::isEnabled() {
    // Mutex is locked in isMotorCurrentEnabled
    return isMotorCurrentEnabled();
  }

  void MotorControllerImpl::enableFullStepping(bool enable) {
    _moveOnlyFullStep = enable;
  }

  bool MotorControllerImpl::isFullStepping() {
    return _moveOnlyFullStep;
  }

  // DEFINE_SIGNED_GET_SET_VALUE( TargetPosition, IDX_TARGET_POSITION,
  // converter24bits )
  int MotorControllerImpl::getTargetPosition() {
    lock_guard guard(_mutex);
    return retrieveTargetPositonAndConvert();
  }

  int MotorControllerImpl::retrieveTargetPositonAndConvert() {
    int readValue = static_cast<int>(_controllerSPI->read(_id, IDX_TARGET_POSITION).getDATA());
    return _converter24bits.customToThirtyTwo(readValue);
  }

  void MotorControllerImpl::setTargetPosition(int value) {
    lock_guard guard(_mutex);

    InterruptData interupts;
    interupts = readTypedRegister<InterruptData>();

    // Enable all interrupt masks
    interupts.setMaskFlags(255);

    // Enable all interrupts
    interupts.setInterruptFlags(255);
    writeTypedControllerRegister(interupts);

    if(_moveOnlyFullStep) {
      roundToNextFullStep(value);
    }

    _localTargetPosition = value;

    auto writeValue = static_cast<unsigned int>(_converter24bits.thirtyTwoToCustom(value));
    _controllerSPI->write(_id, IDX_TARGET_POSITION, writeValue);
  }

  void MotorControllerImpl::roundToNextFullStep(int& targetPosition) { // todo implementing
    int delta = targetPosition - readPositionRegisterAndConvert();
    int deltaMicroStep = delta * _userMicroStepSize;
    unsigned int actualMicroStepCount = readRegisterAccessor(_microStepCount);
    unsigned int newActualMicroStepCount = static_cast<unsigned int>(actualMicroStepCount + deltaMicroStep) & 0x3FFU;
    unsigned int distanceToPreviousFullStep = (newActualMicroStepCount + 1) % 256;
    if(distanceToPreviousFullStep < 128) {
      targetPosition = targetPosition - distanceToPreviousFullStep / _userMicroStepSize;
    }
    else {
      targetPosition = targetPosition + (256 - distanceToPreviousFullStep) / _userMicroStepSize;
    }
  }

  DEFINE_GET_SET_VALUE(MinimumVelocity, IDX_MINIMUM_VELOCITY)
  DEFINE_SIGNED_GET_SET_VALUE(TargetVelocity, IDX_TARGET_VELOCITY, _converter12bits)
  DEFINE_GET_SET_VALUE(MaximumAcceleration, IDX_MAXIMUM_ACCELERATION)
  DEFINE_GET_SET_VALUE(PositionTolerance, IDX_DELTA_X_REFERENCE_TOLERANCE)
  DEFINE_GET_SET_VALUE(PositionLatched, IDX_POSITION_LATCHED)

  unsigned int MotorControllerImpl::getMaximumVelocity() {
    lock_guard guard(_mutex);
    return _controllerSPI->read(_id, IDX_MAXIMUM_VELOCITY).getDATA();
  }

  void MotorControllerImpl::setMaximumVelocity(unsigned int value) {
    lock_guard guard(_mutex);
    _currentVmax = value;
    _controllerSPI->write(_id, IDX_MAXIMUM_VELOCITY, value);
  }

  template<class T>
  T MotorControllerImpl::readTypedRegister() {
    T typedWord;
    typedWord.setSMDA(_id);
    TMC429OutputWord readbackWord = _controllerSPI->read(_id, typedWord.getIDX_JDX());
    typedWord.setDATA(readbackWord.getDATA());
    return typedWord;
  }

  double MotorControllerImpl::getMaxSpeedCapability() {
    // Does not really need a mutex; shared state is only from _controllerConfig,
    // which is const - hence no risk of data changing during parallel access.
    auto hardLimitForVmax = _controllerConfig.maximumVelocity;
    return (convertVMaxToUstepsPerSec(hardLimitForVmax));
  }

  double MotorControllerImpl::setUserSpeedLimit(double microStepsPerSecond) {
    auto validatedVmax = validateVMaxForHardware(convertUstepsPerSecToVmax(microStepsPerSecond));
    // This is the critical path; already protected by mutex internally.
    setMaximumVelocity(validatedVmax);
    return convertVMaxToUstepsPerSec(validatedVmax);
  }

  double MotorControllerImpl::getUserSpeedLimit() {
    lock_guard guard(_mutex);
    return convertVMaxToUstepsPerSec(_currentVmax);
  }

  void MotorControllerImpl::writeTypedControllerRegister(TMC429InputWord inputWord) {
    // set/overwrite the id with this motors id
    inputWord.setSMDA(_id);
    _controllerSPI->write(inputWord);
  }

  DEFINE_SET_GET_TYPED_CONTROLLER_REGISTER(AccelerationThresholdData)
  DEFINE_SET_GET_TYPED_CONTROLLER_REGISTER(ProportionalityFactorData)
  DEFINE_SET_GET_TYPED_CONTROLLER_REGISTER(ReferenceConfigAndRampModeData)
  DEFINE_SET_GET_TYPED_CONTROLLER_REGISTER(InterruptData)
  DEFINE_SET_GET_TYPED_CONTROLLER_REGISTER(DividersAndMicroStepResolutionData)

  DEFINE_SET_GET_TYPED_DRIVER_DATA(DriverControlData, _driverControlData)
  DEFINE_SET_GET_TYPED_DRIVER_DATA(ChopperControlData, _chopperControlData)
  DEFINE_SET_GET_TYPED_DRIVER_DATA(CoolStepControlData, _coolStepControlData)
  DEFINE_SET_GET_TYPED_DRIVER_DATA(StallGuardControlData, _stallGuardControlData)
  DEFINE_SET_GET_TYPED_DRIVER_DATA(DriverConfigData, _driverConfigData)

  template<class T>
  void MotorControllerImpl::setTypedDriverData(T const& driverData, T& localDataInstance) {
    _driverSPI.write(driverData.getDataWord());
    // Remember the written word for readback.
    localDataInstance = driverData;
  }

  MotorReferenceSwitchData MotorControllerImpl::getReferenceSwitchData() {
    lock_guard guard(_mutex);
    return retrieveReferenceSwitchStatus();
  }

  MotorReferenceSwitchData MotorControllerImpl::retrieveReferenceSwitchStatus() {
    // the bit pattern for the active flags
    unsigned int bitMask = 0x3U << 2 * _id;

    unsigned int commonReferenceSwitchWord = _controllerSPI->read(SMDA_COMMON, JDX_REFERENCE_SWITCH).getDATA();
    unsigned int dataWord = (commonReferenceSwitchWord & bitMask) >> 2 * _id;
    // note: the following code uses the implicit bool conversion to/from 0/1 to
    // keep the code short.
    MotorReferenceSwitchData motorReferenceSwitchData(dataWord);

    auto configAndRampModeData = readTypedRegister<ReferenceConfigAndRampModeData>();
    motorReferenceSwitchData.setNegativeSwitchEnabled(!configAndRampModeData.getDISABLE_STOP_L());
    motorReferenceSwitchData.setPositiveSwitchEnabled(!configAndRampModeData.getDISABLE_STOP_R());

    return motorReferenceSwitchData;
  }

  void MotorControllerImpl::setNegativeReferenceSwitchEnabled(bool enableStatus) {
    lock_guard guard(_mutex);
    auto referenceConfigAndRampModeData = readTypedRegister<ReferenceConfigAndRampModeData>();
    referenceConfigAndRampModeData.setDISABLE_STOP_L(!enableStatus);
    writeTypedControllerRegister(referenceConfigAndRampModeData);
  }

  void MotorControllerImpl::setPositiveReferenceSwitchEnabled(bool enableStatus) {
    lock_guard guard(_mutex);
    auto referenceConfigAndRampModeData = readTypedRegister<ReferenceConfigAndRampModeData>();
    referenceConfigAndRampModeData.setDISABLE_STOP_R(!enableStatus);
    // setReferenceConfigAndRampModeData internally obtains mutex
    writeTypedControllerRegister(referenceConfigAndRampModeData);
  }

  void MotorControllerImpl::setCalibrationTime(uint32_t calibrationTime) {
    lock_guard guard(_mutex);
    _calibrationTime = static_cast<int32_t>(calibrationTime);
    _calibrationTime.write();
  }

  uint32_t MotorControllerImpl::getCalibrationTime() {
    lock_guard guard(_mutex);
    _calibrationTime.read();
    return static_cast<uint32_t>(_calibrationTime);
  }

  void MotorControllerImpl::setPositiveReferenceSwitchCalibration(int calibratedPosition) {
    lock_guard guard(_mutex);
    _endSwitchPositive = calibratedPosition;
    _endSwitchPositive.write();
  }

  int MotorControllerImpl::getPositiveReferenceSwitchCalibration() {
    lock_guard guard(_mutex);
    _endSwitchPositive.read();
    return _endSwitchPositive;
  }

  void MotorControllerImpl::setNegativeReferenceSwitchCalibration(int calibratedPosition) {
    lock_guard guard(_mutex);
    _endSwitchNegative = calibratedPosition;
    _endSwitchNegative.write();
  }

  int MotorControllerImpl::getNegativeReferenceSwitchCalibration() {
    lock_guard guard(_mutex);
    _endSwitchNegative.read();
    return _endSwitchNegative;
  }

  bool MotorControllerImpl::targetPositionReached() {
    lock_guard guard(_mutex);
    _controllerStatus.read();
    TMC429StatusWord controllerStatusWord(_controllerStatus);

    return controllerStatusWord.getTargetPositionReached(_id);
  }

  unsigned int MotorControllerImpl::getReferenceSwitchBit() {
    lock_guard guard(_mutex);
    _controllerStatus.read();
    TMC429StatusWord controllerStatusWord(_controllerStatus);

    return controllerStatusWord.getReferenceSwitchBit(_id);
  }

  double MotorControllerImpl::convertVMaxToUstepsPerSec(double vMax) {
    // speedInUstepsPerSec = _conversionFactor * Vmax
    return (_conversionFactor * vMax);
  }

  double MotorControllerImpl::convertUstepsPerSecToVmax(double speedInUstepsPerSec) {
    // Vmax = round(speedInUstepsPerSec / _conversionFactor)
    return speedInUstepsPerSec / _conversionFactor;
  }

  double MotorControllerImpl::calculateConversionFactor(MotorControllerConfig const& motorControllerConfig) {
    // conversionFactor = fclk[Hz] / (2^pulse_div * 2048 *32)
    double systemClockInMhz = MD_22_DEFAULT_CLOCK_FREQ_MHZ;

    auto systemClkInHz = systemClockInMhz * 1000000;
    auto pulse_div = static_cast<double>(motorControllerConfig.dividersAndMicroStepResolutionData.getPulseDivider());

    auto conversionFactor = systemClkInHz / (exp2(pulse_div) * 2048 * 32);
    return conversionFactor;
  }

  double MotorControllerImpl::setUserCurrentLimit(double currentLimit) {
    auto currentScale = limitCurrentScale(convertAmpsToCurrentScale(currentLimit));
    // setCurrent scale internally acquires mutex and is protected against
    // parallel access.
    setCurrentScale(currentScale);
    return convertCurrentScaletoAmps(currentScale);
  }

  double MotorControllerImpl::getUserCurrentLimit() {
    lock_guard guard(_mutex);
    return convertCurrentScaletoAmps(_usrSetCurrentScale);
  }

  double MotorControllerImpl::getMaxCurrentLimit() {
    // Does not really need to be protected with mutex; there is no resource
    // that can be modified on another thread. _controllerConfig is a const
    // object.
    auto configuredCurrentScale = _controllerConfig.stallGuardControlData.getCurrentScale();
    return convertCurrentScaletoAmps(configuredCurrentScale);
  }

  unsigned int MotorControllerImpl::validateVMaxForHardware(double calculatedVmax) {
    unsigned int maxLimitForVmax = _controllerConfig.maximumVelocity;
    unsigned int minLimitForVmax = _controllerConfig.minimumVelocity;
    if(calculatedVmax > maxLimitForVmax) {
      return maxLimitForVmax;
    }
    if(calculatedVmax < minLimitForVmax) {
      return minLimitForVmax;
    }
    return static_cast<unsigned int>(std::floor(calculatedVmax)); // floor the VMax value so we stick below
                                                                  // the requested speed limit.
  }

  double MotorControllerImpl::convertCurrentScaletoAmps(unsigned int currentScale) {
    auto currentInAmps =
        (static_cast<double>(currentScale + 1) / iMaxTMC260C_CURRENT_SCALE_VALUES) * iMaxTMC260C_IN_AMPS;
    return currentInAmps;
  }

  double MotorControllerImpl::convertAmpsToCurrentScale(double currentInAmps) {
    auto currentScale = (currentInAmps / iMaxTMC260C_IN_AMPS * iMaxTMC260C_CURRENT_SCALE_VALUES) - 1;
    return currentScale;
  }

  unsigned int MotorControllerImpl::limitCurrentScale(double calculatedCurrentScale) {
    auto hardLimitForCurrentScale = _controllerConfig.stallGuardControlData.getCurrentScale();
    if(calculatedCurrentScale < iMaxTMC260C_MIN_CURRENT_SCALE_VALUE) {
      return iMaxTMC260C_MIN_CURRENT_SCALE_VALUE;
    }
    if(calculatedCurrentScale > hardLimitForCurrentScale) {
      return hardLimitForCurrentScale;
    }
    return (static_cast<unsigned int>(std::floor(calculatedCurrentScale))); // floor so that we dont exceed the set limt
  }

  bool MotorControllerImpl::isMotorMoving() {
    lock_guard guard(_mutex);
    auto interruptData = readTypedRegister<InterruptData>();

    // Check if we got any of the interrupts that tells us that the motor has stopped
    // POS_END -> Target position was reached
    // STOP -> Reference switch stopped motor
    if(interruptData.getINT_POS_END() || interruptData.getINT_STOP()) {
      DriverStatusData status(readRegisterAccessor(_status));
      return status.getStandstillIndicator() == 0;
    }
    int currentPos = readPositionRegisterAndConvert();

    // There was no interrupt, compare positions
    if(_localTargetPosition == currentPos) {
      DriverStatusData status(readRegisterAccessor(_status));
      return status.getStandstillIndicator() == 0;
    }

    // The positions did not match. Check if we ran into one of the reference switches, without triggering INT_STOP()
    auto referenceSwitchData = retrieveReferenceSwitchStatus();
    // ignore negative switch for rotary motors.
    if((referenceSwitchData.getNegativeSwitchEnabled() && referenceSwitchData.getNegativeSwitchActive() &&
           _localTargetPosition <= currentPos) ||
        (referenceSwitchData.getPositiveSwitchEnabled() && referenceSwitchData.getPositiveSwitchActive() &&
            _localTargetPosition >= currentPos)) {
      DriverStatusData status(readRegisterAccessor(_status));
      return status.getStandstillIndicator() == 0;
    }

    return true;
  }

  void MotorControllerImpl::setMotorCurrentEnabled(bool enable) {
    lock_guard guard(_mutex);
    _motorCurrentEnabled = (enable ? 1 : 0);
    _motorCurrentEnabled.write();
  }

  bool MotorControllerImpl::isMotorCurrentEnabled() {
    lock_guard guard(_mutex);
    return readRegisterAccessor(_motorCurrentEnabled);
  }

  void MotorControllerImpl::setEndSwitchPowerEnabled(bool enable) {
    lock_guard guard(_mutex);
    if(!_endSwitchPowerIndicator.isInitialised()) {
      return; // Meaning we are on an old firmware that does not support this
              // register. Do nothing
    }
    _endSwitchPowerIndicator = (enable ? 3 : 0);
    _endSwitchPowerIndicator.write();
  }

  bool MotorControllerImpl::isEndSwitchPowerEnabled() {
    lock_guard guard(_mutex);
    if(!_endSwitchPowerIndicator.isInitialised()) {
      return false; // taking a convenient default to make logic easier in
      // StepperMotor getEnabled . With the old firmware status of the end
      // switches is a don't care condition anyway
    }
    return readRegisterAccessor(_endSwitchPowerIndicator);
  }

  void MotorControllerImpl::setCurrentScale(unsigned int currentScale) {
    auto stallGuardData = _controllerConfig.stallGuardControlData;
    stallGuardData.setCurrentScale(currentScale);
    // mutex is acquired in setStallGuardControlData
    setStallGuardControlData(stallGuardData);
    _usrSetCurrentScale = currentScale;
  }

} // namespace ChimeraTK::MotorDriver

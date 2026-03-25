#ifndef MTCA4U_MOTOR_CONTROLER_IMPL_H
#define MTCA4U_MOTOR_CONTROLER_IMPL_H

#include "MotorControlerConfig.h"
#include "MotorControlerExpert.h"
#include "SignedIntConverter.h"
#include "SPIviaPCIe.h"
#include "TMC429SPI.h"

#include <ChimeraTK/Device.h>

#include <atomic>
#include <cstdint>
#include <mutex>

#define MCI_DECLARE_SET_GET_VALUE(NAME, VARIABLE_IN_UNITS)                                                             \
  void set##NAME(unsigned int VARIABLE_IN_UNITS) override;                                                             \
  unsigned int get##NAME() override

#define MCI_DECLARE_SIGNED_SET_GET_VALUE(NAME, VARIABLE_IN_UNITS)                                                      \
  void set##NAME(int VARIABLE_IN_UNITS) override;                                                                      \
  int get##NAME() override

#define MCI_DECLARE_SET_GET_TYPED_REGISTER(NAME, VARIABLE_NAME)                                                        \
  void set##NAME(NAME const& VARIABLE_NAME) override;                                                                  \
  NAME get##NAME() override

namespace mtca4u {
  class MotorDriverCardImpl;

  class MotorControlerImpl : public MotorControlerExpert {
   public:
    /** The constructor gets references shared pointers which it copies
     * internally, so the object stays valid even if the original shared pointer
     * goes out of scope. The config is only used in the constuctor, no reference
     * is kept in the class.
     */
    MotorControlerImpl(unsigned int ID, boost::shared_ptr<ChimeraTK::Device> const& device,
        std::string const& moduleName, boost::shared_ptr<TMC429SPI> const& controlerSPI,
        MotorControlerConfig const& motorControlerConfig);

    /// The class is non-copyable
    MotorControlerImpl(MotorControlerImpl const&) = delete;
    /// The class is non-assignable
    MotorControlerImpl& operator=(MotorControlerImpl const&) = delete;

    unsigned int getID() override;
    int getActualPosition() override;
    int getActualVelocity() override;
    unsigned int getActualAcceleration() override;
    unsigned int getMicroStepCount() override;
    unsigned int getStallGuardValue() override;
    unsigned int getCoolStepValue() override;
    DriverStatusData getStatus() override;
    unsigned int getDecoderReadoutMode() override;
    unsigned int getDecoderPosition() override;

    void setActualVelocity(int stepsPerFIXME);
    void setActualAcceleration(unsigned int stepsPerSquareFIXME) override;
    void setMicroStepCount(unsigned int microStepCount) override;
    void setEnabled(bool enable = true) override;
    void setDecoderReadoutMode(unsigned int decoderReadoutMode) override;

    /*
     * @brief Sets the desired upper limit for motor speed in ramp mode. The
     * method may not set the exact desired speed limit, but a value closest to
     * the desired limit, as determined by the operating parameters.
     *
     * @param microStepsPerSecond The desired maximum motor speed during
     *                            operation; unit is microsteps per
     *                            second
     *
     * @return Returns the motor speed that was actually set. This speed may not
     * be what the user requested, but a value closest to it as permitted by the
     * motor controller parameters
     */
    double setUserSpeedLimit(double microStepsPerSecond) override;

    /*
     * @brief Returns the current upper limit for motor speed. Returned speed is
     * in microsteps per second
     */
    double getUserSpeedLimit() override;

    /*
     * @brief Returns the maximum speed the motor is capable of achieving. Speed
     * is expressed in microsteps per second
     */
    double getMaxSpeedCapability() override;

    /*
     * @brief Sets the maximum current the driver should limit the motor to
     *
     */
    double setUserCurrentLimit(double currentLimit) override;

    double getUserCurrentLimit() override;

    double getMaxCurrentLimit() override;

    void enableFullStepping(bool enable = true) override;
    bool isFullStepping() override;

    void setCalibrationTime(uint32_t calibrationTime) override;
    uint32_t getCalibrationTime() override;

    /**
     * @brief Writes a calibrated position for the positive end switch to the
     * firmware
     */
    void setPositiveReferenceSwitchCalibration(int calibratedPosition) override;

    /**
     * @brief Read a calibrated position for the positive end switch from the
     * firmware
     */
    int getPositiveReferenceSwitchCalibration() override;

    /**
     * @brief Writes a calibrated position for the negative end switch to the
     * firmware
     */
    void setNegativeReferenceSwitchCalibration(int calibratedPosition) override;

    /**
     * @brief Read a calibrated position for the negative end switch from the
     * firmware
     */
    int getNegativeReferenceSwitchCalibration() override;

    bool isEnabled() override;

    void setMotorCurrentEnabled(bool enable = true) override;
    bool isMotorCurrentEnabled() override;

    void setEndSwitchPowerEnabled(bool enable = true) override;
    bool isEndSwitchPowerEnabled() override;

    /**
     * @brief Returns True if the motor is in motion or False if at
     * standstill. This command is reliable as long as the motor is not stalled.
     *
     * @details
     * Motor status is decided by monitoring the Standstill indicator bit of the
     * TMC260 driver IC. The standstill indicator bit can reliably track
     * movement as long as the motor is not stalled. When movement has stalled,
     * the controller IC still sends STEP pulses to the driver IC. As a result,
     * for this situation, the Standstill indicator bit indicates movement
     * though the motor is actually stalled and not moving. Once the controller
     * chip has stopped with the STEP pulses, the stand still indicator
     * concludes that the movement is complete.
     *
     * This method has a minimum internal delay of COMMUNICATION_DELAY before
     * returning. This delay compensates for the time it takes for the
     * standstill indicator bit to be updated after the X_TARGET register on the
     * controller has been set.
     */
    bool isMotorMoving() override;

    MotorReferenceSwitchData getReferenceSwitchData() override;

    void setPositiveReferenceSwitchEnabled(bool enableStatus) override;
    void setNegativeReferenceSwitchEnabled(bool enableStatus) override;

    void setActualPosition(int steps);
    MCI_DECLARE_SIGNED_SET_GET_VALUE(TargetPosition, steps);
    MCI_DECLARE_SET_GET_VALUE(MinimumVelocity, stepsPerFIXME);
    MCI_DECLARE_SET_GET_VALUE(MaximumVelocity, stepsPerFIXME);
    MCI_DECLARE_SIGNED_SET_GET_VALUE(TargetVelocity, stepsPerFIXME);
    MCI_DECLARE_SET_GET_VALUE(MaximumAcceleration, stepsPerSquareFIXME);
    MCI_DECLARE_SET_GET_VALUE(PositionTolerance, steps);
    MCI_DECLARE_SET_GET_VALUE(PositionLatched, steps);

    MCI_DECLARE_SET_GET_TYPED_REGISTER(AccelerationThresholdData, accelerationThresholdData);
    MCI_DECLARE_SET_GET_TYPED_REGISTER(ProportionalityFactorData, proportionalityFactorData);
    MCI_DECLARE_SET_GET_TYPED_REGISTER(ReferenceConfigAndRampModeData, referenceConfigAndRampModeData);
    MCI_DECLARE_SET_GET_TYPED_REGISTER(InterruptData, interruptData);
    MCI_DECLARE_SET_GET_TYPED_REGISTER(DividersAndMicroStepResolutionData, dividersAndMicroStepResolutionData);

    void setDriverControlData(DriverControlData const& driverControlData) override;
    void setChopperControlData(ChopperControlData const& chopperControlData) override;
    void setCoolStepControlData(CoolStepControlData const& coolStepControlData) override;
    void setStallGuardControlData(StallGuardControlData const& stallGuardControlData) override;
    void setDriverConfigData(DriverConfigData const& driverConfigData) override;

    DriverControlData const& getDriverControlData() const override;
    ChopperControlData const& getChopperControlData() const override;
    CoolStepControlData const& getCoolStepControlData() const override;
    StallGuardControlData const& getStallGuardControlData() const override;
    DriverConfigData const& getDriverConfigData() const override;

    bool targetPositionReached() override;

    unsigned int getReferenceSwitchBit() override;

   private:
    static const unsigned int COMMUNICATION_DELAY = 20000; /// in microseconds
    // Reason for mtable keyword:
    // http://stackoverflow.com/questions/25521570/can-mutex-locking-function-be-marked-as-const
    mutable std::mutex _mutex;
    boost::shared_ptr<ChimeraTK::Device> _device;

    unsigned int _id;
    const MotorControlerConfig _controlerConfig;

    /*
     * @brief:
     *       _conversionFactor = fclk[Hz] / (2^pulse_div * 2048 *32)
     *       speedInUstepsPerSec = _conversionFactor * Vmax
     */
    double _conversionFactor;
    unsigned int _currentVmax;
    unsigned int _usrSetCurrentScale;

    constexpr static double iMaxTMC260C_IN_AMPS = 1.8;
    constexpr static int iMaxTMC260C_CURRENT_SCALE_VALUES = 32;
    constexpr static unsigned int iMaxTMC260C_MIN_CURRENT_SCALE_VALUE = 0;

    // FIXME: These variables have to be stored uniquely inter process
    DriverControlData _driverControlData;
    ChopperControlData _chopperControlData;
    CoolStepControlData _coolStepControlData;
    StallGuardControlData _stallGuardControlData;
    DriverConfigData _driverConfigData;

    ChimeraTK::ScalarRegisterAccessor<int32_t> _controlerStatus;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _actualPosition;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _actualVelocity;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _actualAcceleration;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _microStepCount;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _stallGuardValue;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _coolStepValue;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _status;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _motorCurrentEnabled;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _decoderReadoutMode;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _decoderPosition;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _calibrationTime;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _endSwitchPositive;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _endSwitchNegative;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _endSwitchPowerIndicator;

    mtca4u::SPIviaPCIe _driverSPI;
    boost::shared_ptr<mtca4u::TMC429SPI> _controlerSPI;

    static const unsigned int MD_22_DEFAULT_CLOCK_FREQ_MHZ = 32;

    template<class T>
    T readTypedRegister();

    void writeTypedControlerRegister(TMC429InputWord inputWord);

    template<class T>
    void setTypedDriverData(T const& driverData, T& localDataInstance);

    SignedIntConverter _converter24bits;
    SignedIntConverter _converter12bits;

    std::atomic<bool> _moveOnlyFullStep;
    // unsigned int _microStepsPerFullStep;
    unsigned int _userMicroStepSize;

    int _localTargetPosition;
    // bool _positiveSwitch, _negativeSwitch;
    int retrieveTargetPositonAndConvert();
    int readPositionRegisterAndConvert();
    MotorReferenceSwitchData retrieveReferenceSwitchStatus();

    double convertVMaxToUstepsPerSec(double vMax);
    double convertUstepsPerSecToVmax(double speedInUstepsPerSec);
    static double calculateConversionFactor(MotorControlerConfig const& motorControlerConfig);
    unsigned int validateVMaxForHardware(double calculatedVmax);

    double convertCurrentScaletoAmps(unsigned int currentScale);
    double convertAmpsToCurrentScale(double currentInAmps);
    unsigned int limitCurrentScale(double calculatedCurrentScale);
    void setCurrentScale(unsigned int currentScale);

    void roundToNextFullStep(int& targetPosition);

    inline unsigned int readRegisterAccessor(ChimeraTK::ScalarRegisterAccessor<int32_t>& readValue);
  };

} // namespace mtca4u

#endif // MTCA4U_MOTOR_CONTROLER_IMPL_H

#ifndef MTCA4U_MOTOR_CONTROLER_IMPL_H
#define MTCA4U_MOTOR_CONTROLER_IMPL_H

#include "MotorControlerExpert.h"

#include <mutex>

#include "MotorControlerConfig.h"
#include "SignedIntConverter.h"
#include "SPIviaPCIe.h"
#include "TMC429SPI.h"
#include <mtca4u/Device.h>
#include  <cstdint>

// Declatations needed in this header.
// We intentionally do not include the mtca4u:Device header here
// in order not to expose its interface. It is not part of the public API 
// of this library.
namespace mtca4u{
  class Device;
  class RegisterAccessor;
}

#define MCI_DECLARE_SET_GET_VALUE( NAME, VARIABLE_IN_UNITS )\
  void set ## NAME (unsigned int VARIABLE_IN_UNITS );	\
  unsigned int get ## NAME ()

#define MCI_DECLARE_SIGNED_SET_GET_VALUE( NAME, VARIABLE_IN_UNITS )\
  void set ## NAME (int VARIABLE_IN_UNITS );	\
  int get ## NAME ()

#define MCI_DECLARE_SET_GET_TYPED_REGISTER( NAME, VARIABLE_NAME )\
  void set ## NAME ( NAME const & VARIABLE_NAME );\
  NAME get ## NAME ()

namespace mtca4u
{
  class MotorDriverCardImpl;

  class MotorControlerImpl: public MotorControlerExpert{

  public:
    /** The constructor gets references shared pointers which it copies internally, so the
     *  object stays valid even if the original shared pointer goes out of scope.
     *  The config is only used in the constuctor, no reference is kept in the class.
     */
    MotorControlerImpl(unsigned int ID, boost::shared_ptr<Device> const& device,
                       std::string const& moduleName,
                       boost::shared_ptr<TMC429SPI> const& controlerSPI,
                       MotorControlerConfig const& motorControlerConfig);

    /// The class is non-copyable
    MotorControlerImpl(MotorControlerImpl const &)=delete;
    /// The class is non-assignable
    MotorControlerImpl & operator=(MotorControlerImpl const &)=delete;

    unsigned int getID();
    int getActualPosition();
    int getActualVelocity();
    unsigned int getActualAcceleration();
    unsigned int getMicroStepCount();
    unsigned int getStallGuardValue();
    unsigned int getCoolStepValue();
    DriverStatusData getStatus();
    unsigned int getDecoderReadoutMode();
    unsigned int getDecoderPosition();
    
    void setActualVelocity(int stepsPerFIXME);
    void setActualAcceleration(unsigned int stepsPerSquareFIXME);
    void setMicroStepCount(unsigned int microStepCount);
    void setEnabled(bool enable=true);
    void setDecoderReadoutMode(unsigned int decoderReadoutMode);

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
    virtual double setUserSpeedLimit(double microStepsPerSecond);

    /*
     * @brief Returns the current upper limit for motor speed. Returned speed is
     * in microsteps per second
     */
    virtual double getUserSpeedLimit();

    /*
     * @brief Returns the maximum speed the motor is capable of achieving. Speed
     * is expressed in microsteps per second
     */
    virtual double getMaxSpeedCapability();

    /*
     * @brief Sets the maximum current the driver should limit the motor to
     *
     */
    virtual double setUserCurrentLimit(double currentLimit);

    virtual double getUserCurrentLimit();

    virtual double getMaxCurrentLimit();

    virtual void enableFullStepping(bool enable=true);
    virtual bool isFullStepping();

    virtual void setCalibrationTime(uint32_t calibrationTime);
    virtual uint32_t getCalibrationTime();

    bool isEnabled();

    virtual void setMotorCurrentEnabled(bool enable = true);
    virtual bool isMotorCurrentEnabled();

    virtual void setEndSwitchPowerEnabled(bool enable = true);
    virtual bool isEndSwitchPowerEnabled();


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
    virtual bool isMotorMoving();

    MotorReferenceSwitchData getReferenceSwitchData();

    void setPositiveReferenceSwitchEnabled(bool enableStatus);
    void setNegativeReferenceSwitchEnabled(bool enableStatus);

    void setActualPosition(int steps);
    MCI_DECLARE_SIGNED_SET_GET_VALUE( TargetPosition, steps );
    MCI_DECLARE_SET_GET_VALUE( MinimumVelocity, stepsPerFIXME );
    MCI_DECLARE_SET_GET_VALUE( MaximumVelocity, stepsPerFIXME );
    MCI_DECLARE_SIGNED_SET_GET_VALUE( TargetVelocity, stepsPerFIXME );
    MCI_DECLARE_SET_GET_VALUE( MaximumAcceleration, stepsPerSquareFIXME );
    MCI_DECLARE_SET_GET_VALUE( PositionTolerance, steps );
    MCI_DECLARE_SET_GET_VALUE( PositionLatched, steps );

    MCI_DECLARE_SET_GET_TYPED_REGISTER(AccelerationThresholdData,
				       accelerationThresholdData);
    MCI_DECLARE_SET_GET_TYPED_REGISTER(ProportionalityFactorData,
				       proportionalityFactorData);
    MCI_DECLARE_SET_GET_TYPED_REGISTER(ReferenceConfigAndRampModeData,
				       referenceConfigAndRampModeData);
    MCI_DECLARE_SET_GET_TYPED_REGISTER(InterruptData, interruptData);
    MCI_DECLARE_SET_GET_TYPED_REGISTER(DividersAndMicroStepResolutionData, 
				       dividersAndMicroStepResolutionData);

     void setDriverControlData(DriverControlData const & driverControlData);
     void setChopperControlData(ChopperControlData const &  chopperControlData);
     void setCoolStepControlData(CoolStepControlData const &  coolStepControlData);
     void setStallGuardControlData(StallGuardControlData const &  stallGuardControlData);
     void setDriverConfigData(DriverConfigData const &  driverConfigData);

     DriverControlData const & getDriverControlData() const;
     ChopperControlData const & getChopperControlData() const;
     CoolStepControlData const & getCoolStepControlData() const;
     StallGuardControlData const & getStallGuardControlData() const;
     DriverConfigData const & getDriverConfigData() const;

     bool targetPositionReached();

     unsigned int getReferenceSwitchBit();
 
  private:

     static const unsigned int COMMUNICATION_DELAY= 20000; /// in microseconds
     // Reason for mtable keyword:
     // http://stackoverflow.com/questions/25521570/can-mutex-locking-function-be-marked-as-const
     mutable std::mutex _mutex;
     boost::shared_ptr< Device > _device;

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

     //FIXME: These variables have to be stored uniquely inter process
     DriverControlData _driverControlData;
     ChopperControlData _chopperControlData;
     CoolStepControlData _coolStepControlData;
     StallGuardControlData _stallGuardControlData;
     DriverConfigData _driverConfigData;

     boost::shared_ptr< mtca4u::RegisterAccessor > _controlerStatus;

     boost::shared_ptr< mtca4u::RegisterAccessor > _actualPosition;
     boost::shared_ptr< mtca4u::RegisterAccessor > _actualVelocity;
     boost::shared_ptr< mtca4u::RegisterAccessor > _actualAcceleration;
     boost::shared_ptr< mtca4u::RegisterAccessor > _microStepCount;
     boost::shared_ptr< mtca4u::RegisterAccessor > _stallGuardValue;
     boost::shared_ptr< mtca4u::RegisterAccessor > _coolStepValue;
     boost::shared_ptr< mtca4u::RegisterAccessor > _status;
     boost::shared_ptr< mtca4u::RegisterAccessor > _motorCurrentEnabled;
     boost::shared_ptr< mtca4u::RegisterAccessor > _decoderReadoutMode;
     boost::shared_ptr< mtca4u::RegisterAccessor > _decoderPosition;
     boost::shared_ptr< mtca4u::RegisterAccessor > _endSwithPowerIndicator;
     boost::shared_ptr< mtca4u::RegisterAccessor > _calibrationTime;
     
     mtca4u::SPIviaPCIe _driverSPI;
     boost::shared_ptr<mtca4u::TMC429SPI>  _controlerSPI;

     static const unsigned int MD_22_DEFAULT_CLOCK_FREQ_MHZ = 32;

     /// Simplify the syntax to read from a RegisterAccessor which need call by reference.
     /// Remove this function once the RegisterAccessor interface has been fixed.
     unsigned int readRegisterAccessor( 
			 boost::shared_ptr< mtca4u::RegisterAccessor > const & registerAccessor);

     template<class T>
       T readTypedRegister();

     void writeTypedControlerRegister(TMC429InputWord inputWord);

     template <class T>
       void setTypedDriverData(T const & driverData, T & localDataInstance);

     SignedIntConverter converter24bits;
     SignedIntConverter converter12bits;

     bool _moveOnlyFullStep;
     //unsigned int _microStepsPerFullStep;
     unsigned int _userMicroStepSize;

     int _localTargetPosition;
     //bool _positiveSwitch, _negativeSwitch;
     int retrieveTargetPositonAndConvert();
     int readPositionRegisterAndConvert();
     MotorReferenceSwitchData retrieveReferenceSwitchStatus();

     double convertVMaxToUstepsPerSec(double vMax);
     double convertUstepsPerSecToVmax(double speedInUstepsPerSec);
     static double calculateConversionFactor(MotorControlerConfig const & motorControlerConfig);
     unsigned int validateVMaxForHardware(double calculatedVmax);

     double convertCurrentScaletoAmps(unsigned int currentScale);
     double convertAmpsToCurrentScale(double currentInAmps);
     unsigned int limitCurrentScale(double calculatedCurrentScale);
     void setCurrentScale(unsigned int currentScale);

     void roundToNextFullStep(int &targetPosition);
  };

}// namespace mtca4u

#endif // MTCA4U_MOTOR_CONTROLER_IMPL_H

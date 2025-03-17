#ifndef MTCA4U_MOTOR_CONTROLER_DUMMY_H
#define MTCA4U_MOTOR_CONTROLER_DUMMY_H

#include "MotorControler.h"

#include <mutex>

// NOLINTBEGIN(bugprone-macro-parentheses)
#define MCD_DECLARE_SET_GET_VALUE(NAME, VARIABLE_IN_UNITS)                                                             \
  void set##NAME(unsigned int VARIABLE_IN_UNITS) override;                                                             \
  unsigned int get##NAME() override

#define MCD_DECLARE_SIGNED_SET_GET_VALUE(NAME, VARIABLE_IN_UNITS)                                                      \
  void set##NAME(int VARIABLE_IN_UNITS) override;                                                                      \
  int get##NAME() override

#define MCD_DECLARE_SET_GET_TYPED_REGISTER(NAME, VARIABLE_NAME)                                                        \
  void set##NAME(NAME const& VARIABLE_NAME) override;                                                                  \
  NAME get##NAME() override
// NOLINTEND(bugprone-macro-parentheses)

namespace mtca4u {

  /** The MotorControlerDummy only implements the
   *  basic functionality defined in the MotorControler class,
   *  not the full functionality defined in MotorControlerExpert.
   *
   *  It emulates a real stepper motor with a hardware position and
   *  hardware end switches. The motion of the motor can be controlled
   *  with the additional functions in this class.
   */
  class MotorControlerDummy : public MotorControler {
   public:
    explicit MotorControlerDummy(unsigned int id);

    // inherited functions

    unsigned int getID() override;
    int getActualPosition() override;
    int getActualVelocity() override;
    unsigned int getActualAcceleration() override;
    unsigned int getMicroStepCount() override;

    DriverStatusData getStatus() override;
    unsigned int getDecoderReadoutMode() override;
    /// The negative end switch position is decoder 0.
    unsigned int getDecoderPosition() override;

    void setActualVelocity(int stepsPerFIXME);
    void setActualAcceleration(unsigned int stepsPerSquareFIXME) override;
    void setMicroStepCount(unsigned int microStepCount) override;
    void setEnabled(bool enable) override;
    void setDecoderReadoutMode(unsigned int decoderReadoutMode) override;

    void enableFullStepping(bool enable) override;
    bool isFullStepping() override;

    void setCalibrationTime(uint32_t calibrationTime) override;
    uint32_t getCalibrationTime() override;

    void setPositiveReferenceSwitchCalibration(int calibratedPosition) override;
    int getPositiveReferenceSwitchCalibration() override;
    void setNegativeReferenceSwitchCalibration(int calibratedPosition) override;
    int getNegativeReferenceSwitchCalibration() override;

    bool isEnabled() override;

    MotorReferenceSwitchData getReferenceSwitchData() override;

    void setPositiveReferenceSwitchEnabled(bool enableStatus) override;
    void setNegativeReferenceSwitchEnabled(bool enableStatus) override;

    void setActualPosition(int steps) override;
    MCD_DECLARE_SIGNED_SET_GET_VALUE(TargetPosition, steps);
    MCD_DECLARE_SET_GET_VALUE(MinimumVelocity, stepsPerFIXME);
    MCD_DECLARE_SET_GET_VALUE(MaximumVelocity, stepsPerFIXME);
    MCD_DECLARE_SIGNED_SET_GET_VALUE(TargetVelocity, stepsPerFIXME);
    MCD_DECLARE_SET_GET_VALUE(MaximumAcceleration, stepsPerSquareFIXME);
    MCD_DECLARE_SET_GET_VALUE(PositionTolerance, steps);
    MCD_DECLARE_SET_GET_VALUE(PositionLatched, steps);

    bool targetPositionReached() override;
    unsigned int getReferenceSwitchBit() override;

    // dummy specific functions
    /** Moves the actual position towards the target position, as long as
     *  no end switch is reached.
     *
     *  @param fraction The fraction of the full way to the target position.
     *   Valid range [0..1]. 1 moves all the way to the target, 0.5 only half the
     * way etc.
     *  @param blockMotor Block the motor and do not move if true.
     *  @param bothEndSwitchesAlwaysOn Emulate that no motor is connected (both
     * end switches are active)
     *  @param zeroPositions FIXME: what exactly does this do? TODO Find out
     *
     *  @attention The 'actual' position is where the step counter currently is.
     *  it is not the hardware position (absolute position). If the motor is
     * disabled the motor is stepping and the target position will be reached if
     * the motor is not at an end switch. The motor will not be moving, tough.
     */
    void moveTowardsTarget(
        float fraction, bool blockMotor = false, bool bothEndSwitchesAlwaysOn = false, bool zeroPositions = false);

    double setUserSpeedLimit(double microStepsPerSecond) override;
    double getUserSpeedLimit() override;
    double getMaxSpeedCapability() override;

    double setUserCurrentLimit(double currentLimit) override;
    double getUserCurrentLimit() override;
    double getMaxCurrentLimit() override;

    void setMotorCurrentEnabled(bool enable) override;
    bool isMotorCurrentEnabled() override;

    void setEndSwitchPowerEnabled(bool enable) override;
    bool isEndSwitchPowerEnabled() override;

    /**
     * Block the motor and do not move if true. simulateBlockedMotor(true) is an
     * alternative to calling moveTowardsTarget with the blockMotor flag set to
     * true.
     */
    void simulateBlockedMotor(bool state);

    /**
     * Does the following:
     * - Current, target and absolute positions reset to zero.
     * - Both positive and negative end switches are enabled.
     * - Sets the internal state _bothEndSwitchesAlwaysOn to false.
     * - Sets calibration time to zero (i.e. not calibrated)
     * - blockMotor status set to false.
     */
    void resetInternalStateToDefaults();

    bool isMotorMoving() override;

    void setPositiveEndSwitch(int endSwitchPos);
    int getPositiveEndSwitch();
    void setNegativeEndSwitch(int endSwitchNeg);
    int getNegativeEndSwitch();

   private:
    mutable std::mutex _motorControllerDummyMutex;
    int _hardwarePosition{0}; ///< Like the real absolute position of a motor, in
                              ///< steps

    int _targetPosition{0};  ///< Target position in steps
    int _currentPosition{0}; ///< The current position can be set by the user
                             ///< (calibration)

    uint32_t _calibrationTime{0};

    bool _positiveEndSwitchEnabled{true}; ///< Flag whether the positive end switch is
                                          ///< being used
    bool _negativeEndSwitchEnabled{true}; ///< Flag whether the negative end switch is
                                          ///< being used
    bool _motorCurrentEnabled{false};     ///< Flag indicating if motor current (driver chip)
                                          ///< is enabled
    bool _endSwitchPowerEnabled{false};   ///< Flag indicating if the end switches are
                                          ///< powered up
    ///< not change if this flag is false.
    int _positiveEndSwitchHardwarePosition{10000};
    int _positiveEndSwitchPosition{10000};
    int _negativeEndSwitchHardwarePosition{-10000};
    int _negativeEndSwitchPosition{-10000};

    double _userSpeedLimit{100000}; // Arbitrary high value

    unsigned int _id;

    /** determines if the motor is considered as "stepping". This is the case
     *  if the target position is not reached, no end switch is reached and
     *  no error has been detected. In this case the current position is moving
     *  towards the target position. The motor will only move if it is enabled.
     *  In this case also the absolute position is changing and an end switch can
     *  be activated which was inactive before.
     */
    bool isStepping();

    bool isPositiveEndSwitchActive();
    bool isNegativeEndSwitchActive();

    bool _blockMotor{false};
    bool _bothEndSwitchesAlwaysOn{false};
    unsigned int _userMicroStepSize{4};
    bool _isFullStepping{false};
    void setMicroStepSize(unsigned int microStepSize) { _userMicroStepSize = microStepSize; }
    unsigned int getMicropStepSize() const { return _userMicroStepSize; }
    void roundToNextFullStep(int& targetPosition);

    uint32_t _encoderReadoutMode{DecoderReadoutMode::HEIDENHAIN};
  };

} // namespace mtca4u

#endif // MTCA4U_MOTOR_CONTROLER_DUMMY_H

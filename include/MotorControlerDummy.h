#ifndef MTCA4U_MOTOR_CONTROLER_DUMMY_H
#define MTCA4U_MOTOR_CONTROLER_DUMMY_H

#include "MotorControler.h"

#define MCD_DECLARE_SET_GET_VALUE( NAME, VARIABLE_IN_UNITS )\
  void set ## NAME (unsigned int VARIABLE_IN_UNITS );	\
  unsigned int get ## NAME ()

#define MCD_DECLARE_SIGNED_SET_GET_VALUE( NAME, VARIABLE_IN_UNITS )\
  void set ## NAME (int VARIABLE_IN_UNITS );	\
  int get ## NAME ()

#define MCD_DECLARE_SET_GET_TYPED_REGISTER( NAME, VARIABLE_NAME )\
  void set ## NAME ( NAME const & VARIABLE_NAME );\
  NAME get ## NAME ()

namespace mtca4u{

  /** The MotorControlerDummy only implements the 
   *  basic functionality defined in the MotorControler class,
   *  not the full functionality defined in MotorControlerExpert.
   *
   *  It emulates a real stepper motor with a hardware positon and
   *  hardware end switches. The motion of the motor can be controlled 
   *  with the additional functions in this class.
   *  The end switch positions are hard coded at +-10000.
   */
  class MotorControlerDummy: public MotorControler {
  public:
    MotorControlerDummy(unsigned int id);

    // inherited functions 

    unsigned int getID();
    int getActualPosition();
    int getActualVelocity();
    unsigned int getActualAcceleration();
    unsigned int getMicroStepCount();

    DriverStatusData getStatus();
    unsigned int getDecoderReadoutMode();
    /// The negative end switch position is decoder 0.
    unsigned int getDecoderPosition();
    
    void setActualVelocity(int stepsPerFIXME);
    void setActualAcceleration(unsigned int stepsPerSquareFIXME);
    void setMicroStepCount(unsigned int microStepCount);
    void setEnabled(bool enable=true);
    void setDecoderReadoutMode(unsigned int decoderReadoutMode);

    bool isEnabled();
    
    MotorReferenceSwitchData getReferenceSwitchData();

    void setPositiveReferenceSwitchEnabled(bool enableStatus);
    void setNegativeReferenceSwitchEnabled(bool enableStatus);

    void setActualPosition(int steps);
    MCD_DECLARE_SIGNED_SET_GET_VALUE( TargetPosition, steps );
    MCD_DECLARE_SET_GET_VALUE( MinimumVelocity, stepsPerFIXME );
    MCD_DECLARE_SET_GET_VALUE( MaximumVelocity, stepsPerFIXME );
    MCD_DECLARE_SIGNED_SET_GET_VALUE( TargetVelocity, stepsPerFIXME );
    MCD_DECLARE_SET_GET_VALUE( MaximumAcceleration, stepsPerSquareFIXME );
    MCD_DECLARE_SET_GET_VALUE( PositionTolerance, steps );
    MCD_DECLARE_SET_GET_VALUE( PositionLatched, steps );

    bool targetPositionReached();
    unsigned int getReferenceSwitchBit();

    // dummy specific functions
    /** Moves the actual position towards the target position, as long as 
     *  no end switch is reached.
     *  @param fraction The fraction of the full way to the target position.
     *   Valid range [0..1]. 1 moves all the way to the target, 0.5 only half the way etc.
     *  @attention The 'actual' position is where the step counter currently is.
    *  it is not the hardware positon (absolute position). If the motor is disabled
    *  the motor is stepping and the target position will be reached if the 
    *  motor is not at an end switch. The motor will not be moving, tough.
    */
    void moveTowardsTarget(float fraction);

    // FIXME : Which errors can occur?

  private:
    int _absolutePosition; ///< Like the real absolute position of a motor, in steps
    static const int _positiveEndSwitchPosition; ///< Like the real position of the positive end switch in steps
    static const int _negativeEndSwitchPosition; ///< Like the real position of the negative end switch in steps

    int _targetPosition; ///< Target position in steps
    int _currentPosition; ///< The current position can be set by the user (calibration)

    bool _positiveEndSwitchEnabled;///< Flag whether the positive end switch is being used
    bool _negativeEndSwitchEnabled;///< Flag whether the negative end switch is being used
    bool _enabled;///< Flag wether the motor is enabled. The absolute position will
    ///< not change if this flag is false.

    unsigned int _id;

    /** determines if the motor is considered as "stepping". This is the case
     *  if the target position is not reached, no end switch is reached and 
     *  no error has been detected. In this case the current position is moving
     *  towards the target position. The motor will only move if it is enabled.
     *  In this case also the absolute position is changing and an end switch can
     *  be activated which was inactive before.
     */
    bool isStepping();

    /** The motor is moving if it is stepping (see isStepping() ) and it is
     *  enabled.
     */
    bool isMoving();

    bool isPositiveEndSwitchActive();
    bool isNegativeEndSwitchActive();
  };  

}// namespace mtca4u

#endif //MTCA4U_MOTOR_CONTROLER_DUMMY_H

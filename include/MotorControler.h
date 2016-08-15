#ifndef MTCA4U_MOTOR_CONTROLER_H
#define MTCA4U_MOTOR_CONTROLER_H

#include "MotorReferenceSwitchData.h"
#include "TMC260Words.h"

#define MC_DECLARE_SET_GET_VALUE( NAME, VARIABLE_IN_UNITS )\
  virtual void set ## NAME (unsigned int VARIABLE_IN_UNITS )=0;	\
  virtual unsigned int get ## NAME ()=0

#define MC_DECLARE_SIGNED_SET_GET_VALUE( NAME, VARIABLE_IN_UNITS )\
  virtual void set ## NAME (int VARIABLE_IN_UNITS )=0;	\
  virtual int get ## NAME ()=0

#define MC_DECLARE_SET_GET_TYPED_REGISTER( NAME, VARIABLE_NAME )\
  virtual void set ## NAME ( NAME const & VARIABLE_NAME )=0;\
  virtual NAME get ## NAME ()=0

namespace mtca4u
{
  /**
   * A class to controll stepper motors using the DFMC-MD22 card.
   * This class represents the base functionality of one motor. 
   * 
   * Note: Technically this controler class addresses registers of the
   * TMC 429 motor controler chip (which can handle up to 3 motors) and
   * the TMC 260 motor driver, which is indiviual for each motor.
   * On the software side these implementation details are not important.
   * Think of this class as a virtual controler with integrated driver.
   *
   * This purely virtual class defines the interface.
   * 
   * @author Martin Killenberg <martin.killenberg@desy.de>
   */
  class MotorControler{

  public:
    
    /// Helper struct to hold the readout modes.
    struct DecoderReadoutMode
    {
      static const unsigned int HEIDENHAIN = 0;
      static const unsigned int INCREMENTAL = 1;
    };

    /// Get the ID of the motor controler on the FMC board (0 or 1).
    virtual unsigned int getID()=0;

    virtual int getActualPosition()=0; ///< Get the actual position in steps
    virtual int getActualVelocity()=0; ///< Get the actual velocity in steps per FIXME
    virtual unsigned int getActualAcceleration()=0; ///< Get the actual acceleration in steps per square FIXME
    virtual unsigned int getMicroStepCount()=0; ///< Get the micro step value (which units? what does it mean?)

    virtual DriverStatusData getStatus()=0; ///< Get the status of the motor driver
    virtual unsigned int getDecoderReadoutMode()=0; ///< Get the decoder readout mode
    virtual unsigned int getDecoderPosition()=0; ///< Get the decoder position (which units?)
    
    virtual double setUserSpeedLimit(double stepsPerFIXME)=0;
    virtual double getUserSpeedLimit()=0;
    virtual double getMaxSpeedCapability()=0;

    virtual double setUserCurrentLimit(double currentLimit)=0;
    virtual double getUserCurrentLimit()=0;
    virtual double getMaxCurrentLimit()=0;

    virtual void setActualAcceleration(unsigned int stepsPerSquareFIXME)=0;
    virtual void setMicroStepCount(unsigned int microStepCount)=0;

    virtual void setEnabled(bool enable=true)=0; ///< @deprecated Use setMotorCurrentEnabled instead.
    virtual void setDecoderReadoutMode(unsigned int decoderReadoutMode)=0;

    virtual bool isEnabled()=0; ///< @deprecated Use isMotorCurrentEnabled instead.
    
    virtual bool isMotorMoving()=0; ///< check if the motor is moving.

    virtual void setMotorCurrentEnabled(bool enable = true) = 0;
    virtual bool isMotorCurrentEnabled() = 0;

    virtual void setEndSwitchPowerEnabled(bool enable = true) = 0;
    virtual bool isEndSwitchPowerEnabled() = 0;

    virtual MotorReferenceSwitchData getReferenceSwitchData()=0; ///< Get information about both reference switches of this Motor

    /** Enable or disable the positive reference switch. (true=enabled)*/
    virtual void setPositiveReferenceSwitchEnabled(bool enableStatus)=0;
    /** Enable or disable the negative reference switch. (true=enabled)*/
    virtual void setNegativeReferenceSwitchEnabled(bool enableStatus)=0;

    // via spi to the tmc429 motor controler
    /// Set the actual position counter for a recalibration of the actual position
    virtual void setActualPosition(int steps)=0;
    MC_DECLARE_SIGNED_SET_GET_VALUE( TargetPosition, steps ); ///< Get the target position in steps
    MC_DECLARE_SET_GET_VALUE( MinimumVelocity, stepsPerFIXME );///< Get minimum velocity in FIXME
    MC_DECLARE_SET_GET_VALUE( MaximumVelocity, stepsPerFIXME );///< Get maximum velocity in FIXME
    MC_DECLARE_SIGNED_SET_GET_VALUE( TargetVelocity, stepsPerFIXME );///< Get target velocity in FIXME
    MC_DECLARE_SET_GET_VALUE( MaximumAcceleration, stepsPerSquareFIXME );///< Get the maximim acceleration in FIXME
    MC_DECLARE_SET_GET_VALUE( PositionTolerance, steps );///< Get the position tolerance in steps
    MC_DECLARE_SET_GET_VALUE( PositionLatched, steps );///< Get the latched position in steps

     /** Extracts the correct bit from the controler status word. */
     virtual bool targetPositionReached()=0;

     /** Returns the ReferenceSwitchBit for this motor as defined in section 10.1 
      *  of the TMC429 data sheet.
      *  This function the extracts the correct bit from the controler status word
      *  and stores it as the least significant bit. */
     virtual unsigned int getReferenceSwitchBit()=0;

     virtual ~MotorControler(){}
   };

}// namespace mtca4u

#endif // MTCA4U_MOTOR_CONTROLER_H

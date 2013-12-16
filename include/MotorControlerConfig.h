#ifndef MTCA4U_MOTOR_CONTROLER_CONFIG_H
#define MTCA4U_MOTOR_CONTROLER_CONFIG_H

#include "TMC429Words.h"
#include "TMC260Words.h"

namespace mtca4u
{
  /** This configuration contains start values for all data words that 
   *  can be written to the MotorControler class. Please note that the IDX field
   *  of the TMC429 words are ignored. It is replaced by the motor ID when the
   *  settings are applied, so settings can be reused for several motors.
   */
  struct MotorControlerConfig{
    AccelerationThresholdData accelerationThresholdData;
    unsigned int actualPosition;
    ChopperControlData chopperControlData;
    CoolStepControlData coolStepControlData;
    uint32_t decoderReadoutMode;
    DividersAndMicroStepResolutionData dividersAndMicroStepResolutionData;
    DriverConfigData driverConfigData;
    DriverControlData driverControlData;
    bool enabled;
    InterruptData interruptData;
    uint32_t maximumAccelleration;
    uint32_t maximumVelocity;
    unsigned int microStepCount;
    uint32_t minimumVelocity;
    unsigned int positionTolerance;
    ProportionalityFactorData proportionalityFactorData;
    ReferenceConfigAndRampModeData referenceConfigAndRampModeData;
    StallGuardControlData stallGuardControlData;
    unsigned int targetPosition; //want it? need it in the config?
    unsigned int targetVelocity; //want it? need it in the config?

    /** The constructor sets reasonable default values to all data words.
     */
    MotorControlerConfig();

    bool operator==(MotorControlerConfig const & right) const;
    bool operator!=(MotorControlerConfig const & right) const;
 };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_CONTROLER_CONFIG_H

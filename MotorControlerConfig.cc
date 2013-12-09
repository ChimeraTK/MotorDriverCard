#include "MotorControlerConfig.h"
#include "MotorControler.h"

unsigned int const CHOPPER_CONTROL_DEFAULT = 0x10765;
unsigned int const DIVIDIERS_AND_MICRO_STEP_RESOLUTION_DEFAULT = 0x003704;
unsigned int const DRIVER_CONFIG_DEFALUT = 0x00040;
unsigned int const MAXIMUM_ACCELERATION_DEFAULT = 0x14;
unsigned int const MAXIMUM_VELOCITY_DEFAULT = 0x64;
unsigned int const MINIMUM_VELOCITY_DEFAULT = 0xA;
unsigned int const PROPORTIONALITY_FACTORS_DEFAULT = 0xC50A;
unsigned int const REFERENCE_CONFIG_AND_RAMP_MODE_DEFAULT = 0x300;

namespace mtca4u
{
  MotorControlerConfig::MotorControlerConfig()
    : accelerationThresholdData(0), actualPosition(0),
      chopperControlData(CHOPPER_CONTROL_DEFAULT),
      coolStepControlData(0),
      decoderReadoutMode(MotorControler::DecoderReadoutMode::INCREMENTAL),
      dividersAndMicroStepResolutionData(DIVIDIERS_AND_MICRO_STEP_RESOLUTION_DEFAULT),
      driverConfigData(DRIVER_CONFIG_DEFALUT),
      driverControlData(0),
      enabled(true),
      interruptData(0),
      maximumAccelleration(MAXIMUM_ACCELERATION_DEFAULT),
      maximumVelocity(MAXIMUM_VELOCITY_DEFAULT),
      microStepCount(0),
      minimumVelocity(MINIMUM_VELOCITY_DEFAULT),
      positionTolerance(0),
      proportionalityFactorData(PROPORTIONALITY_FACTORS_DEFAULT),
      referenceConfigAndRampModeData(REFERENCE_CONFIG_AND_RAMP_MODE_DEFAULT),
      stallGuardControlData(0),
      targetPosition(0),
    targetVelocity(0)
  {}
}

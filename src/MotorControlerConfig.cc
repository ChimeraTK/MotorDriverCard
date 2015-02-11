#include "MotorControlerConfig.h"
#include "MotorControler.h"
#include "MotorControlerConfigDefaults.h"

namespace mtca4u
{
  MotorControlerConfig::MotorControlerConfig()
    : accelerationThresholdData(0), actualPosition(0),
      chopperControlData(CHOPPER_CONTROL_DEFAULT),
      coolStepControlData(COOL_STEP_CONTROL_DEFAULT),
      decoderReadoutMode(MotorControler::DecoderReadoutMode::INCREMENTAL),
      dividersAndMicroStepResolutionData(DIVIDIERS_AND_MICRO_STEP_RESOLUTION_DEFAULT),
      driverConfigData(DRIVER_CONFIG_DEFAULT),
      driverControlData(DRIVER_CONTROL_DEFAULT),
      enabled(false),
      interruptData(0),
      maximumAcceleration(MAXIMUM_ACCELERATION_DEFAULT),
      maximumVelocity(MAXIMUM_VELOCITY_DEFAULT),
      microStepCount(0),
      minimumVelocity(MINIMUM_VELOCITY_DEFAULT),
      positionTolerance(0),
      proportionalityFactorData(PROPORTIONALITY_FACTORS_DEFAULT),
      referenceConfigAndRampModeData(REFERENCE_CONFIG_AND_RAMP_MODE_DEFAULT),
      stallGuardControlData(STALL_GUARD_CONTROL_DEFAULT),
      targetPosition(0),
      targetVelocity(0),
      driverSpiWaitingTime(DRIVER_SPI_WAITING_TIME_DEFAULT)
  {}

  bool MotorControlerConfig::operator==(MotorControlerConfig const & right) const{
    return ( (accelerationThresholdData 	  == right.accelerationThresholdData) &&
	     (actualPosition			  == right.actualPosition) &&
	     (chopperControlData		  == right.chopperControlData) &&
	     (coolStepControlData		  == right.coolStepControlData) &&
	     (decoderReadoutMode		  == right.decoderReadoutMode) &&
	     (dividersAndMicroStepResolutionData  == right.dividersAndMicroStepResolutionData) &&
	     (driverConfigData			  == right.driverConfigData) &&
	     (driverControlData			  == right.driverControlData) &&
	     (enabled				  == right.enabled) &&
	     (interruptData			  == right.interruptData) &&
	     (maximumAcceleration		  == right.maximumAcceleration) &&
	     (maximumVelocity			  == right.maximumVelocity) &&
	     (microStepCount			  == right.microStepCount) &&
	     (minimumVelocity			  == right.minimumVelocity) &&
	     (positionTolerance			  == right.positionTolerance) &&
	     (proportionalityFactorData		  == right.proportionalityFactorData) &&
	     (referenceConfigAndRampModeData	  == right.referenceConfigAndRampModeData) &&
	     (stallGuardControlData		  == right.stallGuardControlData) &&
	     (targetPosition			  == right.targetPosition) &&
	     (targetVelocity                      == right.targetVelocity) &&
	     (driverSpiWaitingTime                == right.driverSpiWaitingTime));              
  }

  bool  MotorControlerConfig::operator!=(MotorControlerConfig const & right) const{
    return !(*this == right);
  }
}

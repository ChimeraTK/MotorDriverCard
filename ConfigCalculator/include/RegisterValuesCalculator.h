#ifndef MTCA4U_TMC429_REGISTER_VALUES_CALCULATOR_H
#define MTCA4U_TMC429_REGISTER_VALUES_CALCULATOR_H

#include "ParametersCalculator.h"
#include <stdint.h>

class RegisterValuesCalculator{
 public:
  enum EndSwitchConfig{ USE_BOTH = 0x000,
			IGNORE_NEGATIVE = 0x100,
			IGNORE_POSITIVE = 0x200,
			IGNORE_BOTH = 0x300 };

  /** The (hex) values which have to be written to the registers.
   */
  struct RegisterValues{
    uint32_t driverControlData;
    uint32_t stallGuardControlData;
    uint32_t referenceConfigAndRampModeData;
    uint32_t proportionalityFactorData;
    uint32_t dividersAndMicroStepResolutionData;
    uint32_t maximumVelocity;
    uint32_t maximumAcceleration;

    /** Convenience constructor to set all parameters
     */
    RegisterValues( uint32_t driverControlData_,
		    uint32_t stallGuardControlData_,
		    uint32_t referenceConfigAndRampModeData_,
		    uint32_t proportionalityFactorData_,
		    uint32_t dividersAndMicroStepResolutionData_,
		    uint32_t maximumVelocity_,
		    uint32_t maximumAcceleration_)
    : driverControlData(driverControlData_),
      stallGuardControlData(stallGuardControlData_),
      referenceConfigAndRampModeData(referenceConfigAndRampModeData_),
      proportionalityFactorData(proportionalityFactorData_),
      dividersAndMicroStepResolutionData(dividersAndMicroStepResolutionData_),
      maximumVelocity(maximumVelocity_),
      maximumAcceleration(maximumAcceleration_)
    {}    
  };

  static RegisterValues calculateRegisterValues( 
    ParametersCalculator::TMC429Parameters tmc429Parameters,
    EndSwitchConfig endSwitchConfig);
};

#endif // MTCA4U_TMC429_REGISTER_VALUES_CALCULATOR_H

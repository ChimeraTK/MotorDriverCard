#ifndef MTCA4U_TMC429_CONFIG_CALCULATOR_H
#define MTCA4U_TMC429_CONFIG_CALCULATOR_H

#include "ParametersCalculator.h"
#include "MotorControlerConfig.h"

#include <stdint.h>

class ConfigCalculator{
 public:
  enum EndSwitchConfig{ USE_BOTH = 0x000,
			IGNORE_NEGATIVE = 0x100,
			IGNORE_POSITIVE = 0x200,
			IGNORE_BOTH = 0x300 };

  static mtca4u::MotorControlerConfig calculateConfig( 
    ParametersCalculator::TMC429Parameters const & tmc429Parameters,
    EndSwitchConfig const & endSwitchConfig);

 private:
  // throws std::invalid_argument if the enum is not one of the 
  // pre-defined values
  static void checkEndSwitchConfig(EndSwitchConfig const & endSwitchConfig);
};

#endif // MTCA4U_TMC429_CONFIG_CALCULATOR_H

#ifndef MTCA4U_TMC429_TMC260_CONFIG_CALCULATOR_H
#define MTCA4U_TMC429_TMC260_CONFIG_CALCULATOR_H

#include "MotorControlerConfig.h"
#include "ParametersCalculator.h"

#include <stdint.h>

/** This calls privides a function to create an mtca4u::MotorControlerConfig
 *  from the ConfigCalculator::ChipParameters (parameters of the
 *  TMC429 and TMC260 chips).
 */
class ConfigCalculator {
 public:
  /** The possible end switch configurations.
   *  The hex values are the bit sequences for the
   *  TMC429 referenceConfigAndRampModeData register with
   *  ramp mode = 0.
   */
  enum EndSwitchConfig { USE_BOTH = 0x000, IGNORE_NEGATIVE = 0x100, IGNORE_POSITIVE = 0x200, IGNORE_BOTH = 0x300 };

  /** Calculate a MotorContolerConfig from the chip parameters
   *  and the end switch configuration.
   */
  static mtca4u::MotorControlerConfig calculateConfig(
      ParametersCalculator::ChipParameters const& chipParameters, EndSwitchConfig const& endSwitchConfig);

 private:
  /** throws std::invalid_argument if the enum is not one of the
   *  pre-defined values
   */
  static void checkEndSwitchConfig(EndSwitchConfig const& endSwitchConfig);
};

#endif // MTCA4U_TMC429_TMC260_CONFIG_CALCULATOR_H

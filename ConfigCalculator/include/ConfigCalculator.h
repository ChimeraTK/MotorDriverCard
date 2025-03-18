// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "MotorControlerConfig.h"
#include "ParametersCalculator.h"

#include <stdint.h>

/** This calls privides a function to create an ChimeraTK::MotorControlerConfig
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
  static ChimeraTK::MotorControlerConfig calculateConfig(
      ParametersCalculator::ChipParameters const& chipParameters, EndSwitchConfig const& endSwitchConfig);

 private:
  /** throws std::invalid_argument if the enum is not one of the
   *  pre-defined values
   */
  static void checkEndSwitchConfig(EndSwitchConfig const& endSwitchConfig);
};

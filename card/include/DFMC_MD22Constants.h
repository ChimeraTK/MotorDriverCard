// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "TMC429Constants.h"

#include <string>
#include <string_view>

namespace ChimeraTK::MotorDriver::dfmc_md22 {

  int32_t const MINIMAL_FIRMWARE_VERSION = 0x02020000;
  int32_t const PROJECT_ID = 0x0;
  int32_t const PROJECT_MAGIC_NUMBER = 0x50524F4A;

  /// @todo FIXME This has to be WORD_PROJ_VERSION in the project, but how to
  /// identify the project?
  inline std::string const PROJECT_MAGIC_ADDRESS_STRING = "WORD_PROJ_MAGIC_NUM";
  inline std::string const PROJECT_ID_ADDRESS_STRING = "WORD_PROJ_ID";
  inline std::string const PROJECT_VERSION_ADDRESS_STRING = "WORD_PROJ_VERSION";
  inline std::string const PROJECT_RESET_ADDRESS_STRING = "WORD_PROJ_RESET";
  inline std::string const PROJECT_NEXT_ADDRESS_STRING = "WORD_PROJ_NEXT";

  inline std::string const CONTROLLER_SPI_WRITE_ADDRESS_STRING = "WORD_CTRL_SPI_WRITE";
  inline std::string const CONTROLLER_SPI_READBACK_ADDRESS_STRING = "WORD_CTRL_SPI_READBACK";
  inline std::string const CONTROLLER_SPI_SYNC_ADDRESS_STRING = "WORD_CTRL_SPI_SYNC";
  inline std::string const CONTROLLER_STATUS_BITS_ADDRESS_STRING = "WORD_CTRL_STATUS_BITS";

  int32_t const SPI_SYNC_OK = 0;
  int32_t const SPI_SYNC_REQUESTED = 0xFF;
  int32_t const SPI_SYNC_ERROR = 0xAA;

  inline constexpr uint32_t N_MOTORS_MAX = 2;

  inline std::string const MOTOR_REGISTER_PREFIX = "WORD_M";

  inline std::string const SPI_WRITE_SUFFIX = "SPI_WRITE";
  inline std::string const SPI_SYNC_SUFFIX = "SPI_SYNC";
  inline std::string const ACTUAL_POSITION_SUFFIX = "ACTUAL_POS";
  inline std::string const ACTUAL_VELOCITY_SUFFIX = "V_ACTUAL";
  inline std::string const ACTUAL_ACCELERATION_SUFFIX = "ACT_ACCEL";
  inline std::string const MICRO_STEP_COUNT_SUFFIX = "MSTEP_VAL";
  inline std::string const COOL_STEP_VALUE_SUFFIX = "CoolStep_VAL";
  inline std::string const STALL_GUARD_VALUE_SUFFIX = "SGUARD_VAL";
  inline std::string const STATUS_SUFFIX = "STATUS";
  inline std::string const MOTOR_CURRENT_ENABLE_SUFFIX = "ENABLE";
  inline std::string const ENDSWITCH_ENABLE_SUFFIX = "VOLTAGE_EN";
  inline std::string const DECODER_READOUT_MODE_SUFFIX = "DEK_MUX";
  inline std::string const DECODER_POSITION_SUFFIX = "DEK_POS";
  inline std::string const CALIBRATION_TIME_SUFFIX = "CAL_TIME";          // Added as support for newer FW > r2591
  inline std::string const END_SWITCH_POSITIVE_SUFFIX = "CAL_END_SW_POS"; // Added as support for newer FW > r2591
  inline std::string const END_SWITCH_NEGATIVE_SUFFIX = "CAL_END_SW_NEG"; // Added as support for newer FW > r2591

  inline constexpr uint32_t INVALID_SPI_READBACK_VALUE = 0xFFFFFFFF;

  /// Creates a string dependent on the motor ID and register name suffix.
  /// The result is WORD_M1_SUFFIX for motor ID = 0 and suffix = SUFFIX.
  /// FIXME: get the 'off by one' consistent
  inline std::string createMotorRegisterName(unsigned int motorID, std::string_view registerNameSuffix) {
    std::string registerName{MOTOR_REGISTER_PREFIX};
    registerName += std::to_string(motorID + 1);
    registerName += '_';
    registerName.append(registerNameSuffix);
    return registerName;
  }

} // namespace ChimeraTK::MotorDriver::dfmc_md22

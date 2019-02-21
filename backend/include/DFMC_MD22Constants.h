#ifndef MTCA4U_DMFC_MD22_CONSTANTS_H
#define MTCA4U_DMFC_MD22_CONSTANTS_H

#include <boost/lexical_cast.hpp>

#include "TMC429Constants.h"

namespace mtca4u {

namespace dfmc_md22 {

int32_t const MINIMAL_FIRMWARE_VERSION = 0x02020000;
int32_t const PROJECT_ID = 0x0;
int32_t const PROJECT_MAGIC_NUMBER = 0x50524F4A;

/// @todo FIXME This has to be WORD_PROJ_VERSION in the project, but how to
/// identify the project?
std::string const PROJECT_MAGIC_ADDRESS_STRING = "WORD_PROJ_MAGIC_NUM";
std::string const PROJECT_ID_ADDRESS_STRING = "WORD_PROJ_ID";
std::string const PROJECT_VERSION_ADDRESS_STRING = "WORD_PROJ_VERSION";
std::string const PROJECT_RESET_ADDRESS_STRING = "WORD_PROJ_RESET";
std::string const PROJECT_NEXT_ADDRESS_STRING = "WORD_PROJ_NEXT";

std::string const CONTROLER_SPI_WRITE_ADDRESS_STRING = "WORD_CTRL_SPI_WRITE";
std::string const CONTROLER_SPI_READBACK_ADDRESS_STRING =
    "WORD_CTRL_SPI_READBACK";
std::string const CONTROLER_SPI_SYNC_ADDRESS_STRING = "WORD_CTRL_SPI_SYNC";
std::string const CONTROLER_STATUS_BITS_ADDRESS_STRING =
    "WORD_CTRL_STATUS_BITS";
std::string const CALIBRATION_TIME =
    "WORD_PROJ_USER"; // TODO: Needs to be keep for backward compatibility?

int32_t const SPI_SYNC_OK = 0;
int32_t const SPI_SYNC_REQUESTED = 0xFF;
int32_t const SPI_SYNC_ERROR = 0xAA;

uint32_t const N_MOTORS_MAX = 2;

std::string const MOTOR_REGISTER_PREFIX = "WORD_M";

std::string const SPI_WRITE_SUFFIX = "SPI_WRITE";
std::string const SPI_SYNC_SUFFIX = "SPI_SYNC";
std::string const ACTUAL_POSITION_SUFFIX = "ACTUAL_POS";
std::string const ACTUAL_VELOCITY_SUFFIX = "V_ACTUAL";
std::string const ACTUAL_ACCELETATION_SUFFIX = "ACT_ACCEL";
std::string const MICRO_STEP_COUNT_SUFFIX = "MSTEP_VAL";
std::string const COOL_STEP_VALUE_SUFFIX = "CoolStep_VAL";
std::string const STALL_GUARD_VALUE_SUFFIX = "SGUARD_VAL";
std::string const STATUS_SUFFIX = "STATUS";
std::string const MOTOR_CURRENT_ENABLE_SUFFIX = "ENABLE";
std::string const ENDSWITCH_ENABLE_SUFFIX = "VOLTAGE_EN";
std::string const DECODER_READOUT_MODE_SUFFIX = "DEK_MUX";
std::string const DECODER_POSITION_SUFFIX = "DEK_POS";
std::string const CALIBRATION_TIME_SUFFIX =
    "CAL_TIME"; // Added as support for newer FW > r2261

uint32_t const INVALID_SPI_READBACK_VALUE = 0xFFFFFFFF;

/// Creates a string dependent on the motor ID and register name suffix.
/// The result is WORD_M1_SUFFIX for motor ID = 0 and suffix = SUFFIX.
/// FIXME: get the 'off by one' consistent
inline std::string createMotorRegisterName(unsigned int motorID,
                                           std::string registerNameSuffix) {
  return std::string(MOTOR_REGISTER_PREFIX) +
         boost::lexical_cast<std::string>(motorID + 1) + "_" +
         registerNameSuffix;
}

} // namespace dfmc_md22

} // namespace mtca4u

#endif // MTCA4U_DMFC_MD22_CONSTANTS_H

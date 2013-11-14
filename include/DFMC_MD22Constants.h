#ifndef MTCA4U_DMFC_MD22_CONSTANTS_H
#define MTCA4U_DMFC_MD22_CONSTANTS_H

#include <boost/lexical_cast.hpp>

#include "TMC429Constants.h"

namespace mtca4u{

namespace dfmc_md22{

  std::string const CONTROLER_SPI_WRITE_ADDRESS_STRING = "WORD_SPI_CTRL";
  std::string const CONTROLER_SPI_READBACK_ADDRESS_STRING = "WORD_SPI_CTRL_B";

  uint32_t const N_MOTORS_MAX = 2;

  std::string const MOTOR_REGISTER_PREFIX = "WORD_M";
  
  std::string const ACTUAL_POSITION_SUFFIX = "ACTUAL_POS";
  std::string const ACTUAL_VELOCITY_SUFFIX = "V_ACTUAL";
  std::string const ACTUAL_ACCELETATION_SUFFIX = "ACT_ACCEL";
  std::string const MICRO_STEP_COUNT_SUFFIX = "MSTEP_VAL";

  /// Creates a string dependent on the motor ID and register name suffix.
  /// The result is WORD_M1_SUFFIX for motor ID = 0 and suffix = SUFFIX.
  /// FIXME: get the 'off by one' consistent
  inline std::string createMotorRegisterName( unsigned int motorID, std::string registerNameSuffix ){
    return std::string( MOTOR_REGISTER_PREFIX)
      + boost::lexical_cast<std::string>(motorID + 1) + "_"
      + registerNameSuffix;
  }

  
}// namespace dfmc_md22

}//namespace mtac4u


#endif// MTCA4U_DMFC_MD22_CONSTANTS_H

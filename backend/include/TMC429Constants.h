#ifndef CHIMERATK_TMC429_CONSTANTS_H
#define CHIMERATK_TMC429_CONSTANTS_H

#include <stdint.h>

namespace mtca4u { namespace tmc429 {

  uint32_t const SMDA_COMMON = 0x3;
  uint32_t const JDX_DATAGRAM_LOW_WORD = 0x0;
  uint32_t const JDX_DATAGRAM_HIGH_WORD = 0x1;
  uint32_t const JDX_COVER_POSITION_AND_LENGTH = 0x2;
  uint32_t const JDX_COVER_DATAGRAM = 0x3;
  uint32_t const JDX_INTERFACE_CONFIGURATION = 0x4;
  uint32_t const JDX_POSITION_COMPARE = 0x5;
  uint32_t const JDX_POSITION_COMPARE_INTERRUPT = 0x6;
  // = 0x7 not defined
  uint32_t const JDX_POWER_DOWN = 0x8;
  uint32_t const JDX_CHIP_VERSION = 0x9;
  // = 0xA through = 0xD not defined;
  uint32_t const JDX_REFERENCE_SWITCH = 0xE;
  uint32_t const JDX_STEPPER_MOTOR_GLOBAL_PARAMETERS = 0xF;

  uint32_t const IDX_TARGET_POSITION = 0x0;
  uint32_t const IDX_ACTUAL_POSITION = 0x1;
  uint32_t const IDX_MINIMUM_VELOCITY = 0x2;
  uint32_t const IDX_MAXIMUM_VELOCITY = 0x3;
  uint32_t const IDX_TARGET_VELOCITY = 0x4;
  uint32_t const IDX_ACTUAL_VELOCITY = 0x5;
  uint32_t const IDX_MAXIMUM_ACCELERATION = 0x6;
  uint32_t const IDX_ACTUAL_ACCELERATION = 0x7;
  uint32_t const IDX_ACCELERATION_THRESHOLD = 0x8;
  uint32_t const IDX_PROPORTIONALITY_FACTORS = 0x9;
  uint32_t const IDX_REFERENCE_CONFIG_AND_RAMP_MODE = 0xA;
  uint32_t const IDX_INTERRUPT_MASK_AND_FLAGS = 0xB;
  uint32_t const IDX_DIVIDERS_AND_MICRO_STEP_RESOLUTION = 0xC;
  uint32_t const IDX_DELTA_X_REFERENCE_TOLERANCE = 0xD;
  uint32_t const IDX_POSITION_LATCHED = 0xE;
  uint32_t const IDX_MICRO_STEP_COUNT = 0xF;

  uint32_t const RW_WRITE = 0;
  uint32_t const RW_READ = 1;

  inline uint32_t spiAddressFromSmdaIdxJdx(uint32_t smda, uint32_t idxJdx) {
    return ((smda << 4) + idxJdx);
  }
}} // namespace mtca4u::tmc429

#endif // CHIMERATK_TMC429_CONSTANTS_H

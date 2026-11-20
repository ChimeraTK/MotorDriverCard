// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <stdint.h>

namespace ChimeraTK::MotorDriver::tmc429 {

  inline constexpr uint32_t SMDA_COMMON = 0x3;
  inline constexpr uint32_t JDX_DATAGRAM_LOW_WORD = 0x0;
  inline constexpr uint32_t JDX_DATAGRAM_HIGH_WORD = 0x1;
  inline constexpr uint32_t JDX_COVER_POSITION_AND_LENGTH = 0x2;
  inline constexpr uint32_t JDX_COVER_DATAGRAM = 0x3;
  inline constexpr uint32_t JDX_INTERFACE_CONFIGURATION = 0x4;
  inline constexpr uint32_t JDX_POSITION_COMPARE = 0x5;
  inline constexpr uint32_t JDX_POSITION_COMPARE_INTERRUPT = 0x6;
  // = 0x7 not defined
  inline constexpr uint32_t JDX_POWER_DOWN = 0x8;
  inline constexpr uint32_t JDX_CHIP_VERSION = 0x9;
  // = 0xA through = 0xD not defined;
  inline constexpr uint32_t JDX_REFERENCE_SWITCH = 0xE;
  inline constexpr uint32_t JDX_STEPPER_MOTOR_GLOBAL_PARAMETERS = 0xF;

  inline constexpr uint32_t IDX_TARGET_POSITION = 0x0;
  inline constexpr uint32_t IDX_ACTUAL_POSITION = 0x1;
  inline constexpr uint32_t IDX_MINIMUM_VELOCITY = 0x2;
  inline constexpr uint32_t IDX_MAXIMUM_VELOCITY = 0x3;
  inline constexpr uint32_t IDX_TARGET_VELOCITY = 0x4;
  inline constexpr uint32_t IDX_ACTUAL_VELOCITY = 0x5;
  inline constexpr uint32_t IDX_MAXIMUM_ACCELERATION = 0x6;
  inline constexpr uint32_t IDX_ACTUAL_ACCELERATION = 0x7;
  inline constexpr uint32_t IDX_ACCELERATION_THRESHOLD = 0x8;
  inline constexpr uint32_t IDX_PROPORTIONALITY_FACTORS = 0x9;
  inline constexpr uint32_t IDX_REFERENCE_CONFIG_AND_RAMP_MODE = 0xA;
  inline constexpr uint32_t IDX_INTERRUPT_MASK_AND_FLAGS = 0xB;
  inline constexpr uint32_t IDX_DIVIDERS_AND_MICRO_STEP_RESOLUTION = 0xC;
  inline constexpr uint32_t IDX_DELTA_X_REFERENCE_TOLERANCE = 0xD;
  inline constexpr uint32_t IDX_POSITION_LATCHED = 0xE;
  inline constexpr uint32_t IDX_MICRO_STEP_COUNT = 0xF;

  inline constexpr uint32_t RW_WRITE = 0;
  inline constexpr uint32_t RW_READ = 1;

  inline uint32_t spiAddressFromSmdaIdxJdx(uint32_t smda, uint32_t idxJdx) {
    return ((smda << 4U) + idxJdx);
  }
} // namespace ChimeraTK::MotorDriver::tmc429

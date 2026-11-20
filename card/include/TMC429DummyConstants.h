// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <stdint.h>

namespace ChimeraTK::MotorDriver::tmc429 {

  inline constexpr uint32_t SIZE_OF_SPI_ADDRESS_SPACE = 0x40;
  inline constexpr uint32_t CONTROLLER_CHIP_VERSION = 0x429101;
  inline constexpr uint32_t SPI_DATA_MASK = 0xFFFFFF;

  inline constexpr unsigned int DEFAULT_DUMMY_SPI_DELAY = 32; ///< 32 microseconds
} // namespace ChimeraTK::MotorDriver::tmc429

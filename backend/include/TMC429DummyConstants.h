// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <stdint.h>

namespace ChimeraTK::tmc429 {

  uint32_t const SIZE_OF_SPI_ADDRESS_SPACE = 0x40;
  uint32_t const CONTROLER_CHIP_VERSION = 0x429101;
  uint32_t const SPI_DATA_MASK = 0xFFFFFF;

  unsigned int const DEFAULT_DUMMY_SPI_DELAY = 32; ///< 32 microseconds
} // namespace ChimeraTK::tmc429

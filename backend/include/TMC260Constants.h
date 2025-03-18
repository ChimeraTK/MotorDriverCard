// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <stdint.h>

namespace ChimeraTK::tmc260 {

  uint32_t const ADDRESS_DRIVER_CONTROL = 0x0;
  uint32_t const ADDRESS_CHOPPER_CONFIG = 0x4;
  uint32_t const ADDRESS_COOL_STEP_CONFIG = 0x5;
  uint32_t const ADDRESS_STALL_GUARD_CONFIG = 0x6;
  uint32_t const ADDRESS_DRIVER_CONFIG = 0x7;

} // namespace ChimeraTK::tmc260

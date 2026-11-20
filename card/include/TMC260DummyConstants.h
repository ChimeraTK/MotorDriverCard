// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "MultiVariableWord.h"

#include <stdint.h>

namespace ChimeraTK::MotorDriver::tmc260 {

  uint32_t const SIZE_OF_SPI_ADDRESS_SPACE = 0x8;

  unsigned int const DEFAULT_DUMMY_SPI_DELAY = 20; ///< 20 microseconds

  inline unsigned int testWordFromSpiAddress(unsigned int spiAddress, unsigned int motorID) {
    return spiAddress * spiAddress + 7 + motorID;
  }

  inline unsigned int spiAddressFromDataWord(unsigned int word) {
    // Sorry for this mess, but I don't see any other chance to implement
    // "if bits 18 and 19 are zero then these bits are the address, else
    //  bits 17 to 19 are the address" than hard-coding it.
    if((word & outputMask(18, 19)) == 0) {
      return 0;
    }
    else {
      return ((word & outputMask(17, 19)) >> 17U);
    }
  }

  inline unsigned int dataMaskFromSpiAddress(unsigned int spiAddress) {
    if(spiAddress == 0) {
      return outputMask(0, 17);
    }
    else {
      return outputMask(0, 16);
    }
  }

} // namespace ChimeraTK::MotorDriver::tmc260

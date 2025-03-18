// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

namespace ChimeraTK::tmc429 {

  inline unsigned int testWordFromSpiAddress(unsigned int smda, unsigned int idx_jdx) {
    unsigned int spiAddress = (smda << 4) | idx_jdx;
    // the test word as defined in the Dummy (independent implementation)
    return spiAddress * spiAddress + 13;
  }

} // namespace ChimeraTK::tmc429

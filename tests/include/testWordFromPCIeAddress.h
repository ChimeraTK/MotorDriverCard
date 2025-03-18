// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

namespace ChimeraTK {

  /// The project only lives on one bar, so the address in the bar is enough
  unsigned int testWordFromPCIeAddress(unsigned int address) {
    // the test word as defined in the Dummy (independent implementation)
    return 3 * address * address + 17;
  }

} // namespace ChimeraTK

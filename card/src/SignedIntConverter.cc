// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SignedIntConverter.h"

namespace ChimeraTK::MotorDriver {

  /// Fixme: is this safe in case someone puts nBits > 31?
  SignedIntConverter::SignedIntConverter(unsigned int nBits)
  : _bitMask((1U << nBits) - 1U), _negativeBit(1U << (nBits - 1U)) {}

  int32_t SignedIntConverter::customToThirtyTwo(int32_t const& customInt) const {
    auto const custom = static_cast<uint32_t>(customInt);
    if((custom & _negativeBit) != 0U) {
      // fill in the leading bits up to 32 bits
      return static_cast<int32_t>(custom | ~_bitMask);
    }
    return customInt;
  }

  int32_t SignedIntConverter::thirtyTwoToCustom(int32_t const& thirtyTwoBitInt) const {
    // just crop the 32 bit int. no range check
    return static_cast<int32_t>(static_cast<uint32_t>(thirtyTwoBitInt) & _bitMask);
  }

} // namespace ChimeraTK::MotorDriver

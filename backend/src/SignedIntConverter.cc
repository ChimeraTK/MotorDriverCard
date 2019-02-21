#include "SignedIntConverter.h"

namespace mtca4u {

/// Fixme: is this safe in case someone puts nBits > 31?
SignedIntConverter::SignedIntConverter(unsigned int nBits)
    : _bitMask((1 << nBits) - 1), _negativeBit(1 << (nBits - 1)) {}

int32_t SignedIntConverter::customToThirtyTwo(int32_t const &customInt) {
  if (customInt & _negativeBit) {
    // fill in the leading bits up to 32 bits
    return customInt | ~_bitMask;
  } else {
    return customInt;
  }
}

int32_t SignedIntConverter::thirtyTwoToCustom(int32_t const &thirtyTwoBitInt) {
  // just crop the 32 bit int. no range check
  return thirtyTwoBitInt & _bitMask;
}

} // namespace mtca4u

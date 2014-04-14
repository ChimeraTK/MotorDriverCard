#ifndef MTCA4U_SIGNED_INT_H
#define MTCA4U_SIGNED_INT_H

#include <boost/cstdint.hpp>

namespace mtca4u{

  /** Convert signed int of arbitraty length (< 32 bits) to 
   *   and from 32 bits on the user side.
   */
  class SignedIntConverter{
  public:
    /** Like a decorator the SignedInt construcotr takes a reference to an 
     *	accessor object, plus the number of bits the integer is representing.
     */
    SignedIntConverter( unsigned int nBits );

    /// Although contained in a 32 bit variable, only nBits are set in the custom int.
    /// Fills in the leading 'Fs' for negative values. 
    int32_t customToThirtyTwo(int32_t const & customInt);

    /// Although the return value is 32 bit, it is truncated to nBits.
    /// No range check is performed, the bits are just cropped. You might 
    /// lose data!
    int32_t thirtyTwoToCustom(int32_t const & thirtyTwoBitInt);

  private: 
    unsigned int _nBits; ///< the number of bits of the signed int
    int32_t _bitMask; ///< The mask which bits are present in the signed int
    int32_t _negativeBit; ///< The most significant bit in the custom int as a mask.

  };

} //namespace mtca4u

#endif// MTCA4U_SIGNED_INT_H

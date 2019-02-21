#ifndef MTCA4U_TEST_WORD_FROM_PCIE_ADDRESS_H
#define MTCA4U_TEST_WORD_FROM_PCIE_ADDRESS_H

namespace mtca4u {

/// The project only lives on one bar, so the address in the bar is enough
unsigned int testWordFromPCIeAddress(unsigned int address) {
  // the test word as defined in the Dummy (independent implementation)
  return 3 * address * address + 17;
}

} // namespace mtca4u

#endif // MTCA4U_TEST_WORD_FROM_PCIE_ADDRESS_H

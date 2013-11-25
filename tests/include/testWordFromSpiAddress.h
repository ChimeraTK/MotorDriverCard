#ifndef MTCA4U_TEST_WORD_FROM_SPI_ADDRESS_H
#define MTCA4U_TEST_WORD_FROM_SPI_ADDRESS_H

namespace mtca4u{

namespace tmc429{

unsigned int testWordFromSpiAddress(unsigned int smda, unsigned int idx_jdx){
  unsigned int spiAddress = (smda << 4) | idx_jdx;
  // the test word as defined in the Dummy (independent implementation)
  return spiAddress*spiAddress+13;
}

}//namespace tmc429

}// namespace mtca4u

#endif //MTCA4U_TEST_WORD_FROM_SPI_ADDRESS_H

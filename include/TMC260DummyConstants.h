#ifndef MTCA4U_TMC260_DUMMY_CONSTANTS_H
#define MTCA4U_TMC260_DUMMY_CONSTANTS_H

#include <stdint.h>

namespace mtca4u{

namespace tmc260{

  uint32_t const SIZE_OF_SPI_ADDRESS_SPACE = 0x8;
  
  inline unsigned int testWordFromSpiAddress(unsigned int spiAddress, unsigned int motorID){
    return spiAddress*spiAddress + 7 + motorID;
  }

}// namespace tmc260

}// namespace mtca4u

#endif// MTCA4U_TMC260_DUMMY_CONSTANTS_H

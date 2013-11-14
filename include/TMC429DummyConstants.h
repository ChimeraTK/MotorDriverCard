#ifndef MTCA4U_TMC429_DUMMY_CONSTANTS_H
#define MTCA4U_TMC429_DUMMY_CONSTANTS_H

#include <stdint.h>

namespace mtca4u{

namespace tmc429{

  uint32_t const SIZE_OF_SPI_ADDRESS_SPACE = 0x40;
  uint32_t const CONTROLER_CHIP_VERSION = 0x429101;
  uint32_t const SPI_DATA_MASK = 0xFFFFFF;

}// namespace tmc429

}// namespace mtca4u

#endif// MTCA4U_TMC429_DUMMY_CONSTANTS_H

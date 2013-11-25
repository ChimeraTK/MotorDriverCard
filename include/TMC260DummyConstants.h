#ifndef MTCA4U_TMC260_DUMMY_CONSTANTS_H
#define MTCA4U_TMC260_DUMMY_CONSTANTS_H

#include <stdint.h>

namespace mtca4u{

namespace tmc260{

  uint32_t const SIZE_OF_SPI_ADDRESS_SPACE = 0x8;
  
  inline unsigned int testWordFromSpiAddress(unsigned int spiAddress, unsigned int motorID){
    return spiAddress*spiAddress + 7 + motorID;
  }

  inline unsigned int spiAddressFromDataWord( MultiVariableWord const & word ){
    return spiAddressFromDataWord( word.getDataWord() ); 
  }

  inline unsigned int spiAddressFromDataWord( unsigned int word ){
    // Sorry for this mess, but I don't see any other chance to implement
    // "if bits 18 and 19 are zero then these bits are the address, else
    //  bits 17 to 19 are the address" than hard-coding it.
    if ((word & OUTPUT_MASK(18,19)) == 0){
      return 0;
    }
    else{
      return ( (word & OUTPUT_MASK(17,19)) >> 17 );
    }
  }

  inline unsigned int dataMaskFromSpiAddress(unsigned int spiAddress){
    if (spiAddress == 0){
      return OUTPUT_MASK(0, 17);
    }
    else{
      return OUTPUT_MASK(0, 16);
    }
  }

}// namespace tmc260

}// namespace mtca4u

#endif// MTCA4U_TMC260_DUMMY_CONSTANTS_H

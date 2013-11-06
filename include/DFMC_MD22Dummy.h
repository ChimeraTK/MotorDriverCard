#ifndef MTCA4U_DFMC_MD22_DUMMY_H
#define MTCA4U_DFMC_MD22_DUMMY_H

#include "DummyDevice.h"

namespace mtca4u{
  
  class DFMC_MD22Dummy : public DummyDevice{
  public:
    void openDev(const std::string &mappingFileName,
		 int perm = O_RDWR, devConfigBase* pConfig = NULL);
  private:
    // callback functions
    void handleSPIWrite();

    // members
    std::vector<unsigned int> _spiAddressSpace;
    uint32_t _spiWriteAddress;
    uint32_t _spiWriteBar;
    uint32_t _spiReadbackAddress;
    uint32_t _spiReadbackBar;
    
    // internal and helper functions
    void writeContentToSpiRegister(unsigned int content, unsigned int spiAddress);
    void writeSpiContentToReadbackRegister(unsigned int spiAddress);
    void prepareSPIRegisters();
  };
}// namespace mtca4u

#endif // MTCA4U_DFMC_MD22_DUMMY_H

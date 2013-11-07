#ifndef MTCA4U_DFMC_MD22_DUMMY_H
#define MTCA4U_DFMC_MD22_DUMMY_H

#include "DummyDevice.h"

namespace mtca4u{
  
  class DFMC_MD22Dummy : public DummyDevice{
  public:
    void openDev(const std::string &mappingFileName,
		 int perm = O_RDWR, devConfigBase* pConfig = NULL);
    
    /** According to the data sheet, all registers are set to 0,
     *  except for the ChipVersion and the StepperMotorGlobalParameters register,
     *  which are initialised to the chip version and Clk2_div=15, respectively.
     */    
    void setSPIRegistersForOperation();

    /** Writes the test pattern spiAddress*spiAddress+13 to all registers.
     */
    void setSPIRegistersForTesting();
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
  };
}// namespace mtca4u

#endif // MTCA4U_DFMC_MD22_DUMMY_H

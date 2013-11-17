#ifndef MTCA4U_DFMC_MD22_DUMMY_H
#define MTCA4U_DFMC_MD22_DUMMY_H

#include "DummyDevice.h"
#include "TMC429Words.h"

namespace mtca4u{
  
  /** Dummy for the DFMC_MD22 Motor driver card.
   *
   * Implemented features:
   * \li All registers have full 32 bit data when written directly. Most registers are not write protected FIXME.
   * \li The following registers are automatically updated to the SPI values
   * after an SPI write. They are write protected:
   *  <ul> <li> ACTUAL_POSITION </li>
   *       <li> ACTUAL_VELOCITY </li>
   *       <li> ACTUAL_ACCELERATION </li>
   *       <li> MICRO_STEP_COUNT </li>
   *  </ul>
   *  The updated values are truncated to 24 bit. The 24 bit are not truncated according
   *  to the bit patterns of the TMC429 chip.
   *  \li Test words with content calculated from register index for SPI registers.
   *  \li Initialisation to start values of TMC429
   *  \li An spi address space of 0x40 words is defined, which can be writen via the
   *      SPI_CTRL register.
   *  \li A read request to the SPI_CTRL register updates the value in the SPI_CTRL_B
   *      register
   */
  class DFMC_MD22Dummy : public DummyDevice{
  public:
    DFMC_MD22Dummy();

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

    /** Function for debugging. We will probably not necessarily implement the 
     *  full power down behaviour. This dummy is mainly for checking the software
     *  and not for full simulation of the hardware (is it?).
     */
    bool isPowerUp();
    
    /** As power up is done via a pin on the TMC429 chip we need a function
     *  to emulate this in the dummy which is not in the normal device.
     */
    void powerUp();

  private:
    // callback functions
    void handleSPIWrite();

    // members
    std::vector<unsigned int> _spiAddressSpace;
    uint32_t _spiWriteAddress;
    uint32_t _spiWriteBar;
    uint32_t _spiReadbackAddress;
    uint32_t _spiReadbackBar;
    bool _powerIsUp;
    
    // internal and helper functions
    void writeContentToSpiRegister(unsigned int content, unsigned int spiAddress);
    void writeSpiContentToReadbackRegister(unsigned int spiAddress);
    void triggerActionsOnSpiWrite(TMC429InputWord const & inputWord);
    void performIdxActions( unsigned int idx );
    void performJdxActions( unsigned int jdx );

    /** In the DFMC_MD22 some registers of the SPI address space have a copy
     *  in the PCIe address space, which is updated by a loop when the FPGA is idle.
     *  The dummy simulates this behaviour.
     */
    void synchroniseFpgaWithSpiRegisters();
    void writeSpiRegisterToFpga( unsigned int ID, unsigned int IDX,
				 std::string suffix );

    void setPCIeRegistersForTesting();
  };
}// namespace mtca4u

#endif // MTCA4U_DFMC_MD22_DUMMY_H
#ifndef MTCA4U_DFMC_MD22_DUMMY_H
#define MTCA4U_DFMC_MD22_DUMMY_H

#include <MtcaMappedDevice/DummyDevice.h>
#include "TMC429Words.h"

namespace mtca4u{
  
  /** Dummy for the DFMC_MD22 Motor driver card.
   *
   * Implemented features:
   * \li All registers have full 32 bit data when written directly. Most registers are not write protected FIXME.
   * \li The following registers are automatically updated to the TMC429 controler SPI values
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
  class DFMC_MD22Dummy : public mtca4u::DummyDevice{
  public:
    DFMC_MD22Dummy();

    void openDev(const std::string &mappingFileName,
		 int perm = O_RDWR, devConfigBase* pConfig = NULL);
    
    /** Writes the test pattern to all registers.
     *  controlerSpiAddress*controlerSpiAddress+13 for the controler SPI address space.
     *  3*address*address+17 for the PCIe addresses.
     *  spiAddress*spiAddress + 7 + motorID for the drivers SPI addres spaces (motor 0 and 1)
     */
    void setRegistersForTesting();

    /** Function for debugging. We will probably not necessarily implement the 
     *  full power down behaviour. This dummy is mainly for checking the software
     *  and not for full simulation of the hardware (is it?).
     */
    bool isPowerUp();
    
    /** As power up is done via a pin on the TMC429 chip we need a function
     *  to emulate this in the dummy which is not in the normal device.
     */
    void powerUp();

    /** For debugging the dummy allows to read back the driver spi registers */
    unsigned int readDriverSpiRegister( unsigned int motorID, unsigned int driverSpiAddress );

    /** For testing all driver registers can be set to test values (also those initialised by the config)
     */
    void setDriverSpiRegistersForTesting();

    /** The dummy does not react on the SPI write registers if this flag is set true.
     */
    void causeSpiTimeouts(bool causeTimeouts);

    /** The dummy always reports an SPI error in the sync registers if this flag is set true.
     */
    void causeSpiErrors(bool causeErrors);

    

  private:
    // callback functions
    void handleControlerSpiWrite();
    void handleDriverSpiWrite(unsigned int ID);

    // members
    std::vector<unsigned int> _controlerSpiAddressSpace;
    uint32_t _controlerSpiWriteAddress;
    uint32_t _controlerSpiWriteBar;
    uint32_t _controlerSpiReadbackAddress;
    uint32_t _controlerSpiReadbackBar;
    bool _powerIsUp;
    
    // internal and helper functions
    void writeContentToControlerSpiRegister(unsigned int content, unsigned int controlerSpiAddress);
    void writeControlerSpiContentToReadbackRegister(unsigned int controlerSpiAddress);
    void triggerActionsOnControlerSpiWrite(TMC429InputWord const & inputWord);
    void performIdxActions( unsigned int idx );
    void performJdxActions( unsigned int jdx );

    /** In the DFMC_MD22 some registers of the controler SPI address space have a copy
     *  in the PCIe address space, which is updated by a loop when the FPGA is idle.
     *  The dummy simulates this behaviour.
     */
    void synchroniseFpgaWithControlerSpiRegisters();
    void writeControlerSpiRegisterToFpga( unsigned int ID, unsigned int IDX,
					  std::string suffix );

    void setPCIeRegistersForTesting();
    void setDriverSpiRegistersForTesting(unsigned int motorID);

    void setControlerSpiRegistersForOperation();
    void setControlerSpiRegistersForTesting();

    /// Helper struct to simulate the spi for one driver
    struct DriverSPI{
      /// each driver has it's own address space vector<uint>
       std::vector<unsigned int> addressSpace;
      unsigned int bar;
      unsigned int pcieWriteAddress;
      unsigned int pcieSyncAddress;
    };

    std::vector< DriverSPI > _driverSPIs;

    bool _causeSpiTimeouts;
    bool _causeSpiErrors;
  };
}// namespace mtca4u

#endif // MTCA4U_DFMC_MD22_DUMMY_H

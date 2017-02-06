#ifndef MTCA4U_DFMC_MD22_DUMMY_H
#define MTCA4U_DFMC_MD22_DUMMY_H

#include <mtca4u/DummyBackend.h>
#include "TMC429Words.h"
#include <mtca4u/BackendFactory.h>
#include <mtca4u/DeviceAccessVersion.h>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace mtca4u{

// forward declarations
class TMC429InputWord;
class TMC429OutputWord;

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
  class DFMC_MD22Dummy : public mtca4u::DummyBackend{
  public:
    enum class TMC260Register{
      DRIVER_CONTROL_REG = 0, // DRVCTRL
      CHOPPER_CONTROL_REG = 4, // CHOPCONF
      COOLSTEP_CONTROL_REG, // SMARTEN
      STALLGUARD2_CONTROL_REGISTER, // SGCSCONF
      DRIVER_CONFIG_REGISTER // DRVCONF
    };


    /*
     * @brief The class creates an addressspace in RAM for all the registers in
     * tmc429ControllerModuleName of the input mapFile. The behavior of these
     * registers when accessed mimic the actual firmware used on the DFMC_MD22
     * board
     *
     * @param mapfileName                : Name of the mapfile for the DFMC_MD22
     *                                     motor driver card.
     * @param tmc429ControllerModuleName : Name of a motor controller module in
     *                                     the above mapfile. A motor controller
     *                                     module has a set of global registers
     *                                     and two sets of registers specific
     *                                     for each motor controlled by this
     *                                     TMC429 IC
     *
     */
    DFMC_MD22Dummy(std::string const & mapFileName, std::string const & tmc429ControllerModuleName);
    void open();
    
    /** Writes the test pattern to all registers.
     *  controlerSpiAddress*controlerSpiAddress+13 for the controler SPI address space.
     *  3*address*address+17 for the PCIe addresses.
     *  spiAddress*spiAddress + 7 + motorID for the drivers SPI addres spaces (motor 0 and 1)
     * 
     *  Exception: The registers from the standard register set are set to their no1rmal values.
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
     *  The limitToNTimeouts allows to return to normal operation (causesSpiTimeouts = false)
     *  when the specified number if timeouts has been caused.
     *  If the limitToNTimeouts is left at 0 there will be an unlimited number of timeouts.
     */
    void causeSpiTimeouts(bool causeTimeouts, unsigned int limitToNTimeouts=0);

    /** The dummy always reports an SPI error in the sync registers if this flag is set true.
     */
    void causeSpiErrors(bool causeErrors);

    /** Overwrite the firmware version for testing. To restore the dummy default use
     *  resetFirmwareVersion().
     */
    void setFirmwareVersion(uint32_t version);

    /** Reset the firmware version to 0xMMmmFFFF (dummy default).
     */
    void resetFirmwareVersion();

    /** Set the simulated SPI delay for the TMC429 controller chip in microseconds.
     */
    void setControllerSpiDelay(unsigned int microseconds);

    /** Set the simulated SPI delay for the TMC260 driver chip in microseconds.
     */
    void setDriverSpiDelay(unsigned int microseconds);

    /*
     * @brief read the configurable TMC429 registers.
     *
     * @param SPIDatagram: SPI datagram framed per TMC429 datasheet to read the
     * desired register. This method expects the SPIDatagram to have the RW bit
     * set to read(1)
     *
     * @returns The SPI response datagram per the TMC429 datasheet.
     */
    uint32_t readTMC429Register(uint32_t SPIDatagram);

    /*
     * @brief read out register contents of the TMC260 driver IC for a
     * motor
     *
     * @returns: Returns the content of the desired register as a uint32_t
     * that can be provided as an input to the corresponding derived version of
     * mtca4u::TMC260Word
     */
    uint32_t readTMC260Register(uint32_t motorID, TMC260Register configuredRegister);


    static boost::shared_ptr<mtca4u::DeviceBackend> createInstance(std::string host, std::string interface,
        std::list<std::string> parameters, std::string mapFileName);


  private:
    // callback functions
    void handleControlerSpiWrite();
    void handleDriverSpiWrite(unsigned int ID);

    // members
    std::vector<unsigned int> _controlerSpiAddressSpace;
    uint32_t _controlerSpiWriteAddress;
    uint32_t _controlerSpiBar;
    uint32_t _controlerSpiReadbackAddress;
    uint32_t _controlerSpiSyncAddress;
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
    void setStandardRegisters();
    /** Set the first word of an address range and make it constant. Intended only for
     *  single word address ranges. In larger address ranges only the first word is set and made const.
     */
    void setConstantPCIeRegister( std::string registerName, int32_t content);
    void setDriverSpiRegistersForTesting(unsigned int motorID);

    void setControlerSpiRegistersForOperation();
    void setControlerSpiRegistersForTesting();

    void driverSpiActions(unsigned int ID, unsigned int spiAddress, unsigned int payloadData);
    uint32_t getDatalengthFromMask(uint32_t mask);

    /// Helper struct to simulate the spi for one driver
    struct DriverSPI{
      /// each driver has it's own address space vector<uint>
      std::vector<unsigned int> addressSpace;
      unsigned int bar;
      unsigned int pcieWriteAddress;
      unsigned int pcieSyncAddress;
      DriverSPI();
    };

    std::vector< DriverSPI > _driverSPIs;

    bool _causeSpiTimeouts;
    unsigned int _nSpiTimeoutsLeft; // when this counter reaches 0 it sets _causeSpiTimeouts back to false
    bool _causeSpiErrors;

    unsigned int _microsecondsControllerSpiDelay;
    unsigned int _microsecondsDriverSpiDelay;
    
    std::string _moduleName;


    bool checkStatusOfRWBit(const TMC429InputWord& inputSPMessage, uint32_t bitStatus);
    TMC429OutputWord frameTMC429SPIResponse(TMC429InputWord &inputSpi);

    // If the timeout counter is not 0 it is decremented. Then it goes from 1 to 0 the
    // _causeSpiTimeouts flag is set to false.
    void checkSpiTimeoutsCounter();
  };
  class DFMC_MD22DummyRegisterer{
  public:
  	DFMC_MD22DummyRegisterer(){
  #ifdef _DEBUG
          std::cout<<"DFMC_MD22DummyRegisterer"<<std::endl;
  #endif
          BackendFactory::getInstance().registerBackendType("dfmc_md22dummy","",&DFMC_MD22Dummy::createInstance, CHIMERATK_DEVICEACCESS_VERSION);
    }
  };


}// namespace mtca4u

#endif // MTCA4U_DFMC_MD22_DUMMY_H

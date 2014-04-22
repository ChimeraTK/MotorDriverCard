#ifndef MTCA4U_SPI_VIA_PCIE_H
#define MTCA4U_SPI_VIA_PCIE_H

#include <MtcaMappedDevice/devMap.h>

namespace mtca4u{
  /** This class implements synchronous SPI operation over PCIexpress, using an SPI command register
   *  and a synchronisation register. Readback is optional.
   * 
   */
  class SPIviaPCIe{
  public:
    /** Constructor for write-only implementations.
     *  It intentionally is overloaded and not a version with an invalid default value for the 
     *  readback register because the register accessors have to be implemented in the initialiser list,
     *  and case switches there would be messy.
     *
     *  The spi command has to be composed by the calling code in order to keep this class
     *  universal.
     */
    SPIviaPCIe( boost::shared_ptr< devMap<devBase> > & mappedDevice,
		std::string const & writeRegisterName, std::string const & syncRegisterName );

    /// Constructor for write and readback.
    SPIviaPCIe( boost::shared_ptr< devMap<devBase> > & mappedDevice,
		std::string const & writeRegisterName, std::string const & syncRegisterName,
		std::string const & readbackRegisterName );

    uint32_t read( int32_t spiCommand ); ///< Write the command and return the readback value.

    /** Write the spi command. This methods blocks until the firmware has returned either success
     *  or an error.
     *  @fixme This function should have a timeout. Use asio?
     */
    void write( int32_t spiCommand );

  private:
    // No need to keep an instance of the mappedDevice shared pointer. Each accessor has one.
    mtca4u::devMap<devBase>::regObject _writeRegister;
    mtca4u::devMap<devBase>::regObject _synchronisationRegister;
    mtca4u::devMap<devBase>::regObject _readbackRegister;

  };

//  class TMC429SPI{
//  public:
//    TMC429SPI( boost::shared_ptr<devPCIE> );
//    void read( unsigned int smda, unsigned int idx_jdx );
//    void write( unsigned int smda, unsigned int idx_jdx, unsigned int data );
//    void write( TMC429InputWord const & writeWord );
//  };

}//namespace mtca4u

#endif// MTCA4U_SPI_VIA_PCIE_H

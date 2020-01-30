#ifndef CHIMERATK_SPI_VIA_PCIE_H
#define CHIMERATK_SPI_VIA_PCIE_H

#include <ChimeraTK/Device.h>

#include <boost/thread/recursive_mutex.hpp>
namespace mtca4u {

  /** This class implements synchronous SPI operation over PCIexpress, using an
   * SPI command register and a synchronisation register. Readback is optional.
   *
   */
  class SPIviaPCIe {
   public:
    static unsigned int const SPI_DEFAULT_WAITING_TIME = 40; ///< microseconds

    /** Constructor for write-only implementations.
     *  It intentionally is overloaded and not a version with an invalid default
     * value for the readback register because the register accessors have to be
     * implemented in the initialiser list, and case switches there would be
     * messy.
     *
     *  The spi command has to be composed by the calling code in order to keep
     * this class universal.
     *
     *  The spiWaitingTime is in microseconds. It defaults to 40 microseconds
     * which should be ok for writing 32 bits at 1 MHz.
     *
     *  An internal copy of the shared pointer of the mapped device is held in
     * this class, so the SPIviaPCIe always stays valid, even if the original
     * shared pointer goes out of scope.
     */
    SPIviaPCIe(boost::shared_ptr<ChimeraTK::Device> const& device,
        std::string const& moduleName,
        std::string const& writeRegisterName,
        std::string const& syncRegisterName,
        unsigned int spiWaitingTime = SPI_DEFAULT_WAITING_TIME);

    /// Constructor for write and readback.
    SPIviaPCIe(boost::shared_ptr<ChimeraTK::Device> const& device,
        std::string const& moduleName,
        std::string const& writeRegisterName,
        std::string const& syncRegisterName,
        std::string const& readbackRegisterName,
        unsigned int spiWaitingTime = SPI_DEFAULT_WAITING_TIME);

    uint32_t read(int32_t spiCommand); ///< Write the command and return the readback value.

    /** Write the spi command. This methods blocks until the firmware has returned
     * either success or an error. In case of an error a MotorDriverException is
     * thrown. After 10 waiting cycles and checks of the synchronisation register
     * the operation is timed out and a MotorDriverException is thrown.
     *
     *  @throw MotorDriverException
     */
    void write(int32_t spiCommand);

    /** The FPGA needs some time to perform the SPI communication to the connected
     * chip. This is the waiting time between the checks of the synchronisation
     * register. It should be set to approximately the time needed for the SPI
     * communication (a little bit larger to compensate for time jitter, so only
     * one waiting cycle is needed). Don't set it to 0 because after 10 attempts
     * without valid synchronisation the read/write operation is timed out and an
     * error is reported.
     */
    void setSpiWaitingTime(unsigned int microSeconds);

    /** Read back the waiting time which is set.
     */
    unsigned int getSpiWaitingTime() const;

   private:
    // No need to keep an instance of the  shared pointer. Each accessor has one.
    ChimeraTK::ScalarRegisterAccessor<int32_t> _writeRegister;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _synchronisationRegister;
    ChimeraTK::ScalarRegisterAccessor<int32_t> _readbackRegister;

    static void sleepMicroSeconds(unsigned int microSeconds);
    /** Time in microseconds to wait for the transaction to be finished on the SPI
     * bus
     */

    unsigned int _spiWaitingTime;
    // store module name for debug purposes
    std::string _moduleName;
    // store these two register names for debug output (exception messages)
    std::string _writeRegisterName;
    std::string _syncRegisterName;

    mutable boost::recursive_mutex _spiMutex;
  };

} // namespace mtca4u

#endif // CHIMERATK_SPI_VIA_PCIE_H

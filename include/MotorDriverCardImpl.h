#ifndef MTCA4U_MOTOR_DRIVER_CARD_IMPL_H
#define MTCA4U_MOTOR_DRIVER_CARD_IMPL_H

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <MtcaMappedDevice/devMap.h>

#include "MotorDriverCardExpert.h"
#include "MotorControler.h"
#include "MotorDriverCardConfig.h"
#include "PowerMonitor.h"
#include "TMC429Words.h"

namespace mtca4u
{
  /**
   * The implementation of the DFMC-MD22 motor driver card.
   */
  class MotorDriverCardImpl: public MotorDriverCardExpert{
  public:

    //FIXME: 
    /* how to construct?
       a) using the device and subdevice ID?
       b) using the MtcaMappedDevice?
       c) only via factotry = MtcaCrate class?
     */
    /// For the time being we require a working version of MtcaMappedDevice in addition 
    /// to the configuration
    MotorDriverCardImpl(boost::shared_ptr< devMap<devBase> > const & mappedDevice,
			MotorDriverCardConfig const & cardConfiguration);

    MotorControler & getMotorControler(unsigned int motorControlerID);
    
 
    /// Get a reference to the power monitor.
    PowerMonitor & getPowerMonitor();

    unsigned int getControlerChipVersion();

    void setDatagramLowWord(unsigned int datagramLowWord);
    unsigned int getDatagramLowWord();

    void setDatagramHighWord(unsigned int datagramHighWord);
    unsigned int getDatagramHighWord();

    void setCoverPositionAndLength(
                  CoverPositionAndLength const & coverPositionAndLength);
    CoverPositionAndLength getCoverPositionAndLength();

    void setCoverDatagram(unsigned int coverDatagram);
    unsigned int getCoverDatagram();

    void setStepperMotorGlobalParameters(
		  StepperMotorGlobalParameters const & stepperMotorGlobalParameters);
    StepperMotorGlobalParameters getStepperMotorGlobalParameters();

    void setInterfaceConfiguration(
                  InterfaceConfiguration const & interfaceConfiguration);
    InterfaceConfiguration getInterfaceConfiguration();

    /// Very expert function only avaialble in the implementation. Usually
    /// the programmer should not have to touch this.
    void setPositionCompareWord(unsigned int positionCompareWord); //??? data sheet is completely unclear
    unsigned int getPositionCompareWord(); //??? data sheet is completely unclear

    /// Very expert function only avaialble in the implementation. Usually
    /// the programmer should not have to touch this.
    void setPositionCompareInterruptData(
		  PositionCompareInterruptData const & positionCompareInterruptData);
    PositionCompareInterruptData getPositionCompareInterruptData();

    void powerDown();
    ReferenceSwitchData getReferenceSwitchData();

    /** The FPGA needs some time to perform the SPI communication to the controler chip.
     *  This is the waiting time added before the write command to the controler SPI register 
     *  returns. During this time the value should be written and it is safe to write a new word
     *  to the PCIe register. Writing a sequence of SPI commands without the waiting time
     *  could cause a word in the PCIe register to be overwritten without having been transmittet via
     *  SPI. There is no SPI command buffer in the FPGA.
     */
    void setSpiCommunicationWaitingTime(unsigned int microSeconds);
    
    unsigned int getSpiCommunicationWaitingTime() const;

  private:
    // Motor controlers need dynamic allocation, so we cannot store them directly.
    // As we do not want to care about cleaning up we use shared pointers.
    std::vector< boost::shared_ptr<MotorControler> > _motorControlers;

    boost::shared_ptr< devMap<devBase> > _mappedDevice;
    friend class MotorControler;

    devMap<devBase>::regObject _controlerSpiWriteRegister;
    devMap<devBase>::regObject _controlerSpiReadbackRegister;

    boost::scoped_ptr<PowerMonitor> _powerMonitor;

    TMC429OutputWord controlerSpiRead( unsigned int smda, unsigned int idx_jdx );

    void controlerSpiWrite( unsigned int smda, unsigned int idx_jdx, unsigned int data );
    void controlerSpiWrite( TMC429InputWord const & writeWord );

    static void sleepMicroSeconds(unsigned int microSeconds);
    unsigned int _spiCommunicationWaitingTime; ///< in microseconds
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_CARD_IMPL_H

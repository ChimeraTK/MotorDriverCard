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
#include "SPIviaPCIe.h"

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
    MotorDriverCardImpl(boost::shared_ptr< mtca4u::devMap<devBase> > const & mappedDevice,
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

  private:
    // Motor controlers need dynamic allocation, so we cannot store them directly.
    // As we do not want to care about cleaning up we use shared pointers.
    std::vector< boost::shared_ptr<MotorControler> > _motorControlers;

    boost::shared_ptr< mtca4u::devMap<devBase> > _mappedDevice;
    friend class MotorControler;

    boost::scoped_ptr<PowerMonitor> _powerMonitor;

    TMC429OutputWord controlerSpiRead( unsigned int smda, unsigned int idx_jdx );

    void controlerSpiWrite( unsigned int smda, unsigned int idx_jdx, unsigned int data );
    void controlerSpiWrite( TMC429InputWord const & writeWord );

    SPIviaPCIe _controlerSPIviaPCIe;
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_CARD_IMPL_H

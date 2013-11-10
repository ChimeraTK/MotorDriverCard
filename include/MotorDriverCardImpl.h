#ifndef MTCA4U_MOTOR_DRIVER_CARD_IMPL_H
#define MTCA4U_MOTOR_DRIVER_CARD_IMPL_H

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <MtcaMappedDevice/devMap.h>

#include "MotorDriverCardExpert.h"
#include "MotorControler.h"
#include "PowerMonitor.h"
#include "TMC429Words.h"

namespace mtca4u
{
  /**
   * The implementation of the DFMC-MD22 motor driver card.
   */
  class MotorDriverCardImpl: public MotorDriverCardExpert{
  public:
    // the helper classes
    struct MotorDriverConfiguration{};
    
    //FIXME: 
    /* how to construct?
       a) using the device and subdevice ID?
       b) using the MtcaMappedDevice?
       c) only via factotry = MtcaCrate class?
     */
    /// For the time being we require a working version of MtcaMappedDevice in addition 
    /// to the configuration
    MotorDriverCardImpl(boost::shared_ptr< devMap<devBase> > const & mappedDevice,
			MotorDriverConfiguration const & motorDriverConfiguration);

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
    void setPositionCompareRegister(unsigned int positionCompareWord); //??? data sheet is completely unclear
    unsigned int getPositionCompareRegister(); //??? data sheet is completely unclear

    /// Very expert function only avaialble in the implementation. Usually
    /// the programmer should not have to touch this.
    void setPositionCompareInterruptRegister(
		  PositionCompareInterruptData const & positionCompareInterruptData);
    PositionCompareInterruptData getPositionCompareInterruptRegister();

    void powerDown();
    ReferenceSwitchData getReferenceSwitchRegister();


  private:
    // Motor controlers need dynamic allocation. So we cannot store them directly.
    // As we do not want to care about cleaning up we use scoped pointers.
    // FIXME: should they be shared pointers? What about the copy constructor?
    // scoped pointers cannot be copied.    
    std::vector< boost::shared_ptr<MotorControler> > _motorControlers;

    boost::shared_ptr< devMap<devBase> > _mappedDevice;

    devMap<devBase>::regObject _spiControlWriteRegister;
    devMap<devBase>::regObject _spiControlReadbackRegister;

    boost::scoped_ptr<PowerMonitor> _powerMonitor;

    TMC429OutputWord spiRead( unsigned int smda, unsigned int idx_jdx );

    void spiWrite( unsigned int smda, unsigned int idx_jdx, unsigned int data );
    void spiWrite( TMC429InputWord const & writeWord );
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_CARD_IMPL_H

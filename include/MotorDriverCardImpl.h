#ifndef MTCA4U_MOTOR_DRIVER_CARD_IMPL_H
#define MTCA4U_MOTOR_DRIVER_CARD_IMPL_H

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <MtcaMappedDevice/devMap.h>

#include "MotorDriverCardExpert.h"
#include "MotorControler.h"
#include "PowerMonitor.h"

namespace mtca4u
{
  /**
   * The implementation of the DFMC-MD22 motor driver card.
   */
  class MotorDriverCardImpl: public MotorDriverCardExpert{
  public:
    // the helper classes
    struct MotorDriverConfiguration{};
    
    typedef TMC429Word PositionCompareInterruptData;

    //FIXME: move constructor to impl
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
    void setDatagramHighWord(unsigned int datagramHighWord);
    void setCoverPositionAndLength(
                  CoverPositionAndLength const & coverPositionAndLength);
    void setCoverDatagram(unsigned int coverDatagram);
    void setStepperMotorGlobalParametersRegister(
		  StepperMotorGlobalParameters const & stepperMotorGlobalParameters);
    void setInterfaceConfiguration(
                  InterfaceConfiguration const & interfaceConfiguration);
    /// Very expert function only avaialble in the implementation. Usually
    /// the programmer should not have to touch this.
    void setPositionCompareRegister(unsigned int positionCompareWord); //??? data sheet is completely unclear

    /// Very expert function only avaialble in the implementation. Usually
    /// the programmer should not have to touch this.
    void setPositionCompareInterruptRegister(
		  PositionCompareInterruptData const & positionCompareInterruptData);

    void powerDown();
    MotorDriverCard::ReferenceSwitchData getReferenceSwitchRegister();

  private:
    // Motor controlers need dynamic allocation. So we cannot store them directly.
    // As we do not want to care about cleaning up we use scoped pointers.
    // FIXME: should they be shared pointers? What about the copy constructor?
    // scoped pointers cannot be copied.    
    std::vector< boost::scoped_ptr<MotorControler> > _motorControlers;

    boost::shared_ptr< devMap<devBase> > _mappedDevice;

    devMap<devBase>::regObject _spiControlWriteRegister;
    devMap<devBase>::regObject _spiControlReadbackRegister;

    boost::scoped_ptr<PowerMonitor> _powerMonitor;

    TMC429Word spiRead( unsigned int smda, unsigned int idx_jdx );

    void spiWrite( unsigned int smda, unsigned int idx_jdx, unsigned int data );
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_CARD_IMPL_H

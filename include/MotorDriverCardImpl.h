#ifndef MTCA4U_MOTOR_DRIVER_CARD_IMPL_H
#define MTCA4U_MOTOR_DRIVER_CARD_IMPL_H

#include "MotorDriverCardExpert.h"

namespace mtca4u
{
  /**
   * The implementation of the DFMC-MD22 motor driver card.
   */
  class MotorDriverCardImpl: public MotorDriverCardExpert{
  public:
    // the helper classes
    struct MotorDriverConfiguration{};
    
    //FIXME: move constructor to impl
    /* how to construct?
       a) using the device and subdevice ID?
       b) using the MtcaMappedDevice?
       c) only via factotry = MtcaCrate class?
     */
    /// For the time being we require a working version of MtcaMappedDevice in addition 
    /// to the configuration
    MotorDriverCard(boost::shared_ptr<MtcaMappedDevice> & mappedDevice,
		    MotorDriverConfiguration & motorDriverConfiguration);

    MotorControler & getMotorControler(unsigned int motorControlerID);
    
 
    /// Get a reference to the power monitor.
    PowerMonitor & getPowerMonitor();

    /// FIXME: expert or not?
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

  private:
    // Motor controlers need dynamic allocation. So we cannot store them directly.
    // As we do not want to care about cleaning up we use scoped pointers.
    std::vector[boost::scoped_ptr<MotorController>] _motorControlers;
 
    unsigned int SPIControlWriteAddress;
    unsigned int SPIControlReadbackAddress;
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_CARD_IMPL_H

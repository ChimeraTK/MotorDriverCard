#ifndef MTCA4U_MOTOR_DRIVER_CARD_EXPERT_H
#define MTCA4U_MOTOR_DRIVER_CARD_EXPERT_H

#include "TMC429Words.h"
#include "MotorDriverCard.h"

namespace mtca4u
{
  /**
   * Extendet interface for experts of the DFMC-MD22 motor driver card.
   */
  class MotorDriverCardExpert: public MotorDriverCard{
  public:

    /// Expert function 
    virtual void setDatagramLowWord(unsigned int datagramLowWord) = 0;
    /// Expert function 
    virtual void setDatagramHighWord(unsigned int datagramHighWord) = 0;
    /// Expert function 
    virtual void setCoverPositionAndLength(
		    CoverPositionAndLength const & coverPositionAndLength) = 0;
    /// Expert function 
    virtual void setCoverDatagram(unsigned int coverDatagram) = 0;
    /// Expert function 
    virtual void setStepperMotorGlobalParametersRegister(
		    StepperMotorGlobalParameters const & stepperMotorGlobalParameters) = 0;

    /// Expert function: Set the Interface configuration register of the
    /// TCM429 chip
    virtual void setInterfaceConfiguration(
		    InterfaceConfiguration const & interfaceConfiguration) = 0;

   /// Expert function: Power down the TCM429 chip
    virtual void powerDown() = 0;
 
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_CARD_EXPERT_H

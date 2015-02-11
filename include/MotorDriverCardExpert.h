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

    virtual void setDatagramLowWord(unsigned int datagramLowWord) = 0;
    virtual unsigned int getDatagramLowWord() = 0;

    virtual void setDatagramHighWord(unsigned int datagramHighWord) = 0;
    virtual unsigned int getDatagramHighWord() = 0;

    virtual void setCoverPositionAndLength(
		    CoverPositionAndLength const & coverPositionAndLength) = 0;
    virtual CoverPositionAndLength getCoverPositionAndLength() = 0;

    virtual void setCoverDatagram(unsigned int coverDatagram) = 0;
    virtual unsigned int getCoverDatagram() = 0;

    virtual void setStepperMotorGlobalParameters(
		    StepperMotorGlobalParameters const & stepperMotorGlobalParameters) = 0;
    virtual StepperMotorGlobalParameters getStepperMotorGlobalParameters() = 0;

    virtual void setInterfaceConfiguration(
		    InterfaceConfiguration const & interfaceConfiguration) = 0;
    virtual InterfaceConfiguration getInterfaceConfiguration() = 0;

    virtual void powerDown() = 0;
 
    /** The reference switch data from the controler chip, as provided by the card,
	are expert because there is a MotorControler::getReferenceSwitchData() function
	implemented which should be used.
    */
    virtual ReferenceSwitchData getReferenceSwitchData() = 0;

    virtual unsigned int getControlerChipVersion() = 0;
 
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_CARD_EXPERT_H

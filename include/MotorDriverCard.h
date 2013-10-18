namespace mtca4u
{
  /**
   * A class to access the DFMC-MD22 motor driver card, which prividers two 
   * MotorControllers.
   */
  class MotorDriverCard{
    /* how to construct?
       a) using the device and subdevice ID?
       b) using the MtcaMappedDevice?
       c) only via factotry = MtcaCrate class?
     */

    MotorController & getMotorController(unsigned int motorControllerID);
    
    void setDatagramLowWord(unsigned int datagramLowWord);
    void setDatagramHighWord(unsigned int datagramHighWord);
    void setCoverPositionAndLength(CoverPositionAndLength coverPositionAndLength);
    void setCoverDatagram(unsigned int coverDatagram);
    void setStepperMotorGlobalParametersRegister(StepperMotorGlobalParameters stepperMotorGlobalParameters);

    //FIXME: implement the 429 special config as either derrived class, implementation or decorator
    void setInterfaceConfiguration429(InterfaceConfiguration429 interfaceConfiguration429);
    void setPositionCompareRegister429(unsigned int positionCompareWord429); //??? data sheet is completely unclear
    void setPositionCompareInterruptRegister429(PositionCompareInterruptData positionCompareInterruptData);
    void powerDown429();
    void setReferenceSwitchRegister429(ReferenceSwitchData429 referenceSwitchData429);

    unsigned int getControllerChipVersion429();
 
    PowerMonitor & getPowerMonitor(); //< Get a reference to the power monitor.
  };
  
}// namespace mtca4u

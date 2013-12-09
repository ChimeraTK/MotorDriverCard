#ifndef MOTOR_DRIVER_CARD_CONFIG_H
#define MOTOR_DRIVER_CARD_CONFIG_H

#include <vector>

#include "MotorControlerConfig.h"
#include "TMC429Words.h"

namespace mtca4u{

  /** This configuration contains start values for all data words that 
   *  can be written to the MotorDriverImpl class, and configurations for
   *  the MotorDrivers is contains.
   */  
  struct MotorDriverCardConfig{
    unsigned int coverDatagram;
    CoverPositionAndLength coverPositionAndLength;
    unsigned int datagramHighWord;
    unsigned int datagramLowWord;
    InterfaceConfiguration interfaceConfiguration;
    PositionCompareInterruptData positionCompareInterruptData;
    unsigned int positionCompareWord;
    StepperMotorGlobalParameters stepperMotorGlobalParameters;
    
    std::vector< MotorControlerConfig > motorControlerConfigurations;

    MotorDriverCardConfig(); 
  };
}// namespace mtca4u

#endif // MOTOR_DRIVER_CARD_CONFIG_H

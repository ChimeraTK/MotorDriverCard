#ifndef MOTOR_DRIVER_CARD_CONFIG_H
#define MOTOR_DRIVER_CARD_CONFIG_H

#include "MotorControlerConfig.h"
#include "TMC429Words.h"

#include <vector>

namespace mtca4u {

  /** This configuration contains start values for all data words that
   *  can be written to the MotorDriverImpl class, and configurations for
   *  the MotorDrivers is contains.
   */
  struct MotorDriverCardConfig {
    unsigned int coverDatagram;
    CoverPositionAndLength coverPositionAndLength;
    unsigned int datagramHighWord;
    unsigned int datagramLowWord;
    InterfaceConfiguration interfaceConfiguration;
    PositionCompareInterruptData positionCompareInterruptData;
    unsigned int positionCompareWord;
    StepperMotorGlobalParameters stepperMotorGlobalParameters;

    unsigned int controlerSpiWaitingTime;

    std::vector<MotorControlerConfig> motorControlerConfigurations;

    MotorDriverCardConfig();

    bool operator==(MotorDriverCardConfig const& right) const;
  };
} // namespace mtca4u

#endif // MOTOR_DRIVER_CARD_CONFIG_H

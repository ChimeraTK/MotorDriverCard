#include "MotorDriverCardConfig.h"

unsigned int const INTERFACE_CONFIGURATION_DEFAULT = 0x000123;
unsigned int const STEPPER_MOTOR_GLOBAL_PARAMETERS_DEFAULT = 0x010702;

namespace mtca4u{

  MotorDriverCardConfig::MotorDriverCardConfig()
    : coverDatagram(0),
      coverPositionAndLength(0),
      datagramHighWord(0),
      datagramLowWord(0),
      interfaceConfiguration(INTERFACE_CONFIGURATION_DEFAULT),
      positionCompareInterruptData(0),
      positionCompareWord(0),
      stepperMotorGlobalParameters(STEPPER_MOTOR_GLOBAL_PARAMETERS_DEFAULT)
  {
    // initialises the with the default constructors of the motor controler config
    motorControlerConfigurations.resize( dfmc_md22::N_MOTORS_MAX );
  }

}// namespace mtca4u

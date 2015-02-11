#ifndef MOTOR_DRIVER_CARD_CONFIG_DEFAULTS
#define MOTOR_DRIVER_CARD_CONFIG_DEFAULTS

// This file should only be included for testing/debugging. It should not be necessary 
// to use these constants in any production code (except for the MotorDriverCardConfig 
// implementation of course).

namespace mtca4u{

  // controler registers
  unsigned int const INTERFACE_CONFIGURATION_DEFAULT = 0x000122;
  unsigned int const STEPPER_MOTOR_GLOBAL_PARAMETERS_DEFAULT = 0x010701;

  // spi communication parametes
  unsigned int const CONTROLER_SPI_WAITING_TIME_DEFAULT = 50; // microseconds

}// namespace mtca4u


#endif // MOTOR_DRIVER_CARD_CONFIG_DEFAULTS

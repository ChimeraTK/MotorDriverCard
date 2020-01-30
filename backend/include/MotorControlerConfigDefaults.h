#ifndef MOTOR_CONTROLER_CONFIG_DEFAULTS
#define MOTOR_CONTROLER_CONFIG_DEFAULTS

// This file should only be included for testing/debugging. It should not be
// necessary to use these constants in any production code (except for the
// MotorControlerConfig implementation of course).

namespace mtca4u {

  // registers on the driver chip
  unsigned int const DRIVER_CONTROL_DEFAULT = 0x0; ///< 256 microsteps
  unsigned int const CHOPPER_CONTROL_DEFAULT = 0x1F3;
  unsigned int const STALL_GUARD_CONTROL_DEFAULT = 0x10000; ///< stall guard with current scale 1/32 of the max possible
                                                            ///< current
  unsigned int const COOL_STEP_CONTROL_DEFAULT = 0x0;       ///< cool step off
  unsigned int const DRIVER_CONFIG_DEFAULT = 0x00040; ///< full-scale sense register voltage is 165 mV. Do not modify
                                                      ///< for MD22

  // registers on the controler chip
  unsigned int const DIVIDIERS_AND_MICRO_STEP_RESOLUTION_DEFAULT = 0x003704;
  unsigned int const MAXIMUM_ACCELERATION_DEFAULT = 0x14;
  unsigned int const MAXIMUM_VELOCITY_DEFAULT = 0x64;
  unsigned int const MINIMUM_VELOCITY_DEFAULT = 0x1; ///< Should be 1 according to data sheet
  unsigned int const PROPORTIONALITY_FACTORS_DEFAULT = 0xC50A;
  unsigned int const REFERENCE_CONFIG_AND_RAMP_MODE_DEFAULT = 0x0;

  // spi communication parametes
  unsigned int const DRIVER_SPI_WAITING_TIME_DEFAULT = 50; ///< in microseconds
  // (contolrer SPI waiting time default is in the MotorDriverCard config)

} // namespace mtca4u

#endif // MOTOR_CONTROLER_CONFIG_DEFAULTS

#ifndef MTCA4U_TMC260_CONSTANTS_H
#define MTCA4U_TMC260_CONSTANTS_H

#include <stdint.h>

namespace mtca4u {

namespace tmc260 {

uint32_t const ADDRESS_DRIVER_CONTROL = 0x0;
uint32_t const ADDRESS_CHOPPER_CONFIG = 0x4;
uint32_t const ADDRESS_COOL_STEP_CONFIG = 0x5;
uint32_t const ADDRESS_STALL_GUARD_CONFIG = 0x6;
uint32_t const ADDRESS_DRIVER_CONFIG = 0x7;

} // namespace tmc260

} // namespace mtca4u

#endif // MTCA4U_TMC260_CONSTANTS_H

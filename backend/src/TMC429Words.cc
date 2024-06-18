#include "TMC429Words.h"

#include "ChimeraTK/Exception.h"

namespace mtca4u {

  bool TMC429StatusWord::getTargetPositionReached(unsigned int motorID) {
    if(motorID > 2) {
      throw ChimeraTK::logic_error("MotorID is too large");
    }
    return static_cast<bool>((getDataWord() >> motorID * 2) & 0x1);
  }

  unsigned int TMC429StatusWord::getReferenceSwitchBit(unsigned int motorID) {
    if(motorID > 2) {
      throw ChimeraTK::logic_error("MotorID is too large");
    }
    return static_cast<bool>((getDataWord() >> (motorID * 2 + 1)) & 0x1);
  }

} // namespace mtca4u

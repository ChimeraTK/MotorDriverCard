#include "TMC429Words.h"
#include "MotorDriverException.h"

namespace mtca4u{

  bool TMC429StatusWord::getTargetPositionReached(unsigned int motorID){
    if (motorID > 2){
      throw MotorDriverException("MotorID is too large", MotorDriverException::OUT_OF_RANGE);
    }
    return static_cast<bool>( (getDataWord() >> motorID*2) & 0x1 );
  }

  unsigned int TMC429StatusWord::getReferenceSwitchBit(unsigned int motorID){
    if (motorID > 2){
      throw MotorDriverException("MotorID is too large", MotorDriverException::OUT_OF_RANGE);
    }
    return static_cast<bool>( (getDataWord() >> (motorID*2 + 1) ) & 0x1 );
  }

}// namespace mtca4u

#include "StepperMotorError.h"

namespace mtca4u {
  using namespace ChimeraTK;

  const StepperMotorError StepperMotorErrorTypes::M_NO_ERROR(1);
  const StepperMotorError StepperMotorErrorTypes::M_COMMUNICATION_LOST(2);
  const StepperMotorError StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX(4);
  const StepperMotorError StepperMotorErrorTypes::M_NO_REACTION_ON_COMMAND(8);
  const StepperMotorError StepperMotorErrorTypes::M_HARDWARE_NOT_CONNECTED(16);

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // LinearStepperMotorErrorTypes class definition
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  const LinearStepperMotorError LinearStepperMotorErrorTypes::M_BOTH_END_SWITCH_ON(32);

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // StepperMotorError class definition !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  StepperMotorError::StepperMotorError() : GeneralStatus(1) {}

  std::string StepperMotorError::asString() const {
    std::ostringstream stream;

    if(_id == StepperMotorErrorTypes::M_NO_ERROR) {
      stream << "Motor is NOT IN ERROR (";
    }
    else if(_id == StepperMotorErrorTypes::M_COMMUNICATION_LOST) {
      stream << "Motor lost communication with hardware (";
    }
    else if(_id == StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX) {
      stream << "Error in configuration. Minimal software limit is greater than "
                "maximum software limit (";
    }
    else if(_id == StepperMotorErrorTypes::M_NO_REACTION_ON_COMMAND) {
      stream << "Motor is not reacting of last command (";
    }
    else if(_id == StepperMotorErrorTypes::M_HARDWARE_NOT_CONNECTED) {
      stream << "Hardware seems to be not connected. Check your hardware (";
    }
    else {
      return GeneralStatus::asString();
    }

    stream << _id << ")";
    return stream.str();
  }

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // LinearStepperMotorError class definition
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  std::string LinearStepperMotorError::asString() const {
    std::ostringstream stream;

    if(_id == LinearStepperMotorErrorTypes::M_BOTH_END_SWITCH_ON) {
      stream << "Both hardware end switches on. Check your hardware (";
    }
    else {
      return StepperMotorError::asString();
    }

    stream << _id << ")";
    return stream.str();
  }

} // namespace mtca4u

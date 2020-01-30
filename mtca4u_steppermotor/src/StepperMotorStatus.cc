#include "StepperMotorStatus.h"

namespace mtca4u {

  const LinearStepperMotorStatus LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON(128);
  const LinearStepperMotorStatus LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON(256);

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // LinearStepperMotorStatus class definition !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  LinearStepperMotorStatus::LinearStepperMotorStatus() : StepperMotorStatus(1) {}

  LinearStepperMotorStatus::LinearStepperMotorStatus(int id) : StepperMotorStatus(id) {}

  LinearStepperMotorStatus::LinearStepperMotorStatus(const LinearStepperMotorStatus& status)
  : StepperMotorStatus(status) {}

  // overload of asString method

  std::string LinearStepperMotorStatus::asString() const {
    std::ostringstream stream;
    // std::cout << "LinearStepperMotorStatus::asString" <<std::endl;
    if(_id == LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON) {
      stream << "Motor HARDWARE NEGATIVE END SWITCH is ON (";
    }
    else if(_id == LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON) {
      stream << "Motor HARDWARE POSITIVE END SWITCH is ON (";
    }
    else {
      return StepperMotorStatus::asString();
    }

    stream << _id << ")";
    return stream.str();
  }

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // StepperMotorStatus Types !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  const StepperMotorStatus StepperMotorStatusTypes::M_OK(1);
  const StepperMotorStatus StepperMotorStatusTypes::M_DISABLED(2);
  const StepperMotorStatus StepperMotorStatusTypes::M_IN_MOVE(4);
  const StepperMotorStatus StepperMotorStatusTypes::M_NOT_IN_POSITION(8);
  const StepperMotorStatus StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON(16);
  const StepperMotorStatus StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON(32);
  const StepperMotorStatus StepperMotorStatusTypes::M_ERROR(64);

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // StepperMotorStatus class definition !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  StepperMotorStatus::StepperMotorStatus() : GeneralStatus(1) {}

  StepperMotorStatus::StepperMotorStatus(int id) : GeneralStatus(id) {}

  StepperMotorStatus::StepperMotorStatus(const StepperMotorStatus& status) : GeneralStatus(status) {}

  // overload of asString method

  std::string StepperMotorStatus::asString() const {
    std::ostringstream stream;
    // std::cout << "StepperMotorStatus::asString" <<std::endl;
    if(_id == StepperMotorStatusTypes::M_OK) {
      stream << "Motor OK (";
    }
    else if(_id == StepperMotorStatusTypes::M_DISABLED) {
      stream << "Motor DISABLED (";
    }
    else if(_id == StepperMotorStatusTypes::M_IN_MOVE) {
      stream << "Motor IN MOVE (";
    }
    else if(_id == StepperMotorStatusTypes::M_NOT_IN_POSITION) {
      stream << "Motor NOT IN POSITION (";
    }
    else if(_id == StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON) {
      stream << "Motor SOFTWARE POSITIVE POSITION LIMIT CROSSED (";
    }
    else if(_id == StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON) {
      stream << "Motor SOFTWARE NEGATIVE POSITION LIMIT CROSSED (";
    }
    else if(_id == StepperMotorStatusTypes::M_ERROR) {
      stream << "Motor IN ERROR (";
    }
    else {
      return GeneralStatus::asString();
    }

    stream << _id << ")";
    return stream.str();
  }

} // namespace mtca4u

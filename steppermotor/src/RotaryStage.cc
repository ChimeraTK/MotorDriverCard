#include "RotaryStage.h"

#include "RotaryStageStateMachine.h"

namespace ChimeraTK::MotorDriver {
  std::shared_ptr<utility::StateMachine> RotaryStage::createStateMachine() {
    return std::make_shared<RotaryStageStateMachine>(*this);
  }

  // move calibration to motor.
} // namespace ChimeraTK::MotorDriver

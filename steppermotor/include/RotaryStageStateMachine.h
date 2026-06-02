#pragma once
#include "ReferenceStateMachine.h"
namespace ChimeraTK::MotorDriver {
  class RotaryStage;
  class RotaryStageStateMachine : public ReferenceStateMachine {
   public:
    explicit RotaryStageStateMachine(RotaryStage& motor);
    ~RotaryStageStateMachine() override = default;

   protected:
    void performCalibration() override;
    void calculateToleranceValues() override;
    int getPositionEndSwitch(Sign) override;
    void findEndSwitch(Sign sign) override;
    int getOffset() const override { return 5000; }
  };
} // namespace ChimeraTK::MotorDriver

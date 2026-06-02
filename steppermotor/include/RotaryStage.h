// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ReferenceStepperMotor.h"

namespace ChimeraTK::MotorDriver {
  class RotaryStageStateMachine; // forward declare

  class RotaryStage : public ReferenceStepperMotor {
   public:
    using ReferenceStepperMotor::ReferenceStepperMotor;
    ~RotaryStage() override = default;
    std::shared_ptr<utility::StateMachine> createStateMachine() override;

   protected:
    bool hasNegativeReferenceSwitch() const override { return false; }
  };

} // namespace ChimeraTK::MotorDriver

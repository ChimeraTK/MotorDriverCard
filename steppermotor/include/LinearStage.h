// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "ReferenceStepperMotor.h"

namespace ChimeraTK::MotorDriver {

  class LinearStageStateMachine; // forward declare

  class LinearStage : public ReferenceStepperMotor {
   public:
    using ReferenceStepperMotor::ReferenceStepperMotor;
    std::shared_ptr<utility::StateMachine> createStateMachine() override;
    ~LinearStage() override = default;

    // linear has real negative switch → default behavior is fine
  };

} // namespace ChimeraTK::MotorDriver

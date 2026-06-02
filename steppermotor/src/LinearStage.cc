// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "LinearStage.h"

#include "LinearStageStateMachine.h"

namespace ChimeraTK::MotorDriver {
  std::shared_ptr<utility::StateMachine> LinearStage::createStateMachine() {
    return std::make_shared<LinearStageStateMachine>(*this);
  }
  // nothing needed here unless we want to override calibration etc.

} // namespace ChimeraTK::MotorDriver

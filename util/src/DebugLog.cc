// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DebugLog.h"

#include <iostream>

namespace ChimeraTK::MotorDriver::utility {

  /********************************************************************************************************************/

  DebugLog& DebugLog::instance() {
    static DebugLog debugLog;
    return debugLog;
  }

  /********************************************************************************************************************/

  void DebugLog::write(const std::string& message) {
    std::lock_guard<std::mutex> lck(_mutex);
    if(_stream == nullptr) {
      _stream = &std::clog;
    }
    (*_stream) << _prefix << message << std::endl;
  }

  /********************************************************************************************************************/

  void DebugLog::setOutputStream(std::ostream& stream) {
    std::lock_guard<std::mutex> lck(_mutex);
    _stream = &stream;
  }

  /********************************************************************************************************************/

  void DebugLog::setPrefix(std::string prefix) {
    std::lock_guard<std::mutex> lck(_mutex);
    _prefix = std::move(prefix);
  }

} // namespace ChimeraTK::MotorDriver::utility

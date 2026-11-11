// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <mutex>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace ChimeraTK::MotorDriver::utility {

  /**
   * @brief Minimal debug logging facility for the MotorDriver library
   *
   * The library is deliberately free of a logging dependency: errors are reported by throwing exceptions, and no
   * runtime logging framework is pulled in. To still allow state changes to be traced during development, this header
   * provides two macros which write one line per message to a std::ostream (std::clog by default):
   *
   * - DEBUG_LOG(...)   - logs a debug message
   * - ERROR_LOG(...)   - logs an error message (e.g. immediately before throwing)
   *
   * Both accept any streamable arguments and assemble the line lazily. Both are always active.
   *
   * The sink and the per-line prefix can be changed via DebugLog::setOutputStream() and DebugLog::setPrefix(),
   * mainly intended for tests.
   */
  class DebugLog {
   public:
    /// Return the singleton instance.
    static DebugLog& instance();

    /// Write a single message to the configured stream, prefixed and newline-terminated. Thread-safe.
    void write(const std::string& message);

    /// Change the output stream (not owned). Defaults to std::clog.
    void setOutputStream(std::ostream& stream);

    /// Change the prefix written in front of every message. Defaults to "MotorDriver: ".
    void setPrefix(std::string prefix);

   private:
    DebugLog() = default;

    std::mutex _mutex;
    std::ostream* _stream{nullptr};
    std::string _prefix{"MotorDriver: "};
  };

  /********************************************************************************************************************/

  /// Stream an arbitrary number of arguments into a single string.
  template<typename... Args>
  std::string debugLogJoin(Args&&... args) {
    std::ostringstream stream;
    (stream << ... << std::forward<Args>(args));
    return stream.str();
  }

} // namespace ChimeraTK::MotorDriver::utility

/**********************************************************************************************************************/

/// Log a debug message about an internal state change.
#define DEBUG_LOG(...)                                                                                                 \
  ::ChimeraTK::MotorDriver::utility::DebugLog::instance().write(                                                       \
      ::ChimeraTK::MotorDriver::utility::debugLogJoin(__VA_ARGS__))

/// Log an error message with context.
#define ERROR_LOG(...)                                                                                                 \
  ::ChimeraTK::MotorDriver::utility::DebugLog::instance().write(                                                       \
      ::ChimeraTK::MotorDriver::utility::debugLogJoin(__VA_ARGS__))

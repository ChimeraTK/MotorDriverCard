#include "Logger.h"

#include <iosfwd>
#include <sstream>

namespace ChimeraTK { namespace MotorDriverCardDetail {

  Logger _logstdout__(Logger::NO_LOGGING, std::cout);
  Logger _logstderr__(Logger::NO_LOGGING, std::cerr);

  Logger::NullStream Logger::_nullStream;

  Logger::Logger(Logger::LogLevel level, std::ostream& logStream) : _logLevel(level), _loggingStream(logStream) {}

  Logger::~Logger() {}

  void Logger::setLogLevel(Logger::LogLevel level) {
    _logLevel = level;
  }

  Logger::LogLevel Logger::getLogLevel() const {
    return _logLevel;
  }

  std::ostream& operator<<(std::ostream& out, const Logger& loger) {
    out << "Current logging level: " << loger._logLevel;
    return out;
  }

  std::ostream& Logger::operator()(Logger::LogLevel level) {
    if(_logLevel >= level && level != NO_LOGGING) {
      return _loggingStream;
    }
    else
      return _nullStream;
  }

  std::ostream& Logger::operator()() {
    return _loggingStream;
  }

  const Logger::LogLevel Logger::NO_LOGGING(0);
  const Logger::LogLevel Logger::ERROR(1);
  const Logger::LogLevel Logger::WARNING(2);
  const Logger::LogLevel Logger::INFO(3);
  const Logger::LogLevel Logger::DETAIL(4);
  const Logger::LogLevel Logger::FULL_DETAIL(5);

  Logger::LogLevel::LogLevel(unsigned int level) : _level(level) {}

  std::ostream& operator<<(std::ostream& out, const Logger::LogLevel& level) {
    std::string str(level);
    out << str;
    return out;
  }

  Logger::LogLevel::operator unsigned int() const {
    return _level;
  }

  Logger::LogLevel::operator std::string() const {
    return asString();
  }

  std::string Logger::LogLevel::asString() const {
    std::ostringstream stream;
    if(_level == Logger::NO_LOGGING) {
      stream << "NO_LOGGING (";
    }
    else if(_level == Logger::ERROR) {
      stream << "ERROR (";
    }
    else if(_level == Logger::WARNING) {
      stream << "WARNING (";
    }
    else if(_level == Logger::INFO) {
      stream << "INFO (";
    }
    else if(_level == Logger::DETAIL) {
      stream << "DETAIL (";
    }
    else if(_level == Logger::FULL_DETAIL) {
      stream << "FULL_DETAIL (";
    }
    stream << _level << ")";
    return stream.str();
  }

  bool Logger::LogLevel::operator==(LogLevel const& right) const {
    if(_level != right._level) {
      return false;
    }
    return true;
  }

  bool Logger::LogLevel::operator!=(LogLevel const& right) const {
    if(_level == right._level) {
      return false;
    }
    return true;
  }
}} // namespace ChimeraTK::MotorDriverCardDetail

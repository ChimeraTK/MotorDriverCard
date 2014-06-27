#include <iosfwd>
#include <sstream>

#include "Logger.h"



Logger::NullStream Logger::_nullStream;

Logger::Logger(Logger::LogingLevel level, std::ostream& logStream) : _logLevel(level), _loggingStream(logStream) {
}

Logger::~Logger() {
}

void Logger::setLogingLevel(Logger::LogingLevel level) {
    _logLevel = level;
}

Logger::LogingLevel Logger::getLogingLevel() const {
    return _logLevel;
}

std::ostream &operator<<(std::ostream &out, const Logger &loger) {
    out << "Current debug level: " << loger._logLevel;
    return out;
}

std::ostream& Logger::operator()(Logger::LogingLevel level) {
    if (_logLevel >= level && level != NO_LOGGING)
        return _loggingStream;
    else
        return _nullStream;
}

std::ostream& Logger::operator()() {
    return _loggingStream;
}



const Logger::LogingLevel Logger::NO_LOGGING(0);
const Logger::LogingLevel Logger::ERROR(1);
const Logger::LogingLevel Logger::WARNING(2);
const Logger::LogingLevel Logger::INFO(3);
const Logger::LogingLevel Logger::DETAIL(4);
const Logger::LogingLevel Logger::FULL_DETAIL(5);

Logger::LogingLevel::LogingLevel(unsigned int level) : _level(level) {
}

std::ostream& operator<<(std::ostream &out, const Logger::LogingLevel &level) {
    std::string str(level);
    out << str;
    return out;

}

Logger::LogingLevel::operator unsigned int () const {
    return _level;
}

Logger::LogingLevel::operator std::string() const {
    return asString();
}

std::string Logger::LogingLevel::asString() const {
    std::ostringstream stream;
    if (_level == Logger::NO_LOGGING) {
        stream << "NO_LOGGING (";
    } else if (_level == Logger::ERROR) {
        stream << "ERROR (";
    } else if (_level == Logger::WARNING) {
        stream << "WARINING (";
    } else if (_level == Logger::INFO) {
        stream << "INFO (";
    } else if (_level == Logger::DETAIL) {
        stream << "DETAIL (";
    } else if (_level == Logger::FULL_DETAIL) {
        stream << "FULL_DETAIL (";
    }
    stream << _level << ")";
    return stream.str();
}


/* 
 * File:   Logger.h
 * Author: tsk
 *
 * Created on June 26, 2014, 4:51 PM
 */

#ifndef LOGGER_H
#define	LOGGER_H

#include <iostream>

namespace mtca4u {

  /**
   * Logger class which provides logging feature with 5 different logging levels.
   * Example of usage:
   *  
\code
  
 int main(int argc, char** argv[]) {

    using namespace mtca4u; // Logger object are encapsulated into 'mtca4u' name space.
 
    Logger logger; // CREATE OBJECT WIT DEFAULT SETTINGS - logging level to NO_LOGGING, logging stream set to std::cout
                   // can be created also with: Logger logger(Logger::FULL_DETAIL, std::cout);
    logger.setLogLevel(Logger::FULL_DETAIL);  // set logging level to FULL_DETAIL (all messages, except NO_LOGGING, will be logged )
    std::cout << logger << std::endl;
    logger(Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl;    // WILL NOT BE LOGGED
    logger(Logger::ERROR) << " ERROR message" << std::endl;              // WILL BE LOGGED
    logger(Logger::WARNING) << " WARNING message" << std::endl;          // WILL BE LOGGED
    logger(Logger::INFO) << " INFO message" << std::endl;                // WILL BE LOGGED
    logger(Logger::DETAIL) << " DETAIL message" << std::endl;            // WILL BE LOGGED
    logger(Logger::FULL_DETAIL) << " FULL_DETAIL message" << std::endl;  // WILL BE LOGGED
 
    logger.setLogLevel(Logger::INFO);// set logging level to INFO ( ERROR, WARINING and INFO messages will be logged )
    std::cout << logger << std::endl;
    logger() << " EMPTY FUNCTOR message" << std::endl;                   // WILL BE LOGGED
    logger(Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl;    // WILL NOT BE LOGGED
    logger(Logger::ERROR) << " ERROR message" << std::endl;              // WILL BE LOGGED
    logger(Logger::WARNING) << " WARNING message" << std::endl;          // WILL BE LOGGED
    logger(Logger::INFO) << " INFO message" << std::endl;                // WILL BE LOGGED
    logger(Logger::DETAIL) << " DETAIL message" << std::endl;            // WILL NOT BE LOGGED
    logger(Logger::FULL_DETAIL) << " FULL_DETAIL message" << std::endl;  // WILL NOT BE LOGGED
  
  
    logger.setLogLevel(Logger::NO_LOGGING); // set logging level to NO_LOGGING ( no messages will be logged, except of this with NO logging level - logger() << "EXAMPLE" << std::endl )
    std::cout << logger << std::endl;
    logger() << " EMPTY FUNCTOR message" << std::endl;                   // WILL BE LOGGED
    logger(Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl;    // WILL NOT BE LOGGED
    logger(Logger::ERROR) << " ERROR message" << std::endl;              // WILL NOT BE LOGGED
    logger(Logger::WARNING) << " WARNING message" << std::endl;          // WILL NOT BE LOGGED
    logger(Logger::INFO) << " INFO message" << std::endl;                // WILL NOT BE LOGGED
    logger(Logger::DETAIL) << " DETAIL message" << std::endl;            // WILL NOT BE LOGGED
    logger(Logger::FULL_DETAIL) << " FULL_DETAIL message" << std::endl;  // WILL NOT BE LOGGED
        
    return 0;
 }
\endcode
  */

    class Logger {
    public:

        /**
         * Helper class which represent logging level. User cannot create object of this class. 
         */
        class LogLevel {
        public:
            friend class Logger;
            friend std::ostream &operator<<(std::ostream &out, const LogLevel &level);
            operator unsigned int () const;
            operator std::string() const;
            bool operator==(LogLevel const& right) const;
            bool operator!=(LogLevel const& right) const;
        private:
            LogLevel(unsigned int level);
            std::string asString() const;

            unsigned int _level;
        };
        /**
         * Available logging levels created as a constant static fields. They are used to mark logging level of messages.
         * The FULL_DETAIL level is highest and the NO_LOGGING is lowest.
         * \li When logging level of Logger class object is set to FULL_DETAIL (highest), all messages are logged (except of ones marked as NO_LOGGING).
         * \li When logging level of Logger class object is set to DETAIL, messages marked as ERROR, WARNING, INFO, DETAIL are logged.
         * \li When logging level of Logger class object is set to INFO, messages marked as ERROR, WARNING, INFO are logged.
         * \li When logging level of Logger class object is set to WARNING, messages marked as ERROR, WARNING are logged.
         * \li When logging level of Logger class object is set to ERROR, messages marked as ERROR are logged.
         * \li When logging level of Logger class object is set to NO_LOGGING, no messages are logged (even marked as NO_LOGGING)
         */
        const static Logger::LogLevel NO_LOGGING;
        const static Logger::LogLevel ERROR;
        const static Logger::LogLevel WARNING;
        const static Logger::LogLevel INFO;
        const static Logger::LogLevel DETAIL;
        const static Logger::LogLevel FULL_DETAIL;


    public:
        /**
         * Constructor of the class object.
         * @param  level - logging level for the object. Default value: NO_LOGGING
         * @param  logStream - reference to debug stream where all log are written into. Default value: std::cout
         */
        Logger(Logger::LogLevel level = NO_LOGGING, std::ostream& logStream = std::cout);

        /**
         * Destructor of the class object.
         */
        ~Logger();

        /**
         * Sets the logging level for class object.
         * @param level - debug level for the object. Default value: NO_LOGGING
         */
        void setLogLevel(Logger::LogLevel level);


        /**
         * Gets current logging level of the class object.
         * @return Logger::LogLevel - current debug level
         */
        Logger::LogLevel getLogLevel() const;

        /**
         * Functor which allows to is object of class like standard stream object. Message pass thru 
         * @param  level - level of the log. If the level is higher then current logging level of object the message will be printed to log stream 
         */
        std::ostream& operator()(Logger::LogLevel level);

        /**
         * Functor which allows to print messages regardless of current logging level set in object.
         * It means log message will be ALLWAYS pass to the logging steam
         */
        std::ostream& operator()();

        /**
         * Which allows to pass Logger object to the std::ostream stream
         */
        friend std::ostream &operator<<(std::ostream &out, const Logger &logger);

    private:

        class NullStream : public std::ostream {
        public:
            NullStream() : std::ios(0), std::ostream(0) {
            };
        };
        static NullStream _nullStream;


        Logger::LogLevel _logLevel;
        std::ostream& _loggingStream;

    };

    extern Logger _logstdout__;
    extern Logger _logstderr__;
    
}

#endif	/* LOGGER_H */


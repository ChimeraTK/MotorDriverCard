/* 
 * File:   Logger.h
 * Author: tsk
 *
 * Created on June 26, 2014, 4:51 PM
 */

#ifndef GENERALLOGGER_H
#define	GENERALLOGGER_H

#include <iostream>
/**
 * Logger class which provides logging feature with 5 different logging levels.
 * Example of usage:
 *  
\code
    Logger loger;
    loger.setLogingLevel(Logger::FULL_DETAIL);  // set logging level to FULL_DETAIL (all messages, except NO_LOGGING, will be logged )
    std::cout << loger << std::endl;
    loger(Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl;    // WILL NOT BE LOGGED
    loger(Logger::ERROR) << std::endl;                                  // WILL BE LOGGED
    loger(Logger::WARNING) << " WARNING message" << std::endl;          // WILL BE LOGGED
    loger(Logger::INFO) << " INFO message" << std::endl;                // WILL BE LOGGED
    loger(Logger::DETAIL) << " DETAIL message" << std::endl;            // WILL BE LOGGED
    loger(Logger::FULL_DETAIL) << " FULL_DETAIL message" << std::endl;  // WILL BE LOGGED
 
    loger.setLogingLevel(Logger::INFO);// set logging level to INFO ( ERROR, WARINING and INFO messages will be logged )
    std::cout << loger << std::endl;
    loger() << " EMPTY FUNCTOR message" << std::endl;                   // WILL BE LOGGED
    loger(Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl;    // WILL NOT BE LOGGED
    loger(Logger::ERROR) << " ERROR message" << std::endl;              // WILL BE LOGGED
    loger(Logger::WARNING) << " WARNING message" << std::endl;          // WILL BE LOGGED
    loger(Logger::INFO) << " INFO message" << std::endl;                // WILL BE LOGGED
    loger(Logger::DETAIL) << " DETAIL message" << std::endl;            // WILL NOT BE LOGGED
    loger(Logger::FULL_DETAIL) << " FULL_DETAIL message" << std::endl;  // WILL NOT BE LOGGED
  
  
    loger.setLogingLevel(Logger::NO_LOGGING); // set logging level to NO_LOGGING ( no messages will be logged, except of this with NO logging level - loger() << "EXAMPLE" << std::endl )
    std::cout << loger << std::endl;
    loger() << " EMPTY FUNCTOR message" << std::endl;                   // WILL BE LOGGED
    loger(Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl;    // WILL NOT BE LOGGED
    loger(Logger::ERROR) << " ERROR message" << std::endl;              // WILL NOT BE LOGGED
    loger(Logger::WARNING) << " WARNING message" << std::endl;          // WILL NOT BE LOGGED
    loger(Logger::INFO) << " INFO message" << std::endl;                // WILL NOT BE LOGGED
    loger(Logger::DETAIL) << " DETAIL message" << std::endl;            // WILL NOT BE LOGGED
    loger(Logger::FULL_DETAIL) << " FULL_DETAIL message" << std::endl;  // WILL NOT BE LOGGED
\endcode
 */
class Logger {
public:

    /**
     * Helper class which represent logging level. User cannot create object of this class. 
     */
    class LogingLevel {
    public:
        friend class Logger;
        friend std::ostream &operator<<(std::ostream &out, const LogingLevel &level);
        operator unsigned int () const;
        operator std::string() const;
    private:
        LogingLevel(unsigned int level);
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
    const static Logger::LogingLevel NO_LOGGING;
    const static Logger::LogingLevel ERROR;
    const static Logger::LogingLevel WARNING;
    const static Logger::LogingLevel INFO;
    const static Logger::LogingLevel DETAIL;
    const static Logger::LogingLevel FULL_DETAIL;


public:
    /**
     * Constructor of the class object.
     * @param  Logger::LogingLevel level - logging level for the object. Default value: NO_LOGGING
     * @param   std::ostream& debugStream - reference to debug stream where all log are written into. Default value: std::cout
     * @return
     */
    Logger(Logger::LogingLevel level = NO_LOGGING, std::ostream& logStream = std::cout);

    /**
     * Destructor of the class object.
     */
    ~Logger();

    /**
     * Sets the logging level for class object.
     * @param  Logger::LogingLevel level - debug level for the object. Default value: NO_LOGGING
     * @return
     */
    void setLogingLevel(Logger::LogingLevel level);


    /**
     * Gets current logging level of the class object.
     * @param  
     * @return Logger::LogingLevel - current debug level
     */
    Logger::LogingLevel getLogingLevel() const;

    /**
     * Functor which allows to is object of class like standard stream object. Message pass thru 
     * @param  Logger::LogingLevel level - level of the log. If the level is higher then current logging level of object the message will be printed to log stream 
     * @return 
     */
    std::ostream& operator()(Logger::LogingLevel level);
    
    /**
     * Functor which allows to is object of class like standard stream object.
     * @param 
     * @return 
     */
    std::ostream& operator()();

    /**
     * Operator << which allows to pass Logger object to the std::ostream stream
     */
    friend std::ostream &operator<<(std::ostream &out, const Logger &loger);

private:

    class NullStream : public std::ostream {
    public:

        NullStream() : std::ios(0), std::ostream(0) {
        };
    };
    static NullStream _nullStream;


    Logger::LogingLevel _logLevel;
    std::ostream& _loggingStream;

};


#endif	/* GENERALLOGGER_H */


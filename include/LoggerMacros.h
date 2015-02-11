/* 
 * File:   LoggerMacros.h
 * Author: tsk
 *
 * Created on June 28, 2014, 11:39 AM
 */

#ifndef LOGGERMACROS_H
#define	LOGGERMACROS_H

#include "Logger.h"

#define STD_LOGGER mtca4u::_logstdout__
#define ERR_LOGGER mtca4u::_logstderr__

#define STD_LOGGER_LOG_ALWAYS mtca4u::_logstdout__()
#define ERR_LOGGER_LOG_ALWAYS mtca4u::_logstderr__()

#define STD_LOGGER_LOG_LEVEL(LOG_LEVEL) mtca4u::_logstdout__(LOG_LEVEL)
#define ERR_LOGGER_LOG_LEVEL(LOG_LEVEL) mtca4u::_logstderr__(LOG_LEVEL)

#define STD_LOGGER_SET_LOG_LEVEL(LOG_LEVEL) mtca4u::_logstdout__.setLogLevel(LOG_LEVEL);    
#define ERR_LOGGER_SET_LOG_LEVEL(LOG_LEVEL) mtca4u::_logstderr__.setLogLevel(LOG_LEVEL);

#define STD_LOGGER_GET_LOG_LEVEL mtca4u::_logstdout__.getLogLevel();    
#define ERR_LOGGER_GET_LOG_LEVEL mtca4u::_logstderr__.getLogLevel();


#define STD_LOGGER_FL(MESSAGE_LEVEL)  mtca4u::_logstdout__(MESSAGE_LEVEL) << __FILE__ << " - Line: " <<__LINE__ << " -> "; mtca4u::_logstdout__(MESSAGE_LEVEL) 

#define ERR_LOGGER_FL(MESSAGE_LEVEL)  mtca4u::_logstderr__(MESSAGE_LEVEL) << __FILE__ << " - Line: " <<__LINE__ << " -> "; mtca4u::_logstderr__(MESSAGE_LEVEL) 

#define STD_LOGGER_FLD(MESSAGE_LEVEL) mtca4u::_logstdout__(MESSAGE_LEVEL) << __FILE__ << " - Line: " <<__LINE__  << " Date: " << __DATE__ << " " << __TIME__\
                                          << " -> "; mtca4u::_logstdout__(MESSAGE_LEVEL) 

#define ERR_LOGGER_FLD(MESSAGE_LEVEL) mtca4u::_logstderr__(MESSAGE_LEVEL) << __FILE__ << " - Line: " <<__LINE__  << " Date: " << __DATE__ << " " << __TIME__\
                                          << " -> "; mtca4u::_logstderr__(MESSAGE_LEVEL) 

#define STD_LOGGER_D(MESSAGE_LEVEL)   mtca4u::_logstdout__(MESSAGE_LEVEL) << " Date: " << __DATE__ << " " << __TIME__\
                                          << " -> "; mtca4u::_logstdout__(MESSAGE_LEVEL) 

#define ERR_LOGGER_D(MESSAGE_LEVEL)   mtca4u::_logstderr__(MESSAGE_LEVEL) << " Date: " << __DATE__ << " " << __TIME__\
                                          << " -> "; mtca4u::_logstderr__(MESSAGE_LEVEL)   



#endif	/* LOGGERMACROS_H */


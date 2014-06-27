#include "Logger.h"

int main(int argc, char* argv[]) {

    std::cout << "Test Logger class !!! : Number of params: " << argc << "\n";

    std::cout << "START OF GENERALLOGER TEST !!!!" << std::endl;
    Logger loger;
    loger.setLogingLevel(Logger::FULL_DETAIL);
    std::cout << loger << std::endl;
    loger() << " EMPTY FUNCTOR message" << std::endl;
    loger(Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl;
    loger(Logger::ERROR) << " ERROR message" << std::endl;
    loger(Logger::WARNING) << " WARNING message" << std::endl;
    loger(Logger::INFO) << " INFO message" << std::endl;
    loger(Logger::DETAIL) << " DETAIL message" << std::endl;
    loger(Logger::FULL_DETAIL) << " FULL_DETAIL message" << std::endl;
    loger.setLogingLevel(Logger::DETAIL);
    std::cout << loger << std::endl;
    loger() << " EMPTY FUNCTOR message" << std::endl;
    loger(Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl;
    loger(Logger::ERROR) << " ERROR message" << std::endl;
    loger(Logger::WARNING) << " WARNING message" << std::endl;
    loger(Logger::INFO) << " INFO message" << std::endl;
    loger(Logger::DETAIL) << " DETAIL message" << std::endl;
    loger(Logger::FULL_DETAIL) << " FULL_DETAIL message" << std::endl;
    loger.setLogingLevel(Logger::INFO);
    std::cout << loger << std::endl;
    loger() << " EMPTY FUNCTOR message" << std::endl;
    loger(Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl;
    loger(Logger::ERROR) << " ERROR message" << std::endl;
    loger(Logger::WARNING) << " WARNING message" << std::endl;
    loger(Logger::INFO) << " INFO message" << std::endl;
    loger(Logger::DETAIL) << " DETAIL message" << std::endl;
    loger(Logger::FULL_DETAIL) << " FULL_DETAIL message" << std::endl;
    loger.setLogingLevel(Logger::WARNING);
    std::cout << loger << std::endl;
    loger() << " EMPTY FUNCTOR message" << std::endl;
    loger(Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl;
    loger(Logger::ERROR) << " ERROR message" << std::endl;
    loger(Logger::WARNING) << " WARNING message" << std::endl;
    loger(Logger::INFO) << " INFO message" << std::endl;
    loger(Logger::DETAIL) << " DETAIL message" << std::endl;
    loger(Logger::FULL_DETAIL) << " FULL_DETAIL message" << std::endl;
    loger.setLogingLevel(Logger::ERROR);
    std::cout << loger << std::endl;
    loger() << " EMPTY FUNCTOR message" << std::endl;
    loger(Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl;
    loger(Logger::ERROR) << " ERROR message" << std::endl;
    loger(Logger::WARNING) << " WARNING message" << std::endl;
    loger(Logger::INFO) << " INFO message" << std::endl;
    loger(Logger::DETAIL) << " DETAIL message" << std::endl;
    loger(Logger::FULL_DETAIL) << " FULL_DETAIL message" << std::endl;
    loger.setLogingLevel(Logger::NO_LOGGING);
    std::cout << loger << std::endl;
    loger() << " EMPTY FUNCTOR message" << std::endl;
    loger(Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl;
    loger(Logger::ERROR) << " ERROR message" << std::endl;
    loger(Logger::WARNING) << " WARNING message" << std::endl;
    loger(Logger::INFO) << " INFO message" << std::endl;
    loger(Logger::DETAIL) << " DETAIL message" << std::endl;
    loger(Logger::FULL_DETAIL) << " FULL_DETAIL message" << std::endl;
    std::cout << "END OF GENERALLOGER TEST !!!!" << std::endl;
    std::cout << "End of Test Logger class !!! \n";
    return 0;
}


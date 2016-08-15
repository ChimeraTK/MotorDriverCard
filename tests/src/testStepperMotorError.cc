#define BOOST_TEST_MODULE StepperMotorErrorTest
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test_framework;

#include "StepperMotorError.h"

bool checkOperatorForDefaultConstructedObject(mtca4u::StepperMotorError& error) {
    switch (error) {
        case 1:
            return true;
            break;
        default:
            return false;
    }
}


BOOST_AUTO_TEST_SUITE( StepperMotorErrorTestSuite )
        
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE( StepperMotorErrorTestCase ) {
#pragma GCC diagnostic pop
    
    std::stringstream testStream; // any kind of object which is std::ostream
    
    //test: dafault constuctor
    mtca4u::StepperMotorError defaultError;
    //test: parameter constuctor
    mtca4u::StepperMotorError error5(5);
    //test: copy constructor
    mtca4u::StepperMotorError defaultErrorCopy1(defaultError);
    //test: operator=
    mtca4u::StepperMotorError defaultErrorCopy2;
    defaultErrorCopy2 = defaultError;
    defaultError = defaultError;
    
    //test: operator int() const
    BOOST_CHECK ( checkOperatorForDefaultConstructedObject(defaultError) );
    BOOST_CHECK ( checkOperatorForDefaultConstructedObject(defaultErrorCopy1) );
    BOOST_CHECK ( checkOperatorForDefaultConstructedObject(defaultErrorCopy2) );
    
    
    //test: getId
    BOOST_CHECK ( defaultError.getId() == 1 );
    BOOST_CHECK ( defaultErrorCopy1.getId() == 1 );
    BOOST_CHECK ( defaultErrorCopy2.getId() == 1 );
    BOOST_CHECK ( error5.getId() == 5 );
    
    //test: operator==
    BOOST_CHECK ( defaultError == defaultErrorCopy1 );
    BOOST_CHECK ( defaultError == defaultErrorCopy2 );
    BOOST_CHECK ( defaultErrorCopy1 == defaultErrorCopy2 );
    
    //test: operator==
    BOOST_CHECK ( (defaultError == error5) == false );
    
    //test: operator!=
    BOOST_CHECK( defaultError != error5);
    BOOST_CHECK( (defaultError != defaultErrorCopy1) == false);
    
    //test: operator<<
    mtca4u::StepperMotorError errorNO_ERROR(mtca4u::StepperMotorErrorTypes::M_NO_ERROR);
    testStream << errorNO_ERROR;
    BOOST_CHECK( testStream.str().compare("Motor is NOT IN ERROR (1)") == 0 );        
    
    testStream.str(std::string());
    testStream.clear();
    mtca4u::StepperMotorError statusCOMMUNICATION_LOST(mtca4u::StepperMotorErrorTypes::M_COMMUNICATION_LOST);
    testStream << statusCOMMUNICATION_LOST;
    BOOST_CHECK( testStream.str().compare("Motor lost communication with hardware (2)") == 0 );   
    
    testStream.str(std::string());
    testStream.clear();
    mtca4u::StepperMotorError statusCONFIGURATION_ERROR(mtca4u::StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX);
    testStream << statusCONFIGURATION_ERROR;
    BOOST_CHECK( testStream.str().compare("Error in configuration. Minimal software limit is greater than maximum software limit (4)") == 0 );  

    testStream.str(std::string());
    testStream.clear();
    mtca4u::StepperMotorError errorNO_REACTION_ON_COMMAND(mtca4u::StepperMotorErrorTypes::M_NO_REACTION_ON_COMMAND);
    testStream << errorNO_REACTION_ON_COMMAND;
    BOOST_CHECK(testStream.str().compare("Motor is not reacting of last command (8)") == 0); 

    testStream.str(std::string());
    testStream.clear();
    mtca4u::StepperMotorError errorHARDWARE_NOT_CONNECTED(mtca4u::StepperMotorErrorTypes::M_HARDWARE_NOT_CONNECTED);
    testStream << errorHARDWARE_NOT_CONNECTED;
    BOOST_CHECK(testStream.str().compare("Hardware seems to be not connected. Check your hardware (16)") == 0); 

    testStream.str(std::string());
    testStream.clear();
    mtca4u::StepperMotorError randomError(123456);
    testStream << randomError;
    BOOST_CHECK( testStream.str().compare("No name available in GeneralStatus object. Status id: (123456)") == 0 ); 
    
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE( LinearStepperMotorErrorTestCase ) {
#pragma GCC diagnostic pop
    
     
    std::stringstream testStream; // any kind of object which is std::ostream
    
    //test: dafault constuctor
    mtca4u::LinearStepperMotorError defaultError;
    //test: parameter constuctor
    mtca4u::LinearStepperMotorError error5(5);
    //test: copy constructor
    mtca4u::LinearStepperMotorError defaultErrorCopy1(defaultError);
    //test: operator=
    mtca4u::LinearStepperMotorError defaultErrorCopy2;
    defaultErrorCopy2 = defaultError;
    defaultError = defaultError;
    
    //test: operator int() const
    BOOST_CHECK ( checkOperatorForDefaultConstructedObject(defaultError) );
    BOOST_CHECK ( checkOperatorForDefaultConstructedObject(defaultErrorCopy1) );
    BOOST_CHECK ( checkOperatorForDefaultConstructedObject(defaultErrorCopy2) );
    
    
    //test: getId
    BOOST_CHECK ( defaultError.getId() == 1 );
    BOOST_CHECK ( defaultErrorCopy1.getId() == 1 );
    BOOST_CHECK ( defaultErrorCopy2.getId() == 1 );
    BOOST_CHECK ( error5.getId() == 5 );
    
    //test: operator==
    BOOST_CHECK ( defaultError == defaultErrorCopy1 );
    BOOST_CHECK ( defaultError == defaultErrorCopy2 );
    BOOST_CHECK ( defaultErrorCopy1 == defaultErrorCopy2 );
    
    //test: operator==
    BOOST_CHECK ( (defaultError == error5) == false );
    
    //test: operator!=
    BOOST_CHECK( defaultError != error5);
    BOOST_CHECK( (defaultError != defaultErrorCopy1) == false);
    
    //test: operator<<
    mtca4u::LinearStepperMotorError errorNO_ERROR(mtca4u::LinearStepperMotorErrorTypes::M_NO_ERROR);
    testStream << errorNO_ERROR;
    BOOST_CHECK( testStream.str().compare("Motor is NOT IN ERROR (1)") == 0 );        
    
    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorError statusCOMMUNICATION_LOST(mtca4u::LinearStepperMotorErrorTypes::M_COMMUNICATION_LOST);
    testStream << statusCOMMUNICATION_LOST;
    BOOST_CHECK( testStream.str().compare("Motor lost communication with hardware (2)") == 0 );   
    
    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorError statusCONFIGURATION_ERROR(mtca4u::LinearStepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX);
    testStream << statusCONFIGURATION_ERROR;
    BOOST_CHECK( testStream.str().compare("Error in configuration. Minimal software limit is greater than maximum software limit (4)") == 0 );  

    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorError errorNO_REACTION_ON_COMMAND(mtca4u::LinearStepperMotorErrorTypes::M_NO_REACTION_ON_COMMAND);
    testStream << errorNO_REACTION_ON_COMMAND;
    BOOST_CHECK(testStream.str().compare("Motor is not reacting of last command (8)") == 0); 

    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorError errorHARDWARE_NOT_CONNECTED(mtca4u::LinearStepperMotorErrorTypes::M_HARDWARE_NOT_CONNECTED);
    testStream << errorHARDWARE_NOT_CONNECTED;
    BOOST_CHECK(testStream.str().compare("Hardware seems to be not connected. Check your hardware (16)") == 0);

    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorError errorBOTH_END_SWITCHES_ON(mtca4u::LinearStepperMotorErrorTypes::M_BOTH_END_SWITCH_ON);
    testStream << errorBOTH_END_SWITCHES_ON;
    BOOST_CHECK(testStream.str().compare("Both hardware end switches on. Check your hardware (32)") == 0);
    
    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorError radnomError(123456);
    testStream << radnomError;
    BOOST_CHECK( testStream.str().compare("No name available in GeneralStatus object. Status id: (123456)") == 0 ); 
}

BOOST_AUTO_TEST_SUITE_END()

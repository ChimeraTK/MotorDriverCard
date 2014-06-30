#define BOOST_TEST_MODULE StepperMotorStatusTest
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test_framework;

#include "StepperMotorStatus.h"

bool checkOperatorForDefaultConstructedObject(mtca4u::StepperMotorStatus& status) {
    switch (status) {
        case 1:
            return true;
            break;
        default:
            return false;
    }
}


BOOST_AUTO_TEST_SUITE( StepperMotorStatusTestSuite )
        
BOOST_AUTO_TEST_CASE( StepperMotorStatusTestCase ) {
    
    std::stringstream testStream; // any kind of object which is std::ostream
    
    //test: dafault constuctor
    mtca4u::StepperMotorStatus defaultStatus;
    //test: parameter constuctor
    mtca4u::StepperMotorStatus status2(2);
    //test: copy constructor
    mtca4u::StepperMotorStatus defaultStatusCopy1(defaultStatus);
    //test: operator=
    mtca4u::StepperMotorStatus defaultStatusCopy2;
    defaultStatusCopy2 = defaultStatus;
    defaultStatus = defaultStatus;
    
    //test: operator int() const
    BOOST_CHECK ( checkOperatorForDefaultConstructedObject(defaultStatus) );
    
    //test: getId
    BOOST_CHECK ( defaultStatus.getId() == 1 );
    BOOST_CHECK ( defaultStatusCopy1.getId() == 1 );
    BOOST_CHECK ( defaultStatusCopy2.getId() == 1 );
    BOOST_CHECK ( status2.getId() == 2 );
    
    //test: operator==
    BOOST_CHECK ( defaultStatus == defaultStatusCopy1 );
    BOOST_CHECK ( defaultStatus == defaultStatusCopy2 );
    BOOST_CHECK ( defaultStatusCopy1 == defaultStatusCopy2 );
    
    //test: operator==
    BOOST_CHECK ( (defaultStatus == status2) == false );
    
    //test: operator!=
    BOOST_CHECK( defaultStatus != status2);
    
    BOOST_CHECK( (defaultStatus != defaultStatusCopy1) == false);
    
    //test: operator<<
    mtca4u::StepperMotorStatus statusOK(mtca4u::StepperMotorStatusTypes::M_OK);
    testStream << statusOK;
    BOOST_CHECK( testStream.str().compare("Motor OK (1)") == 0 );        
    
    testStream.str(std::string());
    testStream.clear();
    mtca4u::StepperMotorStatus statusDISABLED(mtca4u::StepperMotorStatusTypes::M_DISABLED);
    testStream << statusDISABLED;
    BOOST_CHECK( testStream.str().compare("Motor DISABLED (2)") == 0 );   
    
    testStream.str(std::string());
    testStream.clear();
    mtca4u::StepperMotorStatus statusIN_MOVE(mtca4u::StepperMotorStatusTypes::M_IN_MOVE);
    testStream << statusIN_MOVE;
    BOOST_CHECK( testStream.str().compare("Motor IN MOVE (4)") == 0 );  

    testStream.str(std::string());
    testStream.clear();
    mtca4u::StepperMotorStatus statusNOT_IN_POSITION(mtca4u::StepperMotorStatusTypes::M_NOT_IN_POSITION);
    testStream << statusNOT_IN_POSITION;
    BOOST_CHECK(testStream.str().compare("Motor NOT IN POSITION (8)") == 0); 

    testStream.str(std::string());
    testStream.clear();
    mtca4u::StepperMotorStatus statusSOFT_POSITIVE_END_SWITCH_ON(mtca4u::StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON);
    testStream << statusSOFT_POSITIVE_END_SWITCH_ON;
    BOOST_CHECK(testStream.str().compare("Motor SOFTWARE POSITIVE POSITION LIMIT CROSSED (16)") == 0); 

    testStream.str(std::string());
    testStream.clear();
    mtca4u::StepperMotorStatus statusSOFT_NEGATIVE_END_SWITCH_ON(mtca4u::StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON);
    testStream << statusSOFT_NEGATIVE_END_SWITCH_ON;
    BOOST_CHECK(testStream.str().compare("Motor SOFTWARE NEGATIVE POSITION LIMIT CROSSED (32)") == 0); 

    testStream.str(std::string());
    testStream.clear();
    mtca4u::StepperMotorStatus statusERROR(mtca4u::StepperMotorStatusTypes::M_ERROR);
    testStream << statusERROR;
    BOOST_CHECK(testStream.str().compare("Motor IN ERROR (64)") == 0); 
    
    testStream.str(std::string());
    testStream.clear();
    mtca4u::StepperMotorStatus statusRandom(123456);
    testStream << statusRandom;
    BOOST_CHECK( testStream.str().compare("No name available in GeneralStatus object. Status id: (123456)") == 0 ); 
    
}

bool checkOperatorForDefaultConstructedObject(mtca4u::LinearStepperMotorStatus& status) {
    switch (status) {
        case 1:
            return true;
            break;
        default:
            return false;
    }
}

BOOST_AUTO_TEST_CASE( LinearStepperMotorStatusTestCase ) {
    
    std::stringstream testStream; // any kind of object which is std::ostream
    
    //test: dafault constuctor
    mtca4u::LinearStepperMotorStatus defaultStatus;
    //test: parameter constuctor
    mtca4u::LinearStepperMotorStatus status2(2);
    //test: copy constructor
    mtca4u::LinearStepperMotorStatus defaultStatusCopy1(defaultStatus);
    //test: operator=
    mtca4u::LinearStepperMotorStatus defaultStatusCopy2;
    defaultStatusCopy2 = defaultStatus;
    defaultStatus = defaultStatus;
    
    //test: operator int() const
    BOOST_CHECK ( checkOperatorForDefaultConstructedObject(defaultStatus) );
    
    //test: getId
    BOOST_CHECK ( defaultStatus.getId() == 1 );
    BOOST_CHECK ( defaultStatusCopy1.getId() == 1 );
    BOOST_CHECK ( defaultStatusCopy2.getId() == 1 );
    BOOST_CHECK ( status2.getId() == 2 );
    
    //test: operator==
    BOOST_CHECK ( defaultStatus == defaultStatusCopy1 );
    BOOST_CHECK ( defaultStatus == defaultStatusCopy2 );
    BOOST_CHECK ( defaultStatusCopy1 == defaultStatusCopy2 );
    
    //test: operator==
    BOOST_CHECK ( (defaultStatus == status2) == false );
    
    //test: operator!=
    BOOST_CHECK( defaultStatus != status2);
    
    BOOST_CHECK( (defaultStatus != defaultStatusCopy1) == false);
    
    //test: operator<<
    mtca4u::LinearStepperMotorStatus statusOK(mtca4u::LinearStepperMotorStatusTypes::M_OK);
    testStream << statusOK;
    BOOST_CHECK( testStream.str().compare("Motor OK (1)") == 0 );        
    
    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorStatus statusDISABLED(mtca4u::LinearStepperMotorStatusTypes::M_DISABLED);
    testStream << statusDISABLED;
    BOOST_CHECK( testStream.str().compare("Motor DISABLED (2)") == 0 );   
    
    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorStatus statusINMOVE(mtca4u::LinearStepperMotorStatusTypes::M_IN_MOVE);
    testStream << statusINMOVE;
    BOOST_CHECK( testStream.str().compare("Motor IN MOVE (4)") == 0 );  

    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorStatus statusNOT_IN_POSITION(mtca4u::LinearStepperMotorStatusTypes::M_NOT_IN_POSITION);
    testStream << statusNOT_IN_POSITION;
    BOOST_CHECK(testStream.str().compare("Motor NOT IN POSITION (8)") == 0); 

    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorStatus statusSOFT_POSITIVE_END_SWITCH_ON(mtca4u::LinearStepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON);
    testStream << statusSOFT_POSITIVE_END_SWITCH_ON;
    BOOST_CHECK(testStream.str().compare("Motor SOFTWARE POSITIVE POSITION LIMIT CROSSED (16)") == 0); 

    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorStatus statusSOFT_NEGATIVE_END_SWITCH_ON(mtca4u::LinearStepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON);
    testStream << statusSOFT_NEGATIVE_END_SWITCH_ON;
    BOOST_CHECK(testStream.str().compare("Motor SOFTWARE NEGATIVE POSITION LIMIT CROSSED (32)") == 0); 

    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorStatus statusERROR(mtca4u::LinearStepperMotorStatusTypes::M_ERROR);
    testStream << statusERROR;
    BOOST_CHECK(testStream.str().compare("Motor IN ERROR (64)") == 0); 
   
    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorStatus statusHARDWARE_POSITIVE_END_SWITCH_ON(mtca4u::LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON);
    testStream << statusHARDWARE_POSITIVE_END_SWITCH_ON;
    BOOST_CHECK(testStream.str().compare("Motor HARDWARE POSITIVE END SWITCH is ON (128)") == 0); 

    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorStatus statusHARDWARE_NEGATIVE_END_SWITCH_ON(mtca4u::LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON);
    testStream << statusHARDWARE_NEGATIVE_END_SWITCH_ON;
    BOOST_CHECK(testStream.str().compare("Motor HARDWARE NEGATIVE END SWITCH is ON (256)") == 0); 
    
    testStream.str(std::string());
    testStream.clear();
    mtca4u::LinearStepperMotorStatus statusRandom(123456);
    testStream << statusRandom;
    BOOST_CHECK( testStream.str().compare("No name available in GeneralStatus object. Status id: (123456)") == 0 ); 
    
}

BOOST_AUTO_TEST_SUITE_END()

#include "StepperMotor.h"
#include <string>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <pthread.h>



void* statusReaderThreadBody(void* ptr) {
    mtca4u::StepperMotor* motorPointer = (mtca4u::StepperMotor*)ptr;
    while (1) {
        std::cout << "Thread:: motor status is: " << motorPointer->getMotorStatus() << std::endl;
        usleep(100000);
    }
 
    pthread_exit(NULL);
}


int main( int argc, char* argv[] ) {

    std::cout << "Test MotorStepper class !!! : Number of params: " <<   argc <<  "\n";

  


    std::string deviceName;
    std::string configFileNameName;
    int motorDriverId;
    if (argc == 4 ) {
        deviceName = std::string(argv[1]);
        configFileNameName = std::string(argv[2]);
        motorDriverId = boost::lexical_cast<int>(argv[3]);
    } else {
        deviceName = std::string("DFMC-MD22");
        configFileNameName = std::string("VT21-MotorDriverCardConfig.xml");
        motorDriverId = 0;
    }
    
    std::cout << "Try to open MOTOR CARD with device name: " << deviceName << " with config file: " << configFileNameName << " Motor driver ID is: " << motorDriverId << std::endl;

    mtca4u::StepperMotor motor(deviceName, motorDriverId, configFileNameName);

    std::cout << "Change debug stream to the std::cout \n"; 
    motor.setDebugStream(&std::cout);
    motor.setDebugLevel(DEBUG_INFO);
    
    mtca4u::StepperMotorStatus status(motor.getMotorStatus());
    std::cout << "Current motor status is: " << status<< "\n";
    if (status == mtca4u::StepperMotorStatusTypes::M_DISABLED) {
        std::cout << "Enabling the motor.\n";
        motor.setEnable(true);
    }

    status = motor.getMotorStatus();
    std::cout << "Current motor status is: " << status << "\n";

    if (status == mtca4u::StepperMotorStatusTypes::M_ERROR) {
        mtca4u::StepperMotorError error(motor.getMotorError());
        std::cout << "Motor in error state. Error is: " << error << "\n";
        if (error == mtca4u::StepperMotorErrorTypes::M_BOTH_END_SWITCH_ON) {
            std::cout << "Both end switches on. Check you hardware." << error << "\n";
        }
    } else {
        std::cout << "Enabling  autostart.\n";
        motor.setAutostart(true);
        if (motor.getAutostart()) {
            std::cout << "Autostart feature is ENABLED!\n";
        } else {
            std::cout << "Autostart feature is DISABLED!\n";
        }

        std::cout << "Disabling autostart.\n";
        motor.setAutostart(false);
        if (motor.getAutostart()) {
            std::cout << "Autostart feature is ENABLED!\n";
        } else {
            std::cout << "Autostart feature is DISABLED!\n";
        }


        float currentMotorPosition = motor.getMotorPosition();
        std::cout << "Current motor position is: " << currentMotorPosition << "\n";
   
    }
    
    mtca4u::StepperMotorCalibrationStatus calibrationStatus = motor.getCalibrationStatus();
    std::cout << "Current calibration status: " << calibrationStatus << std::endl;
    
    pthread_t statusReaderThread;
    int rc = 0;
    
    std::cout << "Creating status reader thread.\n" << std::flush;
    

    rc = pthread_create(&statusReaderThread, NULL, statusReaderThreadBody, (void*)(&motor));
    
    if (rc) {
        std::cout << "Unable to create thread. Error code: " << rc << std::endl;
        exit(1);
    } 
    
    std::cout << "Calibrate motor start\n";
    calibrationStatus = motor.calibrateMotor();
    std::cout << "Calibration status after interrupted calibration: " << calibrationStatus << std::endl;
   

    for (int i = 0; i < 10; i++) {
        motor.calibrateMotor();
        sleep(1);
        if (motor.getCalibrationStatus() == mtca4u::StepperMotorCalibrationStatusType::M_CALIBRATED) {
            std::cout << "Calibration " <<i << " done with success !!!!!!!!!!!!!!!!!!!!!!!!!!!!! "   << std::endl;
        } else {
            std::cout << "Calibration " <<i << " finished with status: " << motor.getCalibrationStatus() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
            if (motor.getCalibrationStatus() == mtca4u::StepperMotorCalibrationStatusType::M_CALIBRATION_STOPED_BY_USER)
                break;
        }
    }

    std::cout << "Disabling the motor\n";
    motor.setEnable(false);
    std::cout << "End of Test MotorStepper class !!! \n";
    return 0;
}


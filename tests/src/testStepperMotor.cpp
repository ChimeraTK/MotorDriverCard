#include "StepperMotor.h"
#include <string>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <pthread.h>



void* calibrationStopThread(void* ptr) {
    mtca4u::StepperMotor* motorPointer = (mtca4u::StepperMotor*)ptr;
    for (int i = 0; i<6; i++) {
        std::cout << "Stoping thread waiting: " << i << std::endl <<std::flush;
        if (i%2 == 0) {
            mtca4u::StepperMotorCalibrationStatus calibrationStatus = motorPointer->getCalibrationStatus();
            std::cout << "Calibration status during waiting for interruption: " << calibrationStatus << std::endl;
        }
        sleep(1);
    }
    
    motorPointer->stopMotor();
 
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
    motor.setDebugLevel(DEBUG_DETAIL);
    
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
        if (error == mtca4u::StepperMotorErrorTypes::MOTOR_BOTH_ENDSWICHTED_ON) {
            std::cout << "Both end switches on. Check you hardware." << error << "\n";
        }
    } else {
        std::cout << "Disabling autostart.\n";
        motor.setAutostart(true);
        if (motor.getAutostart()) {
            std::cout << "Autostart feature is ENABLED!\n";
        } else {
            std::cout << "Autostart feature is DISABLED!\n";
        }

        std::cout << "Enabling autostart.\n";
        motor.setAutostart(false);
        if (motor.getAutostart()) {
            std::cout << "Autostart feature is ENABLED!\n";
        } else {
            std::cout << "Autostart feature is DISABLED!\n";
        }


        float currentMotorPosition = motor.getMotorPosition();
        std::cout << "Current motor position is: " << currentMotorPosition << "\n";
        std::cout << "Move to position: " << currentMotorPosition+10000 << "\n";
        if (motor.moveToPosition(currentMotorPosition+10000) != mtca4u::StepperMotorStatusTypes::M_OK) {
            std::cout << "Motor movement failed. Status is: " << motor.getMotorStatus() << std::endl;
        }
        std::cout << "!!!!!!!!!!!!!!Current motor position is: " << motor.getMotorPosition() << "\n";
         usleep(100000);
        std::cout << "!!!!!!!!!!!!!!Current motor position is: " << motor.getMotorPosition() << "\n";
   
    }
    
    mtca4u::StepperMotorCalibrationStatus calibrationStatus = motor.getCalibrationStatus();
    std::cout << "Current calibration status: " << calibrationStatus << std::endl;
    
    pthread_t stopThread;
    int rc = 0;
    
    std::cout << "Creating thread which after 5 seconds will stop calibration.\n" << std::flush;
    

    rc = pthread_create(&stopThread, NULL, calibrationStopThread, (void*)(&motor));
    
    if (rc) {
        std::cout << "Unable to create thread. Error code: " << rc << std::endl; 
    } 
    
    std::cout << "Calibrate motor start\n";
    calibrationStatus = motor.calibrateMotor();
    std::cout << "Calibration status after interrupted calibration: " << calibrationStatus << std::endl;
   

    std::cout << "Calibrate motor start without interrupting.\n";
    calibrationStatus = motor.calibrateMotor();
    std::cout << "Calibration status after not interrupted calibration: " << calibrationStatus << std::endl;

    if (calibrationStatus == mtca4u::StepperMotorCalibrationStatusType::M_CALIBRATED) {
        std::cout << "Testing stopping of the motor." << std::endl;
        float newPostion = motor.getMotorPosition() + 50000;
        std::cout << "Current motor position is:" << motor.getMotorPosition() << " . Sending motor to position: " << newPostion << std::endl;
        std::cout << "Creating thread which after 5 seconds will stop motor movement.\n" << std::flush;
        rc = pthread_create(&stopThread, NULL, calibrationStopThread, (void*) (&motor));

        if (rc) {
            std::cout << "Unable to create thread. Error code: " << rc << std::endl;
        }
        motor.moveToPosition(newPostion);
        std::cout << "Motor status after stopping is: " << motor.getMotorStatus() << std::endl;

    }
    
    
    std::cout << "Disabling the motor\n";
    motor.setEnable(false);
    std::cout << "End of Test MotorStepper class !!! \n";
    return 0;
}


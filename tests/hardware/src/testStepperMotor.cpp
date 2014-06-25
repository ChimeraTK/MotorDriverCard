#include "LinearStepperMotor.h"
#include <string>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <pthread.h>



void* statusReaderThreadBody(void* ptr) {
    mtca4u::StepperMotor* motorPointer = (mtca4u::StepperMotor*)ptr;
    while (1) {
        //std::cout << "Thread:: motor status is: " << motorPointer->getStatus() << std::endl;
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

    mtca4u::LinearStepperMotor motor(deviceName, motorDriverId, configFileNameName);

    std::cout << "Change debug stream to the std::cout \n"; 
    motor.setDebugStream(&std::cout);
    motor.setDebugLevel(DEBUG_INFO);
    
    mtca4u::StepperMotorStatus status(motor.getStatus());
    std::cout << "Current motor status is: " << status<< "\n";
    if (status == mtca4u::StepperMotorStatusTypes::M_DISABLED) {
        std::cout << "Enabling the motor.\n";
        motor.setEnabled(true);
    }

    status = motor.getStatus();
    std::cout << "Current motor status is: " << status << "\n";

    

    
    if (status == mtca4u::StepperMotorStatusTypes::M_ERROR) {
        mtca4u::StepperMotorError error(motor.getError());
        std::cout << "Motor in error state. Error is: " << error << "\n";
        if (error == mtca4u::StepperMotorErrorTypes::M_BOTH_END_SWITCH_ON) {
            std::cout << "Both end switches on. Check you hardware." << error << "\n";
        }
        
        return 1;
        
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


        float currentMotorPosition = motor.getCurrentPosition();
        std::cout << "Current motor position is: " << currentMotorPosition << "\n";
   
        float newPosition = currentMotorPosition+1000000;
        std::cout << "Move motor to new position: " << newPosition << std::endl;
        motor.moveToPosition(newPosition);
        
        mtca4u::StepperMotorStatus status = motor.getStatus();
        if (status != mtca4u::LinearStepperMotorStatusTypes::M_OK) {
            std::cout << "Moving to target position failed. Current motor status is: " << status << "\n";
        }
        
        if (motor.getStatus() == mtca4u::LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON) {
            newPosition = motor.getCurrentPosition() - 1000000;
            std::cout << "Stepper Motor at positive end-switch. Move to negative direction to position: " << newPosition << std::endl;
            motor.moveToPosition(newPosition);
            status = motor.getStatus();
            if (motor.getStatus() != mtca4u::LinearStepperMotorStatusTypes::M_OK) {
                std::cout << "Moving to target position failed. Current motor status is: " << status << "\n";
            }
            
        }
    }

    //std::cout << "Disabling the motor\n";
    //motor.setEnabled(false);
    //std::cout << "End of Test MotorStepper class !!! \n";
    //return 0;
    
    mtca4u::StepperMotorCalibrationStatus calibrationStatus = motor.getCalibrationStatus();
    std::cout << "Current calibration status: " << calibrationStatus << std::endl;
    /*
    pthread_t statusReaderThread;
    int rc = 0;
    
    std::cout << "Creating status reader thread.\n" << std::flush;
    

    rc = pthread_create(&statusReaderThread, NULL, statusReaderThreadBody, (void*)(&motor));
    
    if (rc) {
        std::cout << "Unable to create thread. Error code: " << rc << std::endl;
        exit(1);
    } 
    */
    std::cout << "Calibrate motor start\n";
    calibrationStatus = motor.calibrateMotor();
    sleep(1);
    if (motor.getCalibrationStatus() == mtca4u::StepperMotorCalibrationStatusType::M_CALIBRATED) {
        std::cout << "Calibration done with success !!!!!!!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
    } else {
        std::cout << "Calibration finished with status: " << motor.getCalibrationStatus() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        //if (motor.getCalibrationStatus() == mtca4u::StepperMotorCalibrationStatusType::M_CALIBRATION_STOPED_BY_USER)
           // break;
    }

    
    std::cout << "Disabling the motor\n";
    motor.setEnabled(false);
    std::cout << "End of Test MotorStepper class !!! \n";
    return 0;
}


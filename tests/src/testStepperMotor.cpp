#include "StepperMotor.h"
#include <string>
#include <boost/lexical_cast.hpp>






int main( int argc, char* argv[] ) {

    std::cout << "Test MotorStepper class !!! \n";


    std::string deviceName(argv[1]);
    std::string configFileNameName(argv[2]);
    int motorDriverId = boost::lexical_cast<int>(argv[3]);

    std::cout << "Try to open MOTOR CARD with device name: " << deviceName << " with config file: " << configFileNameName << " Motor driver ID is: " << motorDriverId << std::endl;

    mtca4u::StepperMotor motor(deviceName, motorDriverId, configFileNameName);

    mtca4u::StepperMotorStatus status(motor.getMotorStatus());
    std::cout << "Current motor status is: " << status<< "\n";
    if (status == mtca4u::StepperMotorStatusTypes::MOTOR_DISABLED) {
        std::cout << "Enabling the motor.\n";
        motor.setEnable(true);
    }

    status = motor.getMotorStatus();
    std::cout << "Current motor status is: " << status << "\n";

    if (status == mtca4u::StepperMotorStatusTypes::MOTOR_IN_ERROR) {
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
        if (motor.moveToPosition(currentMotorPosition+10000) != mtca4u::StepperMotorStatusTypes::MOTOR_OK) {
            std::cout << "Motor movement failed. Status is: " << motor.getMotorStatus() << std::endl;
        }
        std::cout << "!!!!!!!!!!!!!!Current motor position is: " << motor.getMotorPosition() << "\n";
         usleep(100000);
        std::cout << "!!!!!!!!!!!!!!Current motor position is: " << motor.getMotorPosition() << "\n";
        /*
        std::cout << "Move to position: " << 0<< "\n";
        if (motor.moveToPosition(0) != mtca4u::StepperMotorStatusTypes::MOTOR_OK) {
            std::cout << "Motor movement failed. Status is: " << motor.getMotorStatus() << std::endl;
        }
        std::cout << "!!!!!!!!!!!!!!Current motor position is: " << motor.getMotorPosition() << "\n";
        usleep(100000);
        std::cout << "!!!!!!!!!!!!!!Current motor position is: " << motor.getMotorPosition() << "\n";
         */
    }
     std::cout << "Calibrate motor\n";
     motor.calibrateMotor();
    
    
    std::cout << "Disabling the motor\n";
    motor.setEnable(false);
    std::cout << "End of Test MotorStepper class !!! \n";
    return 0;
}


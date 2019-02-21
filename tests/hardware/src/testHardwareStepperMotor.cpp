

#include "LinearStepperMotor.h"
#include <ChimeraTK/BackendFactory.h>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <pthread.h>
#include <string>

pthread_t statusReaderThreadTab[5];
bool threadActive = false;
bool finishThread = false;

void *statusReaderThreadBody(void *ptr) {
  threadActive = true;
  mtca4u::LinearStepperMotor *motorPointer = (mtca4u::LinearStepperMotor *)ptr;
  while (1) {
    std::cout << "Thread:: " << (unsigned int)(pthread_self())
              << " motor status is: "
              << motorPointer->getStatusAndError().status.asString()
              << std::endl;
    usleep(10000);
    if (finishThread)
      break;
  }
  std::cout << "Thread:: Finish thread: " << (unsigned int)(pthread_self())
            << std::endl;
  threadActive = false;
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

  if (argc < 3) {
    std::cout
        << "Usage: " << argv[0]
        << " dmapFile moduleName [deviceAlias=DFMC-MD22] "
           "[motorConfig=VT21-MotorDriverCardConfig.xml] [motorDriverId=0]\n\n"
        << "Parameters:\n"
        << "  dmapFile The device map file with alias, sdm and map file"
        << "  moduleName  Name of the MD22 module in the firmware map file\n"
        << "  deviceAlias Device alias in the map file, optional, default: "
           "DFMC-MD22\n"
        << "  motorConfig Motor driver config XML file, optional, default: "
           "VT21-MotorDriverCardConfig.xml\n"
        << "  motorId     Motor ID on the MD22 (0 or 1), default: 0"
        << std::endl;

    return -1;
  }

  std::string dmapFileName = argv[1];
  ChimeraTK::BackendFactory::getInstance().setDMapFilePath(dmapFileName);

  std::string moduleName = argv[2];

  std::string deviceName;
  if (argc >= 4) {
    deviceName = argv[2];
  } else {
    deviceName = "DFMC-MD22";
  }

  std::string configFileNameName;
  if (argc >= 5) {
    configFileNameName = argv[3];
  } else {
    configFileNameName = "VT21-MotorDriverCardConfig.xml";
  }

  int motorDriverId;
  if (argc >= 6) {
    motorDriverId = boost::lexical_cast<int>(argv[4]);
  } else {
    motorDriverId = 0;
  }

  std::cout << "Try to open MOTOR CARD with device name: " << deviceName
            << " with config file: " << configFileNameName
            << " Motor driver ID is: " << motorDriverId << std::endl;

  mtca4u::LinearStepperMotor motor(deviceName, moduleName, motorDriverId,
                                   configFileNameName);

  std::cout << "Change debug stream to the std::cout \n";
  motor.setLogLevel(mtca4u::Logger::INFO);

  mtca4u::LinearStepperMotorStatus status(motor.getStatusAndError().status);
  std::cout << "Current motor status is: " << status << "\n";
  if (status == mtca4u::LinearStepperMotorStatusTypes::M_DISABLED) {
    std::cout << "Enabling the motor.\n";
    motor.setEnabled(true);
  }

  status = motor.getStatusAndError().status;
  std::cout << "Current motor status is: " << status << "\n";

  if (status == mtca4u::LinearStepperMotorStatusTypes::M_ERROR) {
    mtca4u::LinearStepperMotorError error(motor.getStatusAndError().error);
    std::cout << "Motor in error state. Error is: " << error << "\n";
    if (error == mtca4u::LinearStepperMotorErrorTypes::M_BOTH_END_SWITCH_ON) {
      std::cout << "Both end switches on. Check you hardware. (" << error
                << "\n";
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
    /*
    float newPosition = currentMotorPosition+1000000;
    std::cout << "Move motor to new position: " << newPosition << std::endl;
    motor.moveToPosition(newPosition);

    mtca4u::StepperMotorStatus status = motor.getStatus();
    if (status != mtca4u::LinearStepperMotorStatusTypes::M_OK) {
        std::cout << "Moving to target position failed. Current motor status is:
    " << status << "\n";
    }

    if (motor.getStatus() ==
    mtca4u::LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON) {
        newPosition = motor.getCurrentPosition() - 1000000;
        std::cout << "Stepper Motor at positive end-switch. Move to negative
    direction to position: " << newPosition << std::endl;
        motor.moveToPosition(newPosition);
        status = motor.getStatus();
        if (motor.getStatus() != mtca4u::LinearStepperMotorStatusTypes::M_OK) {
            std::cout << "Moving to target position failed. Current motor status
    is: " << status << "\n";
        }

    }
    */
  }

  // std::cout << "Disabling the motor\n";
  // motor.setEnabled(false);
  // std::cout << "End of Test MotorStepper class !!! \n";
  // return 0;

  mtca4u::StepperMotorCalibrationStatus calibrationStatus =
      motor.getCalibrationStatus();
  std::cout << "Current calibration status: " << calibrationStatus << std::endl;

  int rc[5] = {0, 0, 0, 0, 0};

  std::cout << "Creating status reader thread.\n" << std::flush;

  for (int i = 0; i < 1; i++) {
    rc[i] = pthread_create(&statusReaderThreadTab[i], NULL,
                           statusReaderThreadBody, (void *)(&motor));

    if (rc[i]) {
      std::cout << "Unable to create thread. Error code: " << rc << std::endl;
      exit(1);
    }
  }
  std::cout << "Calibrate motor start\n";
  calibrationStatus = motor.calibrateMotor();
  if (motor.getCalibrationStatus() ==
      mtca4u::StepperMotorCalibrationStatusType::M_CALIBRATED) {
    std::cout << "Calibration done with success !!!!!!!!!!!!!!!!!!!!!!!!!!!!! "
              << std::endl;
  } else {
    std::cout << "Calibration finished with status: "
              << motor.getCalibrationStatus() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
              << std::endl;
    // if (motor.getCalibrationStatus() ==
    // mtca4u::StepperMotorCalibrationStatusType::M_CALIBRATION_STOPED_BY_USER)
    // break;
  }

  std::cout << "Disabling the motor\n";
  motor.setEnabled(false);

  std::cout << "Stopping thread\n";
  finishThread = true;
  while (threadActive)
    ;

  std::cout << "End of Test MotorStepper class !!! \n";
  return 0;
}

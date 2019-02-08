
#include "ChimeraTK/MotorDriverCard/StepperMotor.h"

#include <iostream>

/**
 * The MotorDriverCard library is based on ChimeraTK-DeviceAccess.
 * It requires a dmap=file....TODO
 * For basic information, please refer to the DeviceAccess documentation.
 */
static const std::string deviceName("STEPPER-MOTOR-DUMMY");
static const std::string dmapPath(".");
static const std::string moduleName("");

// Specify the configuration file
static const std::string stepperMotorConfigFile("VT21-MotorDriverCardConfig.xml");

using namespace ChimeraTK::MotorDriver;

int main(){

  //TODO Easy waz to define dmap path
//  std::string deviceFileName(ChimeraTK::DMapFilesParser(dmapPath).getdMapFileElem(deviceName).deviceName);
//  std::string mapFileName(ChimeraTK::DMapFilesParser(dmapPath).getdMapFileElem(deviceName).mapFileName);

  // Define the parameter structure
  StepperMotorParameters param;
  param.motorType = StepperMotorType::BASIC;
  param.deviceName = deviceName;
  param.moduleName = moduleName;
  param.driverId   = 0U; /* MotorDriverCard provides two channels [0, 1] */
  param.configFileName = stepperMotorConfigFile;


  // Create a StepperMotor instance using the factory
  auto stepperMotor = StepperMotorFactory::instance().create(param);


  // Motor does not have a position reference yet,
  // i.e is not calibrated
  if(stepperMotor->getCalibrationMode() == CalibrationMode::NONE){
    std::cout << "Motor is not yet calibrated" << std::endl;
  }


  // Define current positon as zero
  stepperMotor->setActualPosition(0.f);


  std::cout << "Current motor positon: "
            << stepperMotor->getCurrentPosition() << std::endl;

  return 0;
}

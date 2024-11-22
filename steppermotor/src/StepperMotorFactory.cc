
#include "BasicStepperMotor.h"
#include "LinearStepperMotor.h"
#include "StepperMotor.h"

#include <mutex>

#include <ChimeraTK/Exception.h>

namespace ChimeraTK::MotorDriver {

  StepperMotorFactory::StepperMotorFactory() : _factoryMutex{}, _existingMotors{} {}

  StepperMotorFactory& StepperMotorFactory::instance() {
    static StepperMotorFactory instance;
    return instance;
  }

  std::shared_ptr<StepperMotor> StepperMotorFactory::create(const StepperMotorParameters& parameters) {
    std::lock_guard<std::mutex> lck(_factoryMutex);

    // Check if instance already exists and
    // return it, if this is given
    MotorIdentifier id = std::make_tuple(parameters.deviceName, parameters.moduleName, parameters.driverId);

    auto iterator = _existingMotors.find(id);
    if(iterator != _existingMotors.end()) {
      auto ptr = iterator->second.lock();
      // FIXME: This should catch the case that
      //  motor exists but additional parameters differ
      if(ptr) {
        return ptr;
      }
    }

    // Else, create a new instance as requested
    std::shared_ptr<StepperMotor> ptr;
    switch(parameters.motorType) {
      case StepperMotorType::BASIC:
        ptr = std::make_shared<BasicStepperMotor>(parameters);
        break;
      case StepperMotorType::LINEAR:
        ptr = std::make_shared<LinearStepperMotor>(parameters);
        break;
      default:
        throw ChimeraTK::logic_error("Invalid Stepper motor variant");
    }

    _existingMotors[id] = ptr;
    return ptr;
  }

} // namespace ChimeraTK::MotorDriver

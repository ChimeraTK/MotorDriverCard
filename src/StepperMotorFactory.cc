
#include "StepperMotor.h"
#include "BasicStepperMotor.h"
#include "StepperMotorWithReference.h"

#include <mutex>

namespace ChimeraTK{

  StepperMotorFactory& StepperMotorFactory::instance(){
    static StepperMotorFactory instance;
    return instance;
  }

  std::shared_ptr<StepperMotor> StepperMotorFactory::create(
        const StepperMotorType type,
        StepperMotorParameters&& parameters)
  {
    std::lock_guard<std::mutex> lck(_factoryMutex);

    // Check if instance already exists and
    // return it, if this is given
    MotorIdentifier id
      = std::make_tuple(parameters.deviceName, parameters.moduleName, parameters.driverId);

    auto iterator = _existingMotors.find(id);
    if(iterator != _existingMotors.end()){
      auto ptr = iterator->second.lock();
      if(ptr){
        return ptr;
      }
    }

    // Else, create a new instance as requested
    std::shared_ptr<StepperMotor> ptr;
    switch(type){
      case StepperMotorType::BASIC:
        ptr = std::make_shared<BasicStepperMotor>(parameters);
        break;
      case StepperMotorType::LINEAR:
        ptr = std::make_shared<StepperMotorWithReference>(parameters);
        break;
    }

    _existingMotors[id] = ptr;
    return ptr;
  }

} // namespace ChimeraTK


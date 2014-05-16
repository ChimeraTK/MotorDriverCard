#include "StepperMotorError.h"

namespace mtca4u {
   
    StepperMotorError StepperMotorErrorTypes::MOTOR_NO_ERROR(1, "NO ERROR");
    StepperMotorError StepperMotorErrorTypes::MOTOR_BOTH_ENDSWICHTED_ON(2, "BOTH END SWITCHES ON");
    StepperMotorError StepperMotorErrorTypes::MOTOR_COMMUNICATION_LOST(4, "COMMUNICATION LOST");
    StepperMotorError StepperMotorErrorTypes::MOTOR_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX(8, "MOTOR CONFIGURATION ERROR - MIN POSITION GREATER OR EQUAL TO MAX");

    
    
    
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //StepperMotorError class definition !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    StepperMotorError::StepperMotorError() : StatusGeneral(1, "NO ERROR") {
    }

    StepperMotorError::StepperMotorError(int itemId, std::string itemName) : StatusGeneral(itemId, itemName) {
    }

    StepperMotorError::StepperMotorError(const StepperMotorError &error) : StatusGeneral(error) {
    }
}

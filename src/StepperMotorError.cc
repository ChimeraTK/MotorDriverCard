#include "StepperMotorError.h"

namespace mtca4u {
   
    StepperMotorError StepperMotorErrorTypes::M_NO_ERROR(1, "NO ERROR");
    StepperMotorError StepperMotorErrorTypes::M_BOTH_END_SWITCH_ON(2, "BOTH END SWITCHES ON");
    StepperMotorError StepperMotorErrorTypes::M_COMMUNICATION_LOST(4, "COMMUNICATION LOST");
    StepperMotorError StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX(8, "MOTOR CONFIGURATION ERROR - MIN POSITION GREATER OR EQUAL TO MAX");
    StepperMotorError StepperMotorErrorTypes::M_NO_REACTION_ON_COMMAND(16, "NO REACTION ON COMMAND");

    
    
    
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //StepperMotorError class definition !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    StepperMotorError::StepperMotorError() : GeneralStatus(1, "NO ERROR") {
    }

    StepperMotorError::StepperMotorError(int itemId, std::string itemName) : GeneralStatus(itemId, itemName) {
    }

    StepperMotorError::StepperMotorError(const StepperMotorError &error) : GeneralStatus(error) {
    }
}

#include "StepperMotorStatus.h"

namespace mtca4u {
   
    StepperMotorStatus StepperMotorStatusTypes::M_OK(1, "OK");
    StepperMotorStatus StepperMotorStatusTypes::M_DISABLED(2, "DISABLED");
    StepperMotorStatus StepperMotorStatusTypes::M_IN_MOVE(4, "IN MOVE");
    StepperMotorStatus StepperMotorStatusTypes::M_NOT_IN_POSITION(8, "NOT IN POSITION");
    StepperMotorStatus StepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON(16, "HARDWARE POSITIVE END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON(32, "HARDWARE NEGATIVE END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON(64, "SOFTWARE POSITIVE END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON(128, "SOFTWARE NEGATIVE END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::M_ERROR(256, "ERROR");
    
    
    
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //StepperMotorStatus class definition !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    StepperMotorStatus::StepperMotorStatus() : StatusGeneral(1, "OK") {
    }

    StepperMotorStatus::StepperMotorStatus(int itemId, std::string itemName) : StatusGeneral(itemId, itemName) {
    }

    StepperMotorStatus::StepperMotorStatus(const StepperMotorStatus &error) : StatusGeneral(error) {
    }
}


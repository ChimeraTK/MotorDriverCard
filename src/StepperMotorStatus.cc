#include "StepperMotorStatus.h"

namespace mtca4u {
 

    LinearStepperMotorStatus LinearStepperMotorStatusTypes::M_OK(1, "OK");
    LinearStepperMotorStatus LinearStepperMotorStatusTypes::M_DISABLED(2, "DISABLED");
    LinearStepperMotorStatus LinearStepperMotorStatusTypes::M_IN_MOVE(4, "IN MOVE");
    LinearStepperMotorStatus LinearStepperMotorStatusTypes::M_NOT_IN_POSITION(8, "NOT IN POSITION");
    LinearStepperMotorStatus LinearStepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON(16, "SOFTWARE POSITIVE END SWITCH ON");
    LinearStepperMotorStatus LinearStepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON(32, "SOFTWARE NEGATIVE END SWITCH ON");
    LinearStepperMotorStatus LinearStepperMotorStatusTypes::M_ERROR(64, "ERROR");
    LinearStepperMotorStatus LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON(128, "HARDWARE POSITIVE END SWITCH ON");
    LinearStepperMotorStatus LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON(256, "HARDWARE NEGATIVE END SWITCH ON");
    
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //LinearStepperMotorStatus class definition !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    LinearStepperMotorStatus::LinearStepperMotorStatus() : StepperMotorStatus(1, "OK") {
    }

    LinearStepperMotorStatus::LinearStepperMotorStatus(int itemId, std::string itemName) : StepperMotorStatus(itemId, itemName) {
    }

    LinearStepperMotorStatus::LinearStepperMotorStatus(const LinearStepperMotorStatus &status) : StepperMotorStatus(status) {
    }

    


    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //StepperMotorStatus Types !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    
    StepperMotorStatus StepperMotorStatusTypes::M_OK(1, "OK");
    StepperMotorStatus StepperMotorStatusTypes::M_DISABLED(2, "DISABLED");
    StepperMotorStatus StepperMotorStatusTypes::M_IN_MOVE(4, "IN MOVE");
    StepperMotorStatus StepperMotorStatusTypes::M_NOT_IN_POSITION(8, "NOT IN POSITION");
    StepperMotorStatus StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON(16, "SOFTWARE POSITIVE END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON(32, "SOFTWARE NEGATIVE END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::M_ERROR(64, "ERROR");
    
    
    
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //StepperMotorStatus class definition !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    StepperMotorStatus::StepperMotorStatus() : GeneralStatus(1, "OK") {
    }

    StepperMotorStatus::StepperMotorStatus(int itemId, std::string itemName) : GeneralStatus(itemId, itemName) {
    }

    StepperMotorStatus::StepperMotorStatus(const StepperMotorStatus &status) : GeneralStatus(status) {
    }
}


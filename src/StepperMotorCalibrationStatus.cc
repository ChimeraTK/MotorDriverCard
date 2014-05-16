#include "StepperMotorCalibrationStatus.h"

namespace mtca4u {


    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_UNKNOWN(1, "UNKNOWN");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATED(2, "CALIBRATED");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_NOT_CALIBRATED(4, "NOT CALIBRATED");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED(8, "FAILED");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS(16, "IN PROGRESS");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_STOPED_BY_USER(32, "STOPPED BY USER");

    
    
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //StepperMotorCalibrationStatus class definition !!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!    
    
    StepperMotorCalibrationStatus::StepperMotorCalibrationStatus() : StatusGeneral(1, "UNKNOWN") {
    }

    StepperMotorCalibrationStatus::StepperMotorCalibrationStatus(int itemId, std::string itemName) : StatusGeneral(itemId, itemName) {
    }

    StepperMotorCalibrationStatus::StepperMotorCalibrationStatus(const StepperMotorCalibrationStatus &error) : StatusGeneral(error) {
    }
}

#include "StepperMotorCalibrationStatus.h"

namespace mtca4u {


    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_UNKNOWN(1, "UNKNOWN");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATED(2, "CALIBRATED");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_NOT_CALIBRATED(4, "NOT CALIBRATED");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED(8, "FAILED");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS(16, "IN PROGRESS");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_STOPED_BY_USER(32, "STOPPED BY USER");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE(64, "NOT AVAILABLE");

    
    
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //StepperMotorCalibrationStatus class definition !!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!    
    
    StepperMotorCalibrationStatus::StepperMotorCalibrationStatus() : GeneralStatus(1, "UNKNOWN") {
    }

    StepperMotorCalibrationStatus::StepperMotorCalibrationStatus(int itemId, std::string itemName) : GeneralStatus(itemId, itemName) {
    }

    StepperMotorCalibrationStatus::StepperMotorCalibrationStatus(const StepperMotorCalibrationStatus &error) : GeneralStatus(error) {
    }
}

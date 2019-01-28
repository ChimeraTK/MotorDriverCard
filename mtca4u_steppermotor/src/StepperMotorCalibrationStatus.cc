#include "StepperMotorCalibrationStatus.h"

namespace mtca4u {


    const StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_UNKNOWN(1);
    const StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATED(2);
    const StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_NOT_CALIBRATED(4);
    const StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED(8);
    const StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS(16);
    const StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER(32);
    const StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE(64);

    
    
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //StepperMotorCalibrationStatus class definition !!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!    
    
    StepperMotorCalibrationStatus::StepperMotorCalibrationStatus() : GeneralStatus(1) {
    }

    StepperMotorCalibrationStatus::StepperMotorCalibrationStatus(int id) : GeneralStatus(id) {
    }

    StepperMotorCalibrationStatus::StepperMotorCalibrationStatus(const StepperMotorCalibrationStatus &error) : GeneralStatus(error) {
    }
  
    std::string StepperMotorCalibrationStatus::asString() const {
        std::ostringstream stream;

        if        (_id == StepperMotorCalibrationStatusType::M_CALIBRATION_UNKNOWN) {
            stream << "Calibration status UNKNOWN (";
        } else if (_id == StepperMotorCalibrationStatusType::M_CALIBRATED) {
            stream << "Motor is CALIBRATED (";
        } else if (_id == StepperMotorCalibrationStatusType::M_NOT_CALIBRATED) {
            stream << "Motor is NOT CALIBRATED (";
        } else if (_id == StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED) {
            stream << "Motor calibration FAILED (";
        } else if (_id == StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS) {
            stream << "Motor calibration is IN PROGRESS (";
        } else if (_id == StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER) {
            stream << "Motor calibration STOPPED BY USER (";
        } else if (_id == StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE) {
            stream << "Motor calibration NOT AVAILABLE (";
        } else {
            return GeneralStatus::asString();
        }
        
        stream << _id << ")";
        return stream.str();
    }
}

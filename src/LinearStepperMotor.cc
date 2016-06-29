#include "LinearStepperMotor.h"
#include <cmath>

#define DEBUG(x) if (_debugLevel>=x) *_debugStream 

namespace mtca4u {

    LinearStepperMotor::LinearStepperMotor(std::string const & motorDriverCardDeviceName, std::string const & moduleName, unsigned int motorDriverId, std::string motorDriverCardConfigFileName)
    : StepperMotor(motorDriverCardDeviceName, moduleName, motorDriverId, motorDriverCardConfigFileName) {

        //end switches enabled or not
        _positiveEndSwitchEnabled = _motorControler->getReferenceSwitchData().getPositiveSwitchEnabled();
        _negativeEndSwitchEnabled = _motorControler->getReferenceSwitchData().getNegativeSwitchEnabled();
    
        //calibration
        _calibNegativeEndSwitchInUnits = _calibNegativeEndSwitchInSteps = 0;
        _calibPositiveEndSwitchInUnits = _calibPositiveEndSwitchInSteps = 0;
        _stopMotorCalibration = false;
    }

    LinearStepperMotor::~LinearStepperMotor() {
    }


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // BLOCKING FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    LinearStepperMotorStatusAndError LinearStepperMotor::moveToPosition(
        float newPosition) {

      LinearStepperMotorStatusAndError statusAndError;
      statusAndError = this->determineMotorStatusAndError();

      if (statusAndError.status == StepperMotorStatusTypes::M_ERROR ||
          statusAndError.status == StepperMotorStatusTypes::M_DISABLED) {
        return statusAndError;
      }

      _blockingFunctionActive = true;

      // Ask the hardware to move the motor.
      this->setTargetPosition(newPosition);
      if (!this->getAutostart()) {
          this->start();
      }

      // Block till the motor has finished moving.
      while (StepperMotor::isMoving()){
          usleep(1000);
      }

      // update and return status and error once at target position.
      _blockingFunctionActive = false;
      return(this->determineMotorStatusAndError());
    }

    StepperMotorCalibrationStatus LinearStepperMotor::calibrateMotor() {

        // we need to check whether any previously triggered calibration is ongoing - multi thread programs.
        if (_motorCalibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS) {
            _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Calibration already in progress. Returning. " << std::endl;
            return _motorCalibrationStatus;
        }

        LinearStepperMotorStatusAndError statusAndError = this->determineMotorStatusAndError();

        if (statusAndError.status == LinearStepperMotorStatusTypes::M_ERROR || statusAndError.status == LinearStepperMotorStatusTypes::M_DISABLED) {
            _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
            return _motorCalibrationStatus;
        }

        bool originalSoftwareLimitEnabled = _softwareLimitsEnabled;
        _softwareLimitsEnabled = false;

        _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS;

        if (_motorControler->getReferenceSwitchData().getPositiveSwitchEnabled() == false || _motorControler->getReferenceSwitchData().getNegativeSwitchEnabled() == false) {
            _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : End switches not available. Calibration not possible. " << std::endl;
            _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE;
            return _motorCalibrationStatus;
        }

        //enable the motor
        _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Enable the motor driver." << std::endl;
        _motorControler->setEnabled(true);
        
        //move to negative end switch
        moveToEndSwitch(LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON);
        if (_motorCalibrationStatus != StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS) {
            _softwareLimitsEnabled = originalSoftwareLimitEnabled;
            return _motorCalibrationStatus;
        }
        _calibNegativeEndSwitchInSteps = _motorControler->getActualPosition();
        _calibNegativeEndSwitchInUnits = recalculateStepsToUnits(_calibNegativeEndSwitchInSteps);

        _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Negative end switch reached at (in steps): " << _calibNegativeEndSwitchInSteps << " Sending to positive end switch.\n";
        _logger(Logger::DETAIL) << "LinearStepperMotor::calibrateMotor : Current position is: " << _motorControler->getActualPosition() << ". Go to positive end switch.\n";

        //move to positive end switch
        moveToEndSwitch(LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON);
        if (_motorCalibrationStatus != StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS) {
            _softwareLimitsEnabled = originalSoftwareLimitEnabled;
            return _motorCalibrationStatus;
        }

        _calibPositiveEndSwitchInSteps = _motorControler->getActualPosition();
        _calibPositiveEndSwitchInUnits = recalculateStepsToUnits(_calibPositiveEndSwitchInSteps);

        _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Negative end switch is at (steps):  " << _calibNegativeEndSwitchInSteps << " Positive end switch at (steps): " << _calibPositiveEndSwitchInSteps << std::endl;
        _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Negative end switch is at (units):  " << _calibNegativeEndSwitchInUnits << " Positive end switch at (units): " << _calibPositiveEndSwitchInUnits << std::endl;
        float newPos = (_calibPositiveEndSwitchInUnits + _calibNegativeEndSwitchInUnits) / 2;
        _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Sending motor to middle of range:  " << newPos << std::endl;

        //move to middle of the range
        statusAndError = this->moveToPosition(newPos);

        //in case when user will stop calibration when moving to the middle range between positive and negative end switch positions
        if (_stopMotorCalibration == true) {
            _stopMotorCalibration = false;
            _logger(Logger::WARNING) << "LinearStepperMotor::calibrateMotor : StepperMotor::calibrateMotor : Motor stopped by user.\n" << std::flush;
            _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER;
        } else if (statusAndError.status == mtca4u::LinearStepperMotorStatusTypes::M_ERROR) {
            _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
        } else {
            _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATED;
        }

        _softwareLimitsEnabled = originalSoftwareLimitEnabled;
        return _motorCalibrationStatus;
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // END OF BLOCKING FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // GENERAL FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    void LinearStepperMotor::setCurrentPositionAs(float newPosition) {
        if (_motorCalibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATED) {
            //just to update current position readout
            getCurrentPosition();
            //calculate difference between current position and new wanted position
            float delta = _currentPostionsInUnits - newPosition;

            _logger(Logger::INFO) << "LinearStepperMotor::setCurrenPositionAs : Motor calibrated. Recalculating end switches position." << std::endl;
            _logger(Logger::INFO) << "LinearStepperMotor::setCurrenPositionAs : Current position: " << _currentPostionsInUnits << ", in steps: " << _currentPostionsInSteps << std::endl;
            _logger(Logger::INFO) << "LinearStepperMotor::setCurrenPositionAs : Wanted  position: " << newPosition << ", in steps: " << this->recalculateUnitsToSteps(newPosition) << std::endl;
            _logger(Logger::INFO) << "LinearStepperMotor::setCurrenPositionAs : Delta   position: " << delta << ", in steps: " << this->recalculateUnitsToSteps(delta) << std::endl;
            _logger(Logger::INFO) << "LinearStepperMotor::setCurrenPositionAs : Negative end switch (in steps): " << _calibNegativeEndSwitchInSteps << " positive end switch(in steps): " << _calibPositiveEndSwitchInSteps << std::endl;
            
            //recalculate hardware end switches position
            _calibNegativeEndSwitchInUnits -= delta;
            _calibNegativeEndSwitchInSteps = recalculateUnitsToSteps(_calibNegativeEndSwitchInUnits);

            _calibPositiveEndSwitchInUnits -= delta;
            _calibPositiveEndSwitchInSteps = recalculateUnitsToSteps(_calibPositiveEndSwitchInUnits);
            _logger(Logger::INFO) << "LinearStepperMotor::setCurrenPositionAs :Recalculated. Negative end switch (in steps): " << _calibNegativeEndSwitchInSteps << " positive end switch(in steps): " << _calibPositiveEndSwitchInSteps << std::endl;
        }

        StepperMotor::setCurrentPositionAs(newPosition);
    }

    void LinearStepperMotor::stop() {
        if (getCalibrationStatus() == StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS)
            _stopMotorCalibration = true;

        StepperMotor::stop();
    }

    void LinearStepperMotor::emergencyStop() {
      _motorControler->setEnabled(false);
      this->stop();
      this->_motorCalibrationStatus =
          StepperMotorCalibrationStatusType::M_NOT_CALIBRATED;
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // END OF GENERAL FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // GETTERS AND SETTERS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  

    float LinearStepperMotor::getPositiveEndSwitchPosition() const {
        return _calibPositiveEndSwitchInUnits;
    }

    float LinearStepperMotor::getNegativeEndSwitchPosition() const {
        return _calibNegativeEndSwitchInUnits;
    }
    
    LinearStepperMotorStatusAndError LinearStepperMotor::getStatusAndError() {
        return determineMotorStatusAndError();
    }
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // END OF GETTERS AND SETTERS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // PRIVATE METHODS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    LinearStepperMotorStatusAndError LinearStepperMotor::determineMotorStatusAndError() {
        boost::lock_guard<boost::mutex> lock_guard(_mutex);

        _motorStatus = LinearStepperMotorStatusTypes::M_OK;
        _motorError = StepperMotorErrorTypes::M_NO_ERROR;

        //first check if disabled
        if (!_motorControler->isEnabled()) {
            _motorStatus = LinearStepperMotorStatusTypes::M_DISABLED;
            return LinearStepperMotorStatusAndError(_motorStatus, _motorError);
        }

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // ERROR CONDITION CHECK - it should be done first.
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        //check end switches
        bool positiveSwitchStatus = _motorControler->getReferenceSwitchData().getPositiveSwitchActive() & _positiveEndSwitchEnabled;
        bool negativeSwitchStatus = _motorControler->getReferenceSwitchData().getNegativeSwitchActive() & _negativeEndSwitchEnabled;
   
        //check if both end switches are on in the same time - this means error;    
        if (positiveSwitchStatus && negativeSwitchStatus) {

            positiveSwitchStatus = _motorControler->getReferenceSwitchData().getPositiveSwitchActive();
            negativeSwitchStatus = _motorControler->getReferenceSwitchData().getNegativeSwitchActive();

            if (positiveSwitchStatus)
                _logger(Logger::ERROR) << "LinearStepperMotor::determineMotorStatusAndError(): Positive end-switch on. " << std::endl;
            
            if (negativeSwitchStatus)
                _logger(Logger::ERROR) << "LinearStepperMotor::determineMotorStatusAndError(): Negative end-switch on. " << std::endl;
            
            if (positiveSwitchStatus && negativeSwitchStatus) {
                _motorStatus = LinearStepperMotorStatusTypes::M_ERROR;
                _logger(Logger::ERROR) << "LinearStepperMotor::determineMotorStatusAndError(): Both end switches error detected. " << std::endl;
                _motorError = LinearStepperMotorErrorTypes::M_BOTH_END_SWITCH_ON;

                return LinearStepperMotorStatusAndError(_motorStatus, _motorError);
            }
        }
        //check configuration of software end switches error
        if (_softwareLimitsEnabled && _maxPositionLimit <= _minPositionLimit) {
            _motorStatus = LinearStepperMotorStatusTypes::M_ERROR;
            _motorError = StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX;

            return LinearStepperMotorStatusAndError(_motorStatus, _motorError);
        }

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // STATUS CHECK
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  

        //hardware end switches active
        if (positiveSwitchStatus) {
            _motorStatus = LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON;
            return LinearStepperMotorStatusAndError(_motorStatus, _motorError);
        }

        if (negativeSwitchStatus) {
            _motorStatus = LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON;
            return LinearStepperMotorStatusAndError(_motorStatus, _motorError);
        }
        
        //software end switches active
        if (_softwareLimitsEnabled) {
            if (_targetPositionInUnits >= _maxPositionLimit) {
                _motorStatus = LinearStepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON;
                return LinearStepperMotorStatusAndError(_motorStatus, _motorError);
            }

            if (_targetPositionInUnits <= _minPositionLimit) {
                _motorStatus = LinearStepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON;
                return LinearStepperMotorStatusAndError(_motorStatus, _motorError);
            }
        }
        //current on -> motor is moving
        if (_motorControler->getStatus().getStandstillIndicator() == 0) {
            _motorStatus = LinearStepperMotorStatusTypes::M_IN_MOVE;
            return LinearStepperMotorStatusAndError(_motorStatus, _motorError);
        }
        
        if (_targetPositionInSteps != _motorControler->getActualPosition()) {
            _motorStatus = LinearStepperMotorStatusTypes::M_NOT_IN_POSITION;
            return LinearStepperMotorStatusAndError(_motorStatus, _motorError);
        }

        return LinearStepperMotorStatusAndError(_motorStatus, _motorError);
    }

    void LinearStepperMotor::moveToEndSwitch(LinearStepperMotorStatus targetStatus) {
        LinearStepperMotorStatusAndError statusAndError;
        bool takeFlagInAccount = false;

        //condition for targetStatus equal to LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON
        //one want to move in positive direction
        int dir = 1;
        //and when negative end switch will be set when positive expected -> error
        LinearStepperMotorStatus errortargetStatus = LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON;

        //condition for targetStatus equal to LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON
        if (targetStatus == LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON) {
            dir = -1;
            errortargetStatus = LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON;
        }

        do {
            //get current position and send motor to new lower position
            int currentPosition = _motorControler->getActualPosition();
            float newPosInUnits = recalculateStepsToUnits(currentPosition + dir * 500000);
            _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Current position (steps): " << currentPosition << " (units): " << recalculateStepsToUnits(currentPosition) << " New position (steps): " << currentPosition - 10000 << " (units): " << newPosInUnits << std::endl;
            statusAndError = moveToPosition(newPosInUnits);

            if (_stopMotorCalibration == true) {
                _stopMotorCalibration = false;
                _logger(Logger::WARNING) << "LinearStepperMotor::calibrateMotor : Calibration stopped by user." << std::endl;

                _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER;
                return;
            }

            if (statusAndError.status == LinearStepperMotorStatusTypes::M_IN_MOVE || statusAndError.status == LinearStepperMotorStatusTypes::M_NOT_IN_POSITION) {
                takeFlagInAccount = true;
            }

            if (takeFlagInAccount && statusAndError.status == errortargetStatus) {
                _logger(Logger::ERROR) << "LinearStepperMotor::calibrateMotor : " << (dir == 1 ? "Negative" : "Positive") << " end switch reached when " << (dir == 1 ? "positive" : "negative") << " expected. Calibration failed.\n";
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
                return;
            }
            if (statusAndError.status == LinearStepperMotorStatusTypes::M_ERROR) {
                _logger(Logger::ERROR) << "LinearStepperMotor::calibrateMotor : Motor in error state. Error is: " << statusAndError.error << std::endl;
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
                return;
            }
        } while (statusAndError.status != targetStatus);
    }
}

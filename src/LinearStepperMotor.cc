#include "LinearStepperMotor.h"
#include "math.h"

#define DEBUG(x) if (_debugLevel>=x) *_debugStream 



namespace mtca4u {

    LinearStepperMotor::LinearStepperMotor(std::string motorDriverCardDeviceName, unsigned int motorDriverId, std::string motorDriverCardConfigFileName) : StepperMotor(motorDriverCardDeviceName, motorDriverId, motorDriverCardConfigFileName)  {

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

    MotorStatusAndError LinearStepperMotor::moveToPosition(float newPosition) {

        MotorStatusAndError statusAndError;
        
        statusAndError = this->determineMotorStatusAndError();
        if (getCurrentPosition() == newPosition) {
            return statusAndError;
        }
        
        if (statusAndError.status == StepperMotorStatusTypes::M_ERROR) {
            return statusAndError;
        }
        

        _blockingFunctionActive = true;
        this->setTargetPosition(newPosition);

        if (!this->getAutostart()) {
            this->start();
        }

        int dummy = 0;
        bool continueWaiting = true;
        unsigned int notInPositionCounter = 0;
        do {

            statusAndError = this->determineMotorStatusAndError();
            _logger(Logger::FULL_DETAIL) << dummy++ << ") LinearStepperMotor::moveToPosition : Motor in move. Current position: " << this->getCurrentPosition() << ". Target position: " << _targetPositionInSteps << ". Current status: " << _motorStatus << std::endl;

            int currentMotorPosition = _motorControler->getActualPosition();

            if (statusAndError.status == LinearStepperMotorStatusTypes::M_ERROR) {
                return statusAndError;
            }

            if (_stopMotorForBlocking == true) {
                continueWaiting = false;
                _stopMotorForBlocking = false;
                _logger(Logger::WARNING) << "LinearStepperMotor::moveToPosition : Motor stopped by user." << std::endl;
                while (statusAndError.status == LinearStepperMotorStatusTypes::M_IN_MOVE || statusAndError.status == LinearStepperMotorStatusTypes::M_NOT_IN_POSITION) {
                    _logger(Logger::DETAIL) << "LinearStepperMotor::moveToPosition : Wait to change status from M_IN_MOVE/M_NOT_IN_POSITION to other." << std::endl;
                    statusAndError = this->determineMotorStatusAndError();
                }
            } else if (statusAndError.status == LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON && _targetPositionInSteps > currentMotorPosition) {
                continueWaiting = true;
                _logger(Logger::DETAIL) << "LinearStepperMotor::moveToPosition : Negative end switch on but moving in positive direction. Current position: " << getCurrentPosition() << std::endl;
            } else if (statusAndError.status == LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON && _targetPositionInSteps < currentMotorPosition) {
                continueWaiting = true;
                _logger(Logger::DETAIL) << "LinearStepperMotor::moveToPosition : Positive end switch on but moving in negative direction. Current position: " << getCurrentPosition() << std::endl;
            } else if (statusAndError.status == mtca4u::LinearStepperMotorStatusTypes::M_IN_MOVE || statusAndError.status == LinearStepperMotorStatusTypes::M_NOT_IN_POSITION) {
                continueWaiting = true;
                if (statusAndError.status == LinearStepperMotorStatusTypes::M_NOT_IN_POSITION) {
                    notInPositionCounter++;
                    usleep(1000);
                    if (currentMotorPosition == _motorControler->getActualPosition() && notInPositionCounter > 1000) {
                        _logger(Logger::ERROR) << "LinearStepperMotor::moveToPosition(float) : Motor seems not to move after 1 second waiting. Current position. " << getCurrentPosition() << std::endl;
                        statusAndError.status = LinearStepperMotorStatusTypes::M_ERROR;
                        statusAndError.error = StepperMotorErrorTypes::M_NO_REACTION_ON_COMMAND;
                        continueWaiting = false;
                    }
                } else {
                    notInPositionCounter = 0;
                }
            } else {
                continueWaiting = false;
            }
            usleep(5000);
        } while (continueWaiting);

        return statusAndError;

    }

    StepperMotorCalibrationStatus LinearStepperMotor::calibrateMotor() {

        // we need to check whether any previously triggered calibration is ongoing - multi thread programs.
        if (_motorCalibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS) {
            _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Calibration already in progress. Returning. " << std::endl;
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

            _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Enable both end switches." << std::endl;
            _motorControler->getReferenceSwitchData().setPositiveSwitchEnabled(1);
            _motorControler->getReferenceSwitchData().setNegativeSwitchEnabled(1);

            // check status of the motor. If end switches are not avaliable we should get error condition.
            StepperMotorStatus status = this->getStatus();
            if (status == LinearStepperMotorStatusTypes::M_ERROR) {
                _logger(Logger::ERROR) << "LinearStepperMotor::calibrateMotor : Motor in error state. Error is: " << this->getError() << std::endl;
            }


            bool takeFlagInAccount = false;
            do {
                //get current position and send motor to new lower position
                int currentPosition = _motorControler->getActualPosition();
                float newPosInUnits = recalculateStepsToUnits(currentPosition - 500000);
                _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Current position (steps): " << currentPosition << " (units): " << recalculateStepsToUnits(currentPosition) << " New position (steps): " << currentPosition - 10000 << " (units): " << newPosInUnits << std::endl;
                moveToPosition(newPosInUnits);

                if (_stopMotorCalibration == true) {
                    _stopMotorCalibration = false;
                    _logger(Logger::WARNING) << "LinearStepperMotor::calibrateMotor : Calibration stopped by user." << std::endl;
                    _softwareLimitsEnabled = originalSoftwareLimitEnabled;
                    _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER;
                    return _motorCalibrationStatus;
                }

                if (_motorStatus == LinearStepperMotorStatusTypes::M_IN_MOVE || _motorStatus == LinearStepperMotorStatusTypes::M_NOT_IN_POSITION) {
                    takeFlagInAccount = true;
                }

                if (takeFlagInAccount && _motorStatus == LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON) {
                    _logger(Logger::ERROR) << "LinearStepperMotor::calibrateMotor : Positive end switch reached when negative expected. Calibration failed.\n";
                    _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
                    _softwareLimitsEnabled = originalSoftwareLimitEnabled;
                    return _motorCalibrationStatus;
                }
                if (_motorStatus == LinearStepperMotorStatusTypes::M_ERROR) {
                    _logger(Logger::ERROR) << "LinearStepperMotor::calibrateMotor : Motor in error state. Error is: " << this->getError() << std::endl;
                    _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
                    _softwareLimitsEnabled = originalSoftwareLimitEnabled;
                    return _motorCalibrationStatus;
                }
            } while (_motorStatus != LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON);

            _calibNegativeEndSwitchInSteps = _motorControler->getActualPosition();
            _calibNegativeEndSwitchInUnits = recalculateStepsToUnits(_calibNegativeEndSwitchInSteps);

            _logger(Logger::INFO)  << "LinearStepperMotor::calibrateMotor : Negative end switch reached at (in steps): " << _calibNegativeEndSwitchInSteps << " Sending to positive end switch.\n";


            _logger(Logger::DETAIL)  << "LinearStepperMotor::calibrateMotor : Current position is: " << _motorControler->getActualPosition() << ". Go to positive end switch.\n";


            takeFlagInAccount = false;
            do {
                //get current position and send motor to new lower position
                int currentPosition = _motorControler->getActualPosition();
                float newPosInUnits = recalculateStepsToUnits(currentPosition + 500000);
                _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Current position (steps): " << currentPosition << " (units): " << recalculateStepsToUnits(currentPosition) << " New position (steps): " << currentPosition + 10000 << " (units): " << newPosInUnits << std::endl;
                moveToPosition(newPosInUnits);

                if (_stopMotorCalibration == true) {
                    _stopMotorCalibration = false;
                    _logger(Logger::WARNING) << "LinearStepperMotor::calibrateMotor : StepperMotor::calibrateMotor : Motor stopped by user.\n" << std::flush;
                    _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER;
                    _softwareLimitsEnabled = originalSoftwareLimitEnabled;
                    return _motorCalibrationStatus;
                }

                if (_motorStatus == LinearStepperMotorStatusTypes::M_IN_MOVE || _motorStatus == LinearStepperMotorStatusTypes::M_NOT_IN_POSITION) {
                    takeFlagInAccount = true;
                }

                if (takeFlagInAccount && _motorStatus == LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON) {
                    _logger(Logger::ERROR) << "LinearStepperMotor::calibrateMotor : Negative end switch reached when positive expected. Calibration failed.\n";
                    _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED; // FIX ME - we should also store information why it happened
                    _softwareLimitsEnabled = originalSoftwareLimitEnabled;
                    return _motorCalibrationStatus;
                }

                if (_motorStatus == LinearStepperMotorStatusTypes::M_ERROR) {
                    _logger(Logger::ERROR) << "LinearStepperMotor::calibrateMotor : Motor in error state. Error is: " << this->getError() << std::endl;
                    _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
                    _softwareLimitsEnabled = originalSoftwareLimitEnabled;
                    return _motorCalibrationStatus;
                }
            } while (_motorStatus != LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON);

            _calibPositiveEndSwitchInSteps = _motorControler->getActualPosition();
            _calibPositiveEndSwitchInUnits = recalculateStepsToUnits(_calibPositiveEndSwitchInSteps);


            _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Negative end switch is at (steps):  " << _calibNegativeEndSwitchInSteps << " Positive end switch at (steps): " << _calibPositiveEndSwitchInSteps << std::endl;
            _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Negative end switch is at (units):  " << _calibNegativeEndSwitchInUnits << " Positive end switch at (units): " << _calibPositiveEndSwitchInUnits << std::endl;
            float newPos = (_calibPositiveEndSwitchInUnits + _calibNegativeEndSwitchInUnits) / 2;
            _logger(Logger::INFO) << "LinearStepperMotor::calibrateMotor : Sending motor to middle of range:  " << newPos << std::endl;

            this->moveToPosition(newPos);
            if (_motorStatus == mtca4u::LinearStepperMotorStatusTypes::M_ERROR) {
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
            } else {
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATED;
            }



        //in case when user will stop calibration when moving to the middle range between positive and negative end switch positions
        if (_stopMotorCalibration == true) {
            _stopMotorCalibration = false;
            _logger(Logger::WARNING) << "LinearStepperMotor::calibrateMotor : StepperMotor::calibrateMotor : Motor stopped by user.\n" << std::flush;
            _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER;
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
    
    LinearStepperMotorStatus LinearStepperMotor::getStatus() {
        this->determineMotorStatusAndError();
        return static_cast<LinearStepperMotorStatus>(_motorStatus);
    }

    LinearStepperMotorError LinearStepperMotor::getError() {
        this->determineMotorStatusAndError();
        return static_cast<LinearStepperMotorError> (_motorError);
    }
    
    
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // END OF GETTERS AND SETTERS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // PRIVATE METHODS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



    MotorStatusAndError LinearStepperMotor::determineMotorStatusAndError() {
        //boost::lock_guard<boost::mutex> lock_guard(mutex);
        

            _motorStatus = LinearStepperMotorStatusTypes::M_OK;
            _motorError = StepperMotorErrorTypes::M_NO_ERROR;

            _motorControler->isEnabled();
            
            
            //first check if disabled
            if (!_motorControler->isEnabled()) {
                _motorStatus = LinearStepperMotorStatusTypes::M_DISABLED;
                return MotorStatusAndError(_motorStatus, _motorError);
            }

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // ERROR CONDITION CHECK - it should be done first.
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            //check end switches
            bool positiveSwitchStatus = _motorControler->getReferenceSwitchData().getPositiveSwitchActive();
            bool negativeSwitchStatus = _motorControler->getReferenceSwitchData().getNegativeSwitchActive();

            //check if both end switches are on in the same time - this means error;    
            if (positiveSwitchStatus && negativeSwitchStatus) {

                positiveSwitchStatus = _motorControler->getReferenceSwitchData().getPositiveSwitchActive();
                negativeSwitchStatus = _motorControler->getReferenceSwitchData().getNegativeSwitchActive();
                
                 if (positiveSwitchStatus && negativeSwitchStatus) {
                    _motorStatus = LinearStepperMotorStatusTypes::M_ERROR;
                     _logger(Logger::ERROR) << "LinearStepperMotor::determineMotorStatusAndError(): Both end switches error detected. " << std::endl;
                    _motorError = LinearStepperMotorErrorTypes::M_BOTH_END_SWITCH_ON;

                    return MotorStatusAndError(_motorStatus, _motorError);
                 }
            }

            if (_softwareLimitsEnabled && _maxPositionLimit <= _minPositionLimit) {
                _motorStatus = LinearStepperMotorStatusTypes::M_ERROR;
                _motorError = StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX;

                return MotorStatusAndError(_motorStatus, _motorError);
            }

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // STATUS CHECK
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  

            if (positiveSwitchStatus) {
                _motorStatus = LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON;
                return MotorStatusAndError(_motorStatus, _motorError);
            }

            if (negativeSwitchStatus) {
                _motorStatus = LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON;
                return MotorStatusAndError(_motorStatus, _motorError);
            }

            if (_softwareLimitsEnabled) {
                if (_targetPositionInUnits >= _maxPositionLimit) {
                    _motorStatus = LinearStepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON;
                    return MotorStatusAndError(_motorStatus, _motorError);
                }

                if (_targetPositionInUnits <= _minPositionLimit) {
                    _motorStatus = LinearStepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON;
                    return MotorStatusAndError(_motorStatus, _motorError);
                }
            }

            if (_motorControler->getStatus().getStandstillIndicator() == 0) {
                _motorStatus = LinearStepperMotorStatusTypes::M_IN_MOVE;
                return MotorStatusAndError(_motorStatus, _motorError);
            }


            if (_targetPositionInSteps != _motorControler->getActualPosition()) {
                _motorStatus = LinearStepperMotorStatusTypes::M_NOT_IN_POSITION;
                return MotorStatusAndError(_motorStatus, _motorError);
            }
            
            return MotorStatusAndError(_motorStatus, _motorError);
    }

} 

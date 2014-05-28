#include "LinearStepperMotor.h"
#include "math.h"

#define DEBUG(x) if (_debugLevel>=x) *_debugStream 



namespace mtca4u {

    LinearStepperMotor::LinearStepperMotor(std::string motorDriverCardDeviceName, unsigned int motorDriverId, std::string motorDriverCardConfigFileName) : StepperMotor(motorDriverCardDeviceName, motorDriverId, motorDriverCardConfigFileName)  {

        //calibration
        _calibNegativeEndSwitchInUnits = _calibNegativeEndSwitchInSteps = 0;
        _calibPositiveEndSwitchInUnits = _calibPositiveEndSwitchInSteps = 0;
    }

    LinearStepperMotor::~LinearStepperMotor() {
    }


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // BLOCKING FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    StepperMotorStatus LinearStepperMotor::moveToPosition(float newPosition) throw () {
        try {
            this->setMotorPosition(newPosition);

            if (!this->getAutostart()) {
                this->startMotor();
            }

            int dummy = 0;
            bool continueWaiting = true;
            unsigned int notInPositionCounter = 0;
            do {

                determineMotorStatusAndError();
                DEBUG(DEBUG_MAX_DETAIL) << dummy++ << ") LinearStepperMotor::moveToPosition : Motor in move. Current position: " << this->getMotorPosition() << ". Target position: " << _targetPositionInSteps << ". Current status: " << _motorStatus << std::endl;

                int currentMotorPosition = _motorControler->getActualPosition();

                //just in case if we had communication error in the status check --- SHLULD BE REMOVED WHEN SPI COM WILL BE RELIABLE !!!  -- FIX ME
                if (_motorError == StepperMotorErrorTypes::M_COMMUNICATION_LOST) {
                    DEBUG(DEBUG_WARNING) << "LinearStepperMotor::moveToPosition : Communication error found. Retrying." << std::endl;
                    determineMotorStatusAndError();
                }

                if (_motorStatus == StepperMotorStatusTypes::M_ERROR) {
                    return _motorStatus;
                }

                if (_stopMotorForBlocking == true) {
                    continueWaiting = false;
                    _stopMotorForBlocking = false;
                    DEBUG(DEBUG_WARNING) << "LinearStepperMotor::moveToPosition : Motor stopped by user." << std::endl;
                    while (_motorStatus == StepperMotorStatusTypes::M_IN_MOVE || _motorStatus == StepperMotorStatusTypes::M_NOT_IN_POSITION) {
                        DEBUG(DEBUG_DETAIL) << "LinearStepperMotor::moveToPosition : Wait to change status from M_IN_MOVE/M_NOT_IN_POSITION to other." << std::endl;
                        determineMotorStatusAndError();
                    }
                } else if (_motorStatus == LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON && _targetPositionInSteps > currentMotorPosition) {
                    continueWaiting = true;
                    DEBUG(DEBUG_DETAIL) << "LinearStepperMotor::moveToPosition : Negative end switch on but moving in positive direction. Current position: " << getMotorPosition() << std::endl;
                } else if (_motorStatus == LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON && _targetPositionInSteps < currentMotorPosition) {
                    continueWaiting = true;
                    DEBUG(DEBUG_DETAIL) << "LinearStepperMotor::moveToPosition : Positive end switch on but moving in negative direction. Current position: " << getMotorPosition() << std::endl;
                } else if (_motorStatus == mtca4u::StepperMotorStatusTypes::M_IN_MOVE || _motorStatus == StepperMotorStatusTypes::M_NOT_IN_POSITION) {
                    continueWaiting = true;
                    if (_motorStatus == StepperMotorStatusTypes::M_NOT_IN_POSITION) {
                        notInPositionCounter++;
                        usleep(1000);
                        if (currentMotorPosition == _motorControler->getActualPosition() && notInPositionCounter > 1000) {
                            DEBUG(DEBUG_ERROR) << "LinearStepperMotor::moveToPosition(float) : Motor seems not to move after 1 second waiting. Current position. " << getMotorPosition() << std::endl;
                            _motorStatus = StepperMotorStatusTypes::M_ERROR;
                            _motorError = StepperMotorErrorTypes::M_NO_REACTION_ON_COMMAND;
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

            return _motorStatus;

        } catch (mtca4u::MotorDriverException& ex) {
            DEBUG(DEBUG_ERROR) << "LinearStepperMotor::moveToPosition(float) : mtca4u::MotorDriverException detected." << std::endl;
            _motorStatus = StepperMotorStatusTypes::M_ERROR;
            _motorError = StepperMotorErrorTypes::M_COMMUNICATION_LOST;
            return _motorStatus;
        }
    }

    StepperMotorCalibrationStatus LinearStepperMotor::calibrateMotor() throw () {

        // we need to check whether any previously triggered calibration is ongoing - multi thread programs.
        if (_motorCalibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS) {
            DEBUG(DEBUG_INFO) << "LinearStepperMotor::calibrateMotor : Calibration already in progress. Returning. " << std::endl;
            return _motorCalibrationStatus;
        }

        bool originalSoftwareLimitEnabled = _softwareLimitsEnabled;
        _softwareLimitsEnabled = false;

        try {
            _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS;

            if (_motorControler->getReferenceSwitchData().getPositiveSwitchEnabled() == false || _motorControler->getReferenceSwitchData().getNegativeSwitchEnabled() == false) {
                DEBUG(DEBUG_INFO) << "LinearStepperMotor::calibrateMotor : End switches not available. Calibration not possible. " << std::endl;
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE;
                return _motorCalibrationStatus;
            }

            //enable the motor
            DEBUG(DEBUG_INFO) << "LinearStepperMotor::calibrateMotor : Enable the motor driver." << std::endl;
            _motorControler->setEnabled(true);

            DEBUG(DEBUG_INFO) << "LinearStepperMotor::calibrateMotor : Enable both end switches." << std::endl;
            _motorControler->getReferenceSwitchData().setPositiveSwitchEnabled(1);
            _motorControler->getReferenceSwitchData().setNegativeSwitchEnabled(1);

            // check status of the motor. If end switches are not avaliable we should get error condition.
            StepperMotorStatus status = this->getMotorStatus();
            if (status == StepperMotorStatusTypes::M_ERROR) {
                DEBUG(DEBUG_ERROR) << "LinearStepperMotor::calibrateMotor : Motor in error state. Error is: " << this->getMotorError() << std::endl;
            }


            bool takeFlagInAccount = false;
            do {
                //get current position and send motor to new lower position
                int currentPosition = _motorControler->getActualPosition();
                float newPosInUnits = recalculateStepsToUnits(currentPosition - 10000);
                DEBUG(DEBUG_INFO) << "LinearStepperMotor::calibrateMotor : Current position (steps): " << currentPosition << " (units): " << recalculateStepsToUnits(currentPosition) << " New position (steps): " << currentPosition - 10000 << " (units): " << newPosInUnits << std::endl;
                moveToPosition(newPosInUnits);

                if (_stopMotorCalibration == true) {
                    _stopMotorCalibration = false;
                    DEBUG(DEBUG_WARNING) << "LinearStepperMotor::calibrateMotor : Calibration stopped by user." << std::endl;
                    _softwareLimitsEnabled = originalSoftwareLimitEnabled;
                    _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_STOPED_BY_USER;
                    return _motorCalibrationStatus;
                }

                if (_motorStatus == StepperMotorStatusTypes::M_IN_MOVE || _motorStatus == StepperMotorStatusTypes::M_NOT_IN_POSITION) {
                    takeFlagInAccount = true;
                }

                if (takeFlagInAccount && _motorStatus == LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON) {
                    DEBUG(DEBUG_ERROR) << "LinearStepperMotor::calibrateMotor : Positive end switch reached when negative expected. Calibration failed.\n";
                    _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
                    _softwareLimitsEnabled = originalSoftwareLimitEnabled;
                    return _motorCalibrationStatus;
                }
                if (_motorStatus == StepperMotorStatusTypes::M_ERROR) {
                    DEBUG(DEBUG_ERROR) << "LinearStepperMotor::calibrateMotor : Motor in error state. Error is: " << this->getMotorError() << std::endl;
                    _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
                    _softwareLimitsEnabled = originalSoftwareLimitEnabled;
                    return _motorCalibrationStatus;
                }
            } while (_motorStatus != LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON);

            _calibNegativeEndSwitchInSteps = _motorControler->getActualPosition();
            _calibNegativeEndSwitchInUnits = recalculateStepsToUnits(_calibNegativeEndSwitchInSteps);

            DEBUG(DEBUG_INFO) << "LinearStepperMotor::calibrateMotor : Negative end switch reached at (in steps): " << _calibNegativeEndSwitchInSteps << " Sending to positive end switch.\n";


            DEBUG(DEBUG_DETAIL) << "LinearStepperMotor::calibrateMotor : Current position is: " << _motorControler->getActualPosition() << ". Go to positive end switch.\n";


            takeFlagInAccount = false;
            do {
                //get current position and send motor to new lower position
                int currentPosition = _motorControler->getActualPosition();
                float newPosInUnits = recalculateStepsToUnits(currentPosition + 10000);
                DEBUG(DEBUG_INFO) << "LinearStepperMotor::calibrateMotor : Current position (steps): " << currentPosition << " (units): " << recalculateStepsToUnits(currentPosition) << " New position (steps): " << currentPosition + 10000 << " (units): " << newPosInUnits << std::endl;
                moveToPosition(newPosInUnits);

                if (_stopMotorCalibration == true) {
                    _stopMotorCalibration = false;
                    DEBUG(DEBUG_WARNING) << "LinearStepperMotor::calibrateMotor : StepperMotor::calibrateMotor : Motor stopped by user.\n" << std::flush;
                    _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_STOPED_BY_USER;
                    _softwareLimitsEnabled = originalSoftwareLimitEnabled;
                    return _motorCalibrationStatus;
                }

                if (_motorStatus == StepperMotorStatusTypes::M_IN_MOVE || _motorStatus == StepperMotorStatusTypes::M_NOT_IN_POSITION) {
                    takeFlagInAccount = true;
                }

                if (takeFlagInAccount && _motorStatus == LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON) {
                    DEBUG(DEBUG_ERROR) << "LinearStepperMotor::calibrateMotor : Negative end switch reached when positive expected. Calibration failed.\n";
                    _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED; // FIX ME - we should also store information why it happened
                    _softwareLimitsEnabled = originalSoftwareLimitEnabled;
                    return _motorCalibrationStatus;
                }

                if (_motorStatus == LinearStepperMotorStatusTypes::M_ERROR) {
                    DEBUG(DEBUG_ERROR) << "LinearStepperMotor::calibrateMotor : Motor in error state. Error is: " << this->getMotorError() << std::endl;
                    _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
                    _softwareLimitsEnabled = originalSoftwareLimitEnabled;
                    return _motorCalibrationStatus;
                }
            } while (_motorStatus != LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON);

            _calibPositiveEndSwitchInSteps = _motorControler->getActualPosition();
            _calibPositiveEndSwitchInUnits = recalculateStepsToUnits(_calibPositiveEndSwitchInSteps);


            DEBUG(DEBUG_INFO) << "LinearStepperMotor::calibrateMotor : Negative end switch is at (steps):  " << _calibNegativeEndSwitchInSteps << " Positive end switch at (steps): " << _calibPositiveEndSwitchInSteps << std::endl;
            DEBUG(DEBUG_INFO) << "LinearStepperMotor::calibrateMotor : Negative end switch is at (units):  " << _calibNegativeEndSwitchInUnits << " Positive end switch at (units): " << _calibPositiveEndSwitchInUnits << std::endl;
            float newPos = (_calibPositiveEndSwitchInUnits + _calibNegativeEndSwitchInUnits) / 2;
            DEBUG(DEBUG_INFO) << "LinearStepperMotor::calibrateMotor : Sending motor to middle of range:  " << newPos << std::endl;

            this->moveToPosition(newPos);
            if (_motorStatus == mtca4u::StepperMotorStatusTypes::M_ERROR) {
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
            } else {
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATED;
            }

        } catch (mtca4u::MotorDriverException& ex) {
            DEBUG(DEBUG_ERROR) << "LinearStepperMotor::calibrateMotor : mtca4u::MotorDriverException detected." << std::endl;
            _motorStatus = StepperMotorStatusTypes::M_ERROR;
            _motorError = StepperMotorErrorTypes::M_COMMUNICATION_LOST;
            _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;

        }

        //in case when user will stop calibration when moving to the middle range between positive and negative end switch positions
        if (_stopMotorCalibration == true) {
            _stopMotorCalibration = false;
            DEBUG(DEBUG_WARNING) << "LinearStepperMotor::calibrateMotor : StepperMotor::calibrateMotor : Motor stopped by user.\n" << std::flush;
            _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_STOPED_BY_USER;
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



    void LinearStepperMotor::setCurrentMotorPositionAs(float newPosition) {
        if (_motorCalibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATED) {
            //just to update current position readout
            getMotorPosition();
            //calculate difference between current position and new wanted position
            float delta = _currentPostionsInUnits - newPosition;

            DEBUG(DEBUG_INFO) << "LinearStepperMotor::setCurrentMotorPositionAs : Motor calibrated. Recalculating end switches position." << std::endl;
            DEBUG(DEBUG_INFO) << "LinearStepperMotor::setCurrentMotorPositionAs : Current position: " << _currentPostionsInUnits << ", in steps: " << _currentPostionsInSteps << std::endl;
            DEBUG(DEBUG_INFO) << "LinearStepperMotor::setCurrentMotorPositionAs : Wanted  position: " << newPosition << ", in steps: " << this->recalculateUnitsToSteps(newPosition) << std::endl;
            DEBUG(DEBUG_INFO) << "LinearStepperMotor::setCurrentMotorPositionAs : Delta   position: " << delta << ", in steps: " << this->recalculateUnitsToSteps(delta) << std::endl;
            DEBUG(DEBUG_INFO) << "LinearStepperMotor::setCurrentMotorPositionAs : Negative end switch (in steps): " << _calibNegativeEndSwitchInSteps << " positive end switch(in steps): " << _calibPositiveEndSwitchInSteps << std::endl;

            _calibNegativeEndSwitchInUnits -= delta;
            _calibNegativeEndSwitchInSteps = recalculateUnitsToSteps(_calibNegativeEndSwitchInUnits);

            _calibPositiveEndSwitchInUnits -= delta;
            _calibPositiveEndSwitchInSteps = recalculateUnitsToSteps(_calibPositiveEndSwitchInUnits);
            DEBUG(DEBUG_INFO) << "LinearStepperMotor::setCurrentMotorPositionAs :Recalculated. Negative end switch (in steps): " << _calibNegativeEndSwitchInSteps << " positive end switch(in steps): " << _calibPositiveEndSwitchInSteps << std::endl;
        }
        
        StepperMotor::setCurrentMotorPositionAs(newPosition);
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

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // END OF GETTERS AND SETTERS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // PRIVATE METHODS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



    void LinearStepperMotor::determineMotorStatusAndError() throw () {
        boost::lock_guard<boost::mutex> lock_guard(mutex);
        
        try {
            _motorStatus = LinearStepperMotorStatusTypes::M_OK;
            _motorError = StepperMotorErrorTypes::M_NO_ERROR;

            _motorControler->isEnabled();
            
            
            //first check if disabled
            if (!_motorControler->isEnabled()) {
                _motorStatus = LinearStepperMotorStatusTypes::M_DISABLED;
                return;
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
                     DEBUG(DEBUG_ERROR) << "LinearStepperMotor::determineMotorStatusAndError(): Both end switches error detected. " << std::endl;
                    _motorError = StepperMotorErrorTypes::M_BOTH_END_SWITCH_ON;

                    return;
                 }
            }

            if (_softwareLimitsEnabled && _maxPositionLimit <= _minPositionLimit) {
                _motorStatus = LinearStepperMotorStatusTypes::M_ERROR;
                _motorError = StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX;

                return;
            }

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // STATUS CHECK
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  

            if (positiveSwitchStatus) {
                _motorStatus = LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON;
                return;
            }

            if (negativeSwitchStatus) {
                _motorStatus = LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON;
                return;
            }

            if (_softwareLimitsEnabled) {
                if (_targetPositionInUnits >= _maxPositionLimit) {
                    _motorStatus = LinearStepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON;
                    return;
                }

                if (_targetPositionInUnits <= _minPositionLimit) {
                    _motorStatus = LinearStepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON;
                    return;
                }
            }

            if (_motorControler->getStatus().getStandstillIndicator() == 0) {
                _motorStatus = LinearStepperMotorStatusTypes::M_IN_MOVE;
                return;
            }


            if (_targetPositionInSteps != _motorControler->getActualPosition()) {
                _motorStatus = LinearStepperMotorStatusTypes::M_NOT_IN_POSITION;
                return;
            }
        } catch (mtca4u::MotorDriverException& ex) {
            DEBUG(DEBUG_ERROR) << "LinearStepperMotor::determineMotorStatusAndError : mtca4u::MotorDriverException detected." << std::endl;
            _motorStatus = LinearStepperMotorStatusTypes::M_ERROR;
            _motorError = StepperMotorErrorTypes::M_COMMUNICATION_LOST;
        }

    }

} 

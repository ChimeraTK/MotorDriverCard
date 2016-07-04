#include "StepperMotor.h"
#include <cmath>
#include "MotorDriverCardFactory.h"
#include "MotorDriverException.h"

namespace mtca4u {


    	StepperMotor::StepperMotor(std::string const & motorDriverCardDeviceName, std::string const & moduleName, unsigned int motorDriverId, std::string motorDriverCardConfigFileName) {
        _motorDriverId = motorDriverId;
        _motorDriverCardDeviceName = motorDriverCardDeviceName;

        //std::string deviceFileName(DMapFilesParser(pathToDmapFile).getdMapFileElem(_motorDriverCardDeviceName).dev_file);
        //std::string mapFileName(DMapFilesParser(pathToDmapFile).getdMapFileElem(_motorDriverCardDeviceName).map_file_name);

        /*_motorDriverCard = MotorDriverCardFactory::instance().createMotorDriverCard(
                                deviceFileName, mapFileName, moduleName, motorDriverCardConfigFileName);*/

        _motorDriverCard = MotorDriverCardFactory::instance().createMotorDriverCard(
        		motorDriverCardDeviceName, moduleName, motorDriverCardConfigFileName);




        _motorControler = _motorDriverCard->getMotorControler(_motorDriverId);

        //position limits
        _maxPositionLimit = std::numeric_limits<float>::max();
        _minPositionLimit = -std::numeric_limits<float>::max();

        // position - don't know so set to 0
        _currentPostionsInSteps = 0;
        _currentPostionsInUnits = 0;

        _targetPositionInSteps = 0;
        _targetPositionInUnits = 0;

        _autostartFlag = false;

        _stopMotorForBlocking = false;
        _blockingFunctionActive = false;

        _softwareLimitsEnabled = true;
        _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_NOT_CALIBRATED;
    }

    StepperMotor::~StepperMotor() {
    }


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // BLOCKING FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    StepperMotorStatusAndError StepperMotor::moveToPosition(float newPosition) {
        
        StepperMotorStatusAndError statusAndError = this->determineMotorStatusAndError();
        if (statusAndError.status == StepperMotorStatusTypes::M_ERROR || statusAndError.status == StepperMotorStatusTypes::M_DISABLED) {
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

    StepperMotorCalibrationStatus StepperMotor::calibrateMotor() {
        _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE;
        return _motorCalibrationStatus;
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // END OF BLOCKING FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // GENERAL FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    void StepperMotor::start() {
      // allowing it when status is MOTOR_IN_MOVE give a possibility to change
      // target position during motor movement
      _motorControler->setTargetPosition(_targetPositionInSteps);
    }

    void StepperMotor::stop() {
        //stopping is done by reading real position and setting it as target
        //one. In reality it can cause that motor will stop and move in reverse
        //direction by couple steps Amount of steps done in reverse direction
        //depends on motor speed and general delay in motor control path.

        if (_blockingFunctionActive)
            _stopMotorForBlocking = true;
        else
            _stopMotorForBlocking = false;

        int currentPosition = _motorControler->getActualPosition();
        _motorControler->setTargetPosition(currentPosition);
        _targetPositionInSteps = currentPosition;
        _targetPositionInUnits = recalculateStepsToUnits(currentPosition);

    }

    void StepperMotor::emergencyStop() {
        //Emergency stop is done disabling motor driver. It can cause lost of calibration for the motor. After emergency stop calibration flag will be set to FALSE
        _motorControler->setEnabled(false);
        // we can execute stop function to stop internal controller counter as close to the emergency stop position. Moreover, it will also interrupt blocking function.
        this->stop();
    }

    int StepperMotor::recalculateUnitsToSteps(float units) {
        if (!_stepperMotorUnitsConverter)
            return static_cast<int> (units);

        return _stepperMotorUnitsConverter.get()->unitsToSteps(units);
    }

    float StepperMotor::recalculateStepsToUnits(int steps) {

        if (!_stepperMotorUnitsConverter)
            return static_cast<float> (steps);

        return _stepperMotorUnitsConverter.get()->stepsToUnits(steps);
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // END OF GENERAL FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // GETTERS AND SETTERS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  

    StepperMotorCalibrationStatus StepperMotor::getCalibrationStatus() {
        return _motorCalibrationStatus;
    }

    void StepperMotor::setSoftwareLimitsEnabled(bool newVal) {
        _softwareLimitsEnabled = newVal;
    }

    bool StepperMotor::getSoftwareLimitsEnabled() const {
        return _softwareLimitsEnabled;
    }

    void StepperMotor::setCurrentPositionAs(float newPosition) {
        _currentPostionsInUnits = newPosition;
        _targetPositionInUnits = newPosition;

        _currentPostionsInSteps = this->recalculateUnitsToSteps(newPosition);
        _targetPositionInSteps = this->recalculateUnitsToSteps(newPosition);
        bool enable = _motorControler->isEnabled();
        _motorControler->setEnabled(false);
        _motorControler->setActualPosition(_currentPostionsInSteps);
        _motorControler->setTargetPosition(_currentPostionsInSteps);
        _motorControler->setEnabled(enable);

    }

    void StepperMotor::setTargetPosition(float newPosition) {
        this->determineMotorStatusAndError();
        if (_motorError == StepperMotorErrorTypes::M_NO_ERROR) {
            float position = truncateMotorPosition(newPosition);
            _targetPositionInUnits = position;
            _targetPositionInSteps = this->recalculateUnitsToSteps(position);
            if (_autostartFlag) {
                this->start();
            }
        }
    }

    float StepperMotor::getTargetPosition() const {
        return _targetPositionInUnits;
    }

    float StepperMotor::getCurrentPosition() {
        _currentPostionsInSteps = _motorControler->getActualPosition();
        _currentPostionsInUnits = this->recalculateStepsToUnits(_currentPostionsInSteps);
        return _currentPostionsInUnits;
    }

    void StepperMotor::setStepperMotorUnitsConverter(boost::shared_ptr<StepperMotorUnitsConverter> stepperMotorUnitsConverter) {
        _stepperMotorUnitsConverter = stepperMotorUnitsConverter;
    }

    double StepperMotor::setUserSpeedLimit(double newSpeed) {
      return _motorControler->setUserSpeedLimit(newSpeed);
    }

    void StepperMotor::setMaxPositionLimit(float maxPos) {
        _maxPositionLimit = maxPos;
    }

    void StepperMotor::setMinPositionLimit(float minPos) {
        _minPositionLimit = minPos;
    }

    float StepperMotor::getMaxPositionLimit() {
        return _maxPositionLimit;
    }

    float StepperMotor::getMinPositionLimit() {
        return _minPositionLimit;
    }

    StepperMotorStatusAndError StepperMotor::getStatusAndError() {
        return determineMotorStatusAndError();
    }
    
    bool StepperMotor::getAutostart() {
        return _autostartFlag;
    }

    void StepperMotor::setAutostart(bool autostart) {
        _autostartFlag = autostart;

        // if flag set to true we need to synchronize target position with from class with one in hardware
        if (_autostartFlag)
            this->start();
    }

    void StepperMotor::setEnabled(bool enable) {
        _motorControler->setEnabled(enable);
    }

    bool StepperMotor::getEnabled() const {
        return _motorControler->isEnabled();
    }

    void StepperMotor::setLogLevel(Logger::LogLevel newLevel) {
        _logger.setLogLevel(newLevel);
    }

    Logger::LogLevel StepperMotor::getLogLevel() {
        return _logger.getLogLevel();
    }

    double StepperMotor::getMaxSpeedCapability() {
      return _motorControler->getMaxSpeedCapability();
    }

    double StepperMotor::setUserCurrentLimit(double currentInAmps) {
      return (_motorControler->setUserCurrentLimit(currentInAmps));
    }

    double StepperMotor::getUserCurrentLimit() {
      return (_motorControler->getUserCurrentLimit());
    }

    double StepperMotor::getSafeCurrentLimit() {
      return (_motorControler->getMaxCurrentLimit());
    }

    double StepperMotor::getUserSpeedLimit() {
      return (_motorControler->getUserSpeedLimit());
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // END OF GETTERS AND SETTERS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // PRIVATE METHODS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    float StepperMotor::truncateMotorPosition(float newPosition) {
        float position = newPosition;
        
        if (!_softwareLimitsEnabled)
            return position;

        // truncate new position according to the internal limits.        
        if (position > _maxPositionLimit) {
            position = _maxPositionLimit;
        } else if (position < _minPositionLimit) {
            position = _minPositionLimit;
        }

        return position;
    }

    StepperMotorStatusAndError StepperMotor::determineMotorStatusAndError() {

        //InternalMutex intMutex(this);
        boost::lock_guard<boost::mutex> guard(_mutex);
        
        _motorStatus = StepperMotorStatusTypes::M_OK;
        _motorError = StepperMotorErrorTypes::M_NO_ERROR;

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // ERROR CONDITION CHECK - it should be done first.
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


        if (_softwareLimitsEnabled && (_maxPositionLimit <= _minPositionLimit)) {
            _motorStatus = StepperMotorStatusTypes::M_ERROR;
            _motorError = StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX;

            return StepperMotorStatusAndError(_motorStatus, _motorError);
        }

        //first check if disabled
        if (!_motorControler->isEnabled()) {
            _motorStatus = StepperMotorStatusTypes::M_DISABLED;
            return StepperMotorStatusAndError(_motorStatus, _motorError);
        }

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // STATUS CHECK
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  

        if (_softwareLimitsEnabled) {
            if (_targetPositionInUnits >= _maxPositionLimit) {
                _motorStatus = StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON;
                return StepperMotorStatusAndError(_motorStatus, _motorError);
            }

            if (_targetPositionInUnits <= _minPositionLimit) {
                _motorStatus = StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON;
                return StepperMotorStatusAndError(_motorStatus, _motorError);
            }
        }

        if (_motorControler->getStatus().getStandstillIndicator() == 0) {
            _motorStatus = StepperMotorStatusTypes::M_IN_MOVE;
            return StepperMotorStatusAndError(_motorStatus, _motorError);
        }
         

        if (_targetPositionInSteps != _motorControler->getActualPosition()) {
            _motorStatus = StepperMotorStatusTypes::M_NOT_IN_POSITION;
            return StepperMotorStatusAndError(_motorStatus, _motorError);
        }

        return StepperMotorStatusAndError(_motorStatus, _motorError);
    }
    }

    bool mtca4u::StepperMotor::isMoving() {
      return (_motorControler->isMotorMoving());
    }

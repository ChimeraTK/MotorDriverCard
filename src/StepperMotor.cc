#include "StepperMotor.h"
#include "math.h"

#define DEBUG(x) if (_debugLevel>=x) *_debugStream 



namespace mtca4u {

    boost::mutex StepperMotor::mutex;
    
    StepperMotor::StepperMotor(std::string motorDriverCardDeviceName, unsigned int motorDriverId, std::string motorDriverCardConfigFileName) {
        // save basics infos
        _motorDriverId = motorDriverId;
        _motorDriverCardDeviceName = motorDriverCardDeviceName;
        _motorDriverCardConfigFileName = motorDriverCardConfigFileName;

        //open device and prepare to initialize MotorDriverCardImpl and MotorControler objects
        //this->hw_device->openDev(dmapFilesParser(".").getdMapFileElem(dev_name_lsp_->value()).getDeviceFileAndMapFileName());


        //std::pair<std::string, std::string> deviceFileAndMapFileName = dmapFileParser().parse(_motorDriverCardDeviceName)->begin()->getDeviceFileAndMapFileName();
        std::string devicePath(dmapFilesParser(".").getdMapFileElem(_motorDriverCardDeviceName).dev_file);
        std::string deviceMapFileName(dmapFilesParser(".").getdMapFileElem(_motorDriverCardDeviceName).map_file_name);

        boost::shared_ptr< devPCIE > ioDevice(new devPCIE);
        ioDevice->openDev(devicePath);

        boost::shared_ptr< mapFile > registerMapping = mapFileParser().parse(deviceMapFileName);

        boost::shared_ptr< devMap< devBase > > mappedDevice(new devMap<devBase>);
        mappedDevice->openDev(ioDevice, registerMapping);

        _motorDriverCardConfig = MotorDriverCardConfigXML::read(_motorDriverCardConfigFileName);

        _motorDriverCardPtr.reset(new MotorDriverCardImpl(mappedDevice, _motorDriverCardConfig));


        //_motorControler.reset(&_motorDriverCardPtr->getMotorControler(_motorDriverId));      
        _motorControler = _motorDriverCardPtr->getMotorControler(_motorDriverId).get();

        //position limits
        _maxPositionLimit = std::numeric_limits<float>::max();
        _minPositionLimit = -std::numeric_limits<float>::max();

        //pointer to units to step converter object
        _stepperMotorUnitsConverter = NULL;

        // position - don't know so set to 0
        _currentPostionsInSteps = 0;
        _currentPostionsInUnits = 0;


        _targetPositionInSteps = 0;
        _targetPositionInUnits = 0;

        _autostartFlag = false;

        _stopMotorForBlocking = false;
        _stopMotorCalibration = false;

        _debugLevel = DEBUG_OFF;
        _debugStream = &std::cout;

        _softwareLimitsEnabled = true;
        _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_UNKNOWN;

    }

    StepperMotor::~StepperMotor() {
        //delete _motorDriverCardPtr;
    }


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // BLOCKING FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    StepperMotorStatus StepperMotor::moveToPosition(float newPosition) throw () {
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
                DEBUG(DEBUG_MAX_DETAIL) << dummy++ << ") StepperMotor::moveToPosition : Motor in move. Current position: " << this->getMotorPosition() << ". Target position: " << _targetPositionInSteps << ". Current status: " << _motorStatus << std::endl;

                int currentMotorPosition = _motorControler->getActualPosition();

                //just in case if we had communication error in the status check --- SHLULD BE REMOVED WHEN SPI COM WILL BE RELIABLE !!!  -- FIX ME
                if (_motorError == StepperMotorErrorTypes::M_COMMUNICATION_LOST) {
                    DEBUG(DEBUG_WARNING) << "StepperMotor::moveToPosition : Communication error found. Retrying." << std::endl;
                    this->determineMotorStatusAndError();
                }

                if (_motorStatus == StepperMotorStatusTypes::M_ERROR) {
                    return _motorStatus;
                }

                if (_stopMotorForBlocking == true) {
                    continueWaiting = false;
                    _stopMotorForBlocking = false;
                    DEBUG(DEBUG_WARNING) << "StepperMotor::moveToPosition : Motor stopped by user." << std::endl;
                    while (_motorStatus == StepperMotorStatusTypes::M_IN_MOVE || _motorStatus == StepperMotorStatusTypes::M_NOT_IN_POSITION) {
                        DEBUG(DEBUG_DETAIL) << "StepperMotor::moveToPosition : Wait to change status from M_IN_MOVE/M_NOT_IN_POSITION to other." << std::endl;
                        this->determineMotorStatusAndError();
                    }
                } else if (_motorStatus == mtca4u::StepperMotorStatusTypes::M_IN_MOVE || _motorStatus == StepperMotorStatusTypes::M_NOT_IN_POSITION) {
                    continueWaiting = true;
                    if (_motorStatus == StepperMotorStatusTypes::M_NOT_IN_POSITION) {
                        notInPositionCounter++;
                        usleep(1000);
                        if (currentMotorPosition == _motorControler->getActualPosition() && notInPositionCounter > 1000) {
                            DEBUG(DEBUG_ERROR) << "StepperMotor::moveToPosition(float) : Motor seems not to move after 1 second waiting. Current position. " << getMotorPosition() << std::endl;
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
            DEBUG(DEBUG_ERROR) << "StepperMotor::moveToPosition(float) : mtca4u::MotorDriverException detected." << std::endl;
            _motorStatus = StepperMotorStatusTypes::M_ERROR;
            _motorError = StepperMotorErrorTypes::M_COMMUNICATION_LOST;
            return _motorStatus;
        }
    }

    StepperMotorCalibrationStatus StepperMotor::calibrateMotor() throw () {
        _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE;
        return _motorCalibrationStatus;
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // END OF BLOCKING FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // GENERAL FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    void StepperMotor::startMotor() {
        // allowing it when status is MOTOR_IN_MOVE give a possibility to change target position during motor movement
        //StepperMotorStatus status = this->getMotorStatus();
        //if (status == StepperMotorStatusTypes::M_OK || status == StepperMotorStatusTypes::M_IN_MOVE)
        _motorControler->setTargetPosition(_targetPositionInSteps);

    }

    void StepperMotor::stopMotor() {
        //stopping is done by reading real position and setting it as target one. In reality it can cause that motor will stop and move in reverse direction by couple steps
        //Amount of steps done in reverse direction depends on motor speed and general delay in motor control path

        if (getMotorStatus() == StepperMotorStatusTypes::M_IN_MOVE)
            _stopMotorForBlocking = true;
        if (getCalibrationStatus() == StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS)
            _stopMotorCalibration = true;

        int currentPosition = _motorControler->getActualPosition();
        _motorControler->setTargetPosition(currentPosition);
        _targetPositionInSteps = currentPosition;
        _targetPositionInUnits = recalculateStepsToUnits(currentPosition);

    }

    void StepperMotor::stopMotorEmergency() {
        //Emergency stop is done disabling motor driver. It can cause lost of calibration for the motor. After emergency stop calibration flag will be set to FALSE
        _motorControler->setEnabled(false);
        // we can execute stopMotor function to stop internal controller counter as close to the emergency stop position. Moreover, it will also interrupt blocking function.
        this->stopMotor();
        this->_motorCalibrationStatus = StepperMotorCalibrationStatusType::M_NOT_CALIBRATED;
    }

    int StepperMotor::recalculateUnitsToSteps(float units) {
        if (_stepperMotorUnitsConverter == NULL)
            return static_cast<int> (units);

        return _stepperMotorUnitsConverter->unitsToSteps(units);

    }

    float StepperMotor::recalculateStepsToUnits(int steps) {
        if (_stepperMotorUnitsConverter == NULL)
            return static_cast<float> (steps);

        return _stepperMotorUnitsConverter->stepsToUnits(steps);
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

    void StepperMotor::setCurrentMotorPositionAs(float newPosition) {

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

    void StepperMotor::setMotorPosition(float newPosition) {
        StepperMotorError error = this->getMotorError();

        if (error == StepperMotorErrorTypes::M_NO_ERROR) {
            float position = truncateMotorPosition(newPosition);
            _targetPositionInUnits = position;
            _targetPositionInSteps = this->recalculateUnitsToSteps(position);
            if (_autostartFlag) {
                this->startMotor();
            }
        }
    }

    float StepperMotor::getMotorPosition() {
        _currentPostionsInSteps = _motorControler->getActualPosition();
        _currentPostionsInUnits = this->recalculateStepsToUnits(_currentPostionsInSteps);
        return _currentPostionsInUnits;
    }

    void StepperMotor::setStepperMotorUnitsConverter(StepperMotorUnitsConverter* stepperMotorUnitsConverter) {
        _stepperMotorUnitsConverter = stepperMotorUnitsConverter;
    }

    // Speed setting

    void StepperMotor::setMotorSpeed(float newSpeed) {
        //FIX ME
        int ns = static_cast<int> (fabs(newSpeed));
   
    }

    float StepperMotor::getMotorSpeed() {
        //FIX ME
        return static_cast<float> (0);
    }

    //Software limits settings

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

    StepperMotorStatus StepperMotor::getMotorStatus() throw () {
        this->determineMotorStatusAndError();
        return _motorStatus;
    }

    StepperMotorError StepperMotor::getMotorError() throw () {
        this->determineMotorStatusAndError();
        return _motorError;
    }


    //autostart 

    bool StepperMotor::getAutostart() {
        return _autostartFlag;
    }

    void StepperMotor::setAutostart(bool autostart) {
        _autostartFlag = autostart;

        // if flag set to true we need to synchronize target position with from class with one in hardware
        if (_autostartFlag)
            this->startMotor();
    }

    void StepperMotor::setEnable(bool enable) {
        _motorControler->setEnabled(enable);
    }

    bool StepperMotor::getEnabled() const {
        return _motorControler->isEnabled();
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

    void StepperMotor::determineMotorStatusAndError() throw () {

        try {
            _motorStatus = StepperMotorStatusTypes::M_OK;
            _motorError = StepperMotorErrorTypes::M_NO_ERROR;
            
            
            //first check if disabled
            if (!_motorControler->isEnabled()) {
                _motorStatus = StepperMotorStatusTypes::M_DISABLED;
                return;
            }

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // ERROR CONDITION CHECK - it should be done first.
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 

            if (_softwareLimitsEnabled && _maxPositionLimit <= _minPositionLimit) {
                _motorStatus = StepperMotorStatusTypes::M_ERROR;
                _motorError = StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX;

                return;
            }

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // STATUS CHECK
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  

            if (_softwareLimitsEnabled) {
                if (_targetPositionInUnits >= _maxPositionLimit) {
                    _motorStatus = StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON;
                    return;
                }

                if (_targetPositionInUnits <= _minPositionLimit) {
                    _motorStatus = StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON;
                    return;
                }
            }

            if (_motorControler->getStatus().getStandstillIndicator() == 0) {
                _motorStatus = StepperMotorStatusTypes::M_IN_MOVE;
                return;
            }


            if (_targetPositionInSteps != _motorControler->getActualPosition()) {
                _motorStatus = StepperMotorStatusTypes::M_NOT_IN_POSITION;
                return;
            }
        } catch (mtca4u::MotorDriverException& ex) {
            DEBUG(DEBUG_ERROR) << "StepperMotor::determineMotorStatusAndError : mtca4u::MotorDriverException detected." << std::endl;
            _motorStatus = StepperMotorStatusTypes::M_ERROR;
            _motorError = StepperMotorErrorTypes::M_COMMUNICATION_LOST;
        }

    }

    //

    void StepperMotor::setDebugLevel(unsigned int newLevel) {
        _debugLevel = newLevel;
    }

    unsigned int StepperMotor::getDebugLevel() {
        return _debugLevel;
    }

    void StepperMotor::setDebugStream(std::ostream* debugStream) {
        _debugStream = debugStream;
    }
} 
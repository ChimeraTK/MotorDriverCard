#include "StepperMotor.h"
#include "math.h"

#define DEBUG_ON
#define DEBUG_LEVEL 1


#define MAX_MOTOR_POS_COUNTS 0x7FFFFF
#define MIN_MOTOR_POS_COUNTS 0x100000

namespace mtca4u {



    StepperMotorError StepperMotorErrorTypes::MOTOR_NO_ERROR(0, "No error");
    StepperMotorError StepperMotorErrorTypes::MOTOR_BOTH_ENDSWICHTED_ON(1, "Both hardware end switches on");
    StepperMotorError StepperMotorErrorTypes::MOTOR_COMMUNICATION_LOST(2, "Communication with motor lost");


    StepperMotorStatus StepperMotorStatusTypes::MOTOR_OK(0, "OK");
    StepperMotorStatus StepperMotorStatusTypes::MOTOR_DISABLED(1, "DISABLED");
    StepperMotorStatus StepperMotorStatusTypes::MOTOR_IN_MOVE(2, "IN MOVE");
    StepperMotorStatus StepperMotorStatusTypes::MOTOR_POSITIVE_END_SWITCHED_ON(4, "HARDWARE POSITIVE END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::MOTOR_NEGATIVE_END_SWITCHED_ON(8, "HARDWARE NEGATIVE END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::MOTOR_SOFT_PLUS_END_SWITCHED_ON(16, "SOFTWARE PLUS END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::MOTOR_SOFT_MINUS_END_SWITCHED_ON(32, "SOFTWARE MINUS END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::MOTOR_IN_ERROR(64, "ERROR");
    StepperMotorStatus StepperMotorStatusTypes::MOTOR_CONFIGURATION_ERROR(128, "MOTOR_CONFIGURATION_ERROR");

    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::MOTOR_CALIBRATION_UNKNOWN(0, "UNKNOWN");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::MOTOR_CALIBRATED(1, "CALIBRATED");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::MOTOR_NOT_CALIBRATED(2, "NOT CALIBRATED");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::MOTOR_CALIBRATION_FAILED(4, "FAILED");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::MOTOR_CALIBRATION_IN_PROGRESS(8, "CALIBRATION IN PROGRESS");

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
        _motorControler = &_motorDriverCardPtr->getMotorControler(_motorDriverId);


        //position limits
        _maxPositionLimit = std::numeric_limits<float>::max();
        _minPositionLimit = -std::numeric_limits<float>::max();


        // pointers to conversion functions
        unitsToSteps = NULL;
        stepsToUnits = NULL;


        // position - don't know so set to 0
        _currentPostionsInSteps = 0;
        _currentPostionsInUnits = 0;

        _setPositionInSteps = 0;
        _setPositionInUnits = 0;

        _autostartFlag = false;

    }

    StepperMotor::~StepperMotor() {
        //delete _motorDriverCardPtr;
    }

    StepperMotorStatus StepperMotor::moveToPosition(float newPosition) {
        
        StepperMotorStatus status = this->getMotorStatus();

        this->setMotorPosition(newPosition);

        if (!this->getAutostart()) {
            this->startMotor();
        }

        int dummy = 0;
        bool continueWaiting = true;
       
        do {
            status = this->getMotorStatus();
            //std::cout << dummy++ << ") StepperMotor::moveToPosition : Motor in move. Current position:" << this->getMotorPosition() << "\n";
            if (status == StepperMotorStatusTypes::MOTOR_NEGATIVE_END_SWITCHED_ON && _setPositionInSteps > _motorControler->getActualPosition()) {
                continueWaiting = true;
                //std::cout << "StepperMotor::moveToPosition : Negative end switch on but moving in positive direction.\n";
            } else if (status == StepperMotorStatusTypes::MOTOR_POSITIVE_END_SWITCHED_ON && _setPositionInSteps < _motorControler->getActualPosition()) {
                continueWaiting = true;
                //std::cout << "StepperMotor::moveToPosition : Positive end switch on but moving in negative direction.\n";
            } else if (status == mtca4u::StepperMotorStatusTypes::MOTOR_IN_MOVE) {
                continueWaiting = true;
            } else {
                 continueWaiting = false;
            }
        } while (continueWaiting);

        std::cout << "StepperMotor::moveToPosition : Motor status after wait ending: " << status << "\n";

        return this->getMotorStatus();

    }

    StepperMotorCalibrationStatus StepperMotor::calibrateMotor() {
        _motorCalibrationStatus = StepperMotorCalibrationStatusType::MOTOR_CALIBRATION_IN_PROGRESS;
        //enable the motor
        _motorControler->setEnabled(true);
        //set current position to maximum
        _motorControler->setActualPosition(MAX_MOTOR_POS_COUNTS);
        //sending motor to the end switch
        _motorControler->setTargetPosition(MIN_MOTOR_POS_COUNTS);
        int dummy = 0;
        bool takeFlagInAccount = false;
        do {
            determineMotorStatusAndError();

            if (_motorStatus == StepperMotorStatusTypes::MOTOR_IN_MOVE) {
                takeFlagInAccount = true;
            }
            
            if (takeFlagInAccount && _motorStatus == StepperMotorStatusTypes::MOTOR_POSITIVE_END_SWITCHED_ON) {
                std::cout << "Positive end switch reached when negative expected.\n";
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::MOTOR_CALIBRATION_FAILED;
                return _motorCalibrationStatus;
            }
            if (_motorStatus == StepperMotorStatusTypes::MOTOR_IN_ERROR) {
                std::cout << "Motor in error state.\n";
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::MOTOR_CALIBRATION_FAILED;
                return _motorCalibrationStatus;
            }
            if (dummy % 10 && _motorStatus == StepperMotorStatusTypes::MOTOR_IN_MOVE) {
                std::cout << "Motor in move. Current position: " << this->getMotorPosition() << "\n";
            }
        } while (_motorStatus != StepperMotorStatusTypes::MOTOR_NEGATIVE_END_SWITCHED_ON);

        std::cout << "Negative end switch reached. Setting position to 0 and sending to positive end switch.\n";
        //_motorControler->setActualPosition(MIN_MOTOR_POS_COUNTS); // this MIN_MOTOR_POS_COUNTS value seems not to be correct
        _motorControler->setActualPosition(0);
         std::cout << "Current position is: "<< _motorControler->getActualPosition() <<". Go to positive end switch.\n";
        _motorControler->setTargetPosition(MAX_MOTOR_POS_COUNTS);
        dummy = 0;
        takeFlagInAccount = false;
        do {
            determineMotorStatusAndError();

            if (_motorStatus == StepperMotorStatusTypes::MOTOR_IN_MOVE) {
                takeFlagInAccount = true;
            }

            if (takeFlagInAccount && _motorStatus == StepperMotorStatusTypes::MOTOR_NEGATIVE_END_SWITCHED_ON) {
                std::cout << "Negative end switch reached when positive expected.\n";
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::MOTOR_CALIBRATION_FAILED;
                return _motorCalibrationStatus;
            }

            
            if (_motorStatus == StepperMotorStatusTypes::MOTOR_IN_ERROR) {
                std::cout << "Motor in error state.\n";
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::MOTOR_CALIBRATION_FAILED;
                return _motorCalibrationStatus;
            }
            if (dummy % 10 && _motorStatus == StepperMotorStatusTypes::MOTOR_IN_MOVE) {
                std::cout << "Motor in move. Current position: " << this->getMotorPosition() << "\n";
            }
        } while (_motorStatus != StepperMotorStatusTypes::MOTOR_POSITIVE_END_SWITCHED_ON);
        std::cout << "Current motor position is steps: " << _motorControler->getActualPosition() << "\n";
        sleep(1);
        std::cout << "Current motor position is steps 2: " << _motorControler->getActualPosition() << "\n";
        std::cout << "Sending motor to middle of range:  " << _motorControler->getActualPosition() / 2 << "\n";
        float newPos = this->recalculateStepsToUnits(_motorControler->getActualPosition() / 2 );
        std::cout << "Sending motor to middle of range. Position after recalc:  " << newPos << "\n";
        this->moveToPosition(newPos);
        _motorCalibrationStatus = StepperMotorCalibrationStatusType::MOTOR_CALIBRATED;
        return _motorCalibrationStatus;
    }

    void StepperMotor::setMotorPosition(float newPosition) {
        float position = newPosition;
        if (newPosition > _maxPositionLimit)
            position = _maxPositionLimit;

        if (newPosition < _minPositionLimit)
            position = _minPositionLimit;

        _setPositionInUnits = position;
        _setPositionInSteps = this->recalculateUnitsToSteps(position);
        if (_autostartFlag) {
            this->startMotor();
        }

    }

    float StepperMotor::getMotorPosition() {
        _currentPostionsInSteps = _motorControler->getActualPosition();
        _currentPostionsInUnits = this->recalculateStepsToUnits(_currentPostionsInSteps);
        return _currentPostionsInUnits;
    }

    void StepperMotor::setConversionFunctions(unsigned int (*UnitToStepsPtr)(float), float (*StepsToUnitsPtr)(int)) {
        unitsToSteps = UnitToStepsPtr;
        stepsToUnits = StepsToUnitsPtr;
    }

    void StepperMotor::startMotor() {
        _motorControler->setTargetPosition(_setPositionInSteps);
    }

    void StepperMotor::stopMotor() {
        //stopping is done by reading real position and setting it as target one. In reality it can cause that motor will stop and move in reverse direction by couple steps
        //Amount of steps done in reverse direction depends on motor speed and general delay in motor control path
        unsigned int currentPosition = _motorControler->getActualPosition();
        _motorControler->setTargetPosition(currentPosition);
    }

    void StepperMotor::emergencyStopMotor() {
        //Emergency stop is done disabling motor driver. It can cause lost of calibration for the motor. After emergency stop calibration flag will be set to FALSE
        _motorControler->setEnabled(false);
        // we can execute stopMotor function to stop internal controller counter as close to the emergency stop position
        this->stopMotor();
        this->_motorCalibrationStatus = StepperMotorCalibrationStatusType::MOTOR_NOT_CALIBRATED;
    }

    // Speed setting

    void StepperMotor::setMotorSpeed(float newSpeed) {
        //FIX ME
        int ns = static_cast<unsigned int> (fabs(newSpeed));
        _motorDriverCardPtr->getMotorControler(_motorDriverId).setActualVelocity(ns);
    }

    float StepperMotor::getMotorSpeed() {
        //FIX ME
        return static_cast<float> (_motorControler->getActualVelocity());
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

    StepperMotorStatus StepperMotor::getMotorStatus() {
        this->determineMotorStatusAndError();
        return _motorStatus;
    }

    StepperMotorError StepperMotor::getMotorError() {
        this->determineMotorStatusAndError();
        return _motorError;
    }


    //autostart 

    bool StepperMotor::getAutostart() {
        return _autostartFlag;
    }

    void StepperMotor::setAutostart(bool autostart) {
        _autostartFlag = autostart;

        if (_autostartFlag)
            this->startMotor();
    }

    void StepperMotor::setEnable(bool enable) {
        _motorControler->setEnabled(enable);
    }


    //
    // PRIVATE METHODS
    //

    void StepperMotor::determineMotorStatusAndError() {

        _motorStatus = StepperMotorStatusTypes::MOTOR_OK;

        _motorControler->isEnabled();
        //first check if disabled
        if (!_motorDriverCardPtr->getMotorControler(_motorDriverId).isEnabled()) {
            _motorStatus = StepperMotorStatusTypes::MOTOR_DISABLED;
            return;
        }

        //check end switches
        bool positiveSwitchStatus = _motorControler->getReferenceSwitchData().getPositiveSwitchActive();
        bool negativeSwitchStatus = _motorControler->getReferenceSwitchData().getNegativeSwitchActive();


        //check if both end switches are on in the same time - this means error;    
        if (positiveSwitchStatus && negativeSwitchStatus) {
            _motorStatus = StepperMotorStatusTypes::MOTOR_IN_ERROR;
            _motorError = StepperMotorErrorTypes::MOTOR_BOTH_ENDSWICHTED_ON;

            return;
        }

        if (positiveSwitchStatus) {
            _motorStatus = StepperMotorStatusTypes::MOTOR_POSITIVE_END_SWITCHED_ON;
            return;
        }

        if (negativeSwitchStatus) {
            _motorStatus = StepperMotorStatusTypes::MOTOR_NEGATIVE_END_SWITCHED_ON;
            return;
        }

        if (_setPositionInUnits == _maxPositionLimit) {
            _motorStatus = StepperMotorStatusTypes::MOTOR_SOFT_PLUS_END_SWITCHED_ON;
            return;
        }

        if (_setPositionInUnits == _maxPositionLimit) {
            _motorStatus = StepperMotorStatusTypes::MOTOR_SOFT_MINUS_END_SWITCHED_ON;
            return;
        }


        if (_motorControler->getStatus().getStandstillIndicator() == 0) {
            _motorStatus = StepperMotorStatusTypes::MOTOR_IN_MOVE;
            return;
        }


        if (_setPositionInSteps != _motorControler->getActualPosition()) {
            _motorStatus = StepperMotorStatusTypes::MOTOR_IN_MOVE;
            return;
        }

    }

    int StepperMotor::recalculateUnitsToSteps(float units) {
        if (unitsToSteps == NULL)
            return static_cast<unsigned int> (fabs(units));

        return unitsToSteps(units);

    }

    float StepperMotor::recalculateStepsToUnits(int steps) {
        if (stepsToUnits == NULL)
            return static_cast<float> (steps);

        return stepsToUnits(steps);
    }


} 
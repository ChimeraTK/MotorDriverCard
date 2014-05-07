#include "StepperMotor.h"
#include "math.h"

#define DEBUG_ON
#define DEBUG_LEVEL 1


#define MAX_MOTOR_POS_COUNTS 0x7FFFFF
#define MIN_MOTOR_POS_COUNTS 0x100000

namespace mtca4u {



    StepperMotorError StepperMotorErrorTypes::MOTOR_NO_ERROR(0, "NO ERROR");
    StepperMotorError StepperMotorErrorTypes::MOTOR_BOTH_ENDSWICHTED_ON(1, "BOTH END SWITCHES ON");
    StepperMotorError StepperMotorErrorTypes::MOTOR_COMMUNICATION_LOST(2, "COMMUNICATION LOST");
    StepperMotorError StepperMotorErrorTypes::MOTOR_CONFIGURATION_ERROR(4, "MOTOR CONFIGURATION ERROR");


    StepperMotorStatus StepperMotorStatusTypes::M_OK(0, "OK");
    StepperMotorStatus StepperMotorStatusTypes::M_DISABLED(1, "DISABLED");
    StepperMotorStatus StepperMotorStatusTypes::M_IN_MOVE(2, "IN MOVE");
    StepperMotorStatus StepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON(4, "HARDWARE POSITIVE END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON(8, "HARDWARE NEGATIVE END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON(16, "SOFTWARE POSITIVE END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON(32, "SOFTWARE NEGATIVE END SWITCH ON");
    StepperMotorStatus StepperMotorStatusTypes::M_ERROR(64, "ERROR");


    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_UNKNOWN(0, "UNKNOWN");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATED(1, "CALIBRATED");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_NOT_CALIBRATED(2, "NOT CALIBRATED");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED(4, "FAILED");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS(8, "IN PROGRESS");
    StepperMotorCalibrationStatus StepperMotorCalibrationStatusType::M_CALIBRATION_STOPED_BY_USER(16, "STOPPED BY USER");

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

        _targetPositionInSteps = 0;
        _targetPositionInUnits = 0;

        _autostartFlag = false;
        
        _stopMotorForBlocking = false;

    }

    StepperMotor::~StepperMotor() {
        //delete _motorDriverCardPtr;
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
    
    StepperMotorStatus StepperMotor::moveToPosition(float newPosition) {
        
        StepperMotorStatus status = this->getMotorStatus();

        this->setMotorPosition(newPosition);

        if (!this->getAutostart()) {
            this->startMotor();
        }

        //int dummy = 0;
        bool continueWaiting = true;
       
        do {
            status = this->getMotorStatus();
            //std::cout << dummy++ << ") StepperMotor::moveToPosition : Motor in move. Current position:" << this->getMotorPosition() << "\n";
            if (_stopMotorForBlocking == true) {
                this->stopMotor();
                continueWaiting = false;
                _stopMotorForBlocking = false;
                std::cout << "StepperMotor::moveToPosition : Motor stopped by user.\n" << std::flush;
                while (this->getMotorStatus() == StepperMotorStatusTypes::M_IN_MOVE) {
                    //std::cout << "StepperMotor::moveToPosition : Wait to change status for M_IN_MOVE to other.\n" << std::flush;
                }
                
            }
            else if (status == StepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON && _targetPositionInSteps > _motorControler->getActualPosition()) {
                continueWaiting = true;
                //std::cout << "StepperMotor::moveToPosition : Negative end switch on but moving in positive direction.\n";
            } else if (status == StepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON && _targetPositionInSteps < _motorControler->getActualPosition()) {
                continueWaiting = true;
                //std::cout << "StepperMotor::moveToPosition : Positive end switch on but moving in negative direction.\n";
            } else if (status == mtca4u::StepperMotorStatusTypes::M_IN_MOVE) {
                continueWaiting = true;
            } else {
                 continueWaiting = false;
            }
        } while (continueWaiting);
        
        //std::cout << "StepperMotor::moveToPosition : Motor status after wait ending: " << this->getMotorStatus() << std::endl;

        return this->getMotorStatus();

    }

    StepperMotorCalibrationStatus StepperMotor::calibrateMotor() {
        
        //  !!! WE NEED TO BE SURE THAT PHYSICAL END_SWITCHES ARE AVAIABLE AND ARE ON !!!!! - FIX ME !!!!!!!!!!!! maybe read from config, motor config should contain this info if end switches are avaiable
        
        _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS;
        //enable the motor
        _motorControler->setEnabled(true);

        std::cout << "StepperMotor::calibrateMotor : Enable both end switches." << std::endl;
        _motorControler->getReferenceSwitchData().setPositiveSwitchEnabled(1);
        _motorControler->getReferenceSwitchData().setNegativeSwitchEnabled(1);
        
        // check status of the motor. If end switches are not avaliable we should get error condition.
        StepperMotorStatus status = this->getMotorStatus();
        if (status == StepperMotorStatusTypes::M_ERROR) {
            std::cout << "StepperMotor::calibrateMotor : Motor in error state. Error is: " << this->getMotorError() << std::endl;
        }
            
        
        //set current position to maximum
        _motorControler->setActualPosition(MAX_MOTOR_POS_COUNTS);
        //sending motor to the end switch
        _motorControler->setTargetPosition(MIN_MOTOR_POS_COUNTS);
        int dummy = 0;
        bool takeFlagInAccount = false;
        do {
            determineMotorStatusAndError();

            if (_stopMotorForBlocking == true) {
                _stopMotorForBlocking = false;
                std::cout << "StepperMotor::calibrateMotor : Motor stopped by user." << std::endl;
                return StepperMotorCalibrationStatusType::M_CALIBRATION_STOPED_BY_USER;
            }
            
            if (_motorStatus == StepperMotorStatusTypes::M_IN_MOVE) {
                takeFlagInAccount = true;
            }
            
            if (takeFlagInAccount && _motorStatus == StepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON) {
                std::cout << "StepperMotor::calibrateMotor : Positive end switch reached when negative expected.\n";
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
                return _motorCalibrationStatus;
            }
            if (_motorStatus == StepperMotorStatusTypes::M_ERROR) {
                std::cout << "StepperMotor::calibrateMotor : Motor in error state." << std::endl;
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
                return _motorCalibrationStatus;
            }
            if ( ((dummy++) % 500 == 0) && _motorStatus == StepperMotorStatusTypes::M_IN_MOVE) {
                std::cout << "StepperMotor::calibrateMotor : Motor in move. Current position: " << this->getMotorPosition() << "\n";
            }
        } while (_motorStatus != StepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON);

        std::cout << "StepperMotor::calibrateMotor : Negative end switch reached. Setting position to 0 and sending to positive end switch.\n";
        //_motorControler->setActualPosition(MIN_MOTOR_POS_COUNTS); // this MIN_MOTOR_POS_COUNTS value seems not to be correct
        _motorControler->setActualPosition(0);
        
        _calibNegativeEndSwitchAtSteps = _motorControler->getActualPosition();
        _calibNegativeEndSwitchAtUnits = recalculateStepsToUnits(_calibNegativeEndSwitchAtSteps);
        
        std::cout << "StepperMotor::calibrateMotor : Current position is: "<< _motorControler->getActualPosition() <<". Go to positive end switch.\n";
        _motorControler->setTargetPosition(MAX_MOTOR_POS_COUNTS);
        dummy = 0;
        takeFlagInAccount = false;
        do {
            determineMotorStatusAndError();
            
            if (_stopMotorForBlocking == true) {
                _stopMotorForBlocking = false;
                std::cout << "StepperMotor::calibrateMotor : StepperMotor::calibrateMotor : Motor stopped by user.\n" << std::flush;
                return StepperMotorCalibrationStatusType::M_CALIBRATION_STOPED_BY_USER;
            }
            
            if (_motorStatus == StepperMotorStatusTypes::M_IN_MOVE) {
                takeFlagInAccount = true;
            }

            if (takeFlagInAccount && _motorStatus == StepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON) {
                std::cout << "StepperMotor::calibrateMotor : Negative end switch reached when positive expected.\n";
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED; // FIX ME - we should also store information why it happened
                return _motorCalibrationStatus;
            }

            
            if (_motorStatus == StepperMotorStatusTypes::M_ERROR) {
                std::cout << "StepperMotor::calibrateMotor : Motor in error state." << std::endl;
                _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED;
                return _motorCalibrationStatus;
            }
            if ( ((dummy++) % 500 == 0) && _motorStatus == StepperMotorStatusTypes::M_IN_MOVE) {
                std::cout << "StepperMotor::calibrateMotor : Motor in move. Current position: " << this->getMotorPosition() << "\n";
            }
        } while (_motorStatus != StepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON);

        _calibPositiveEndSwitchAtSteps = _motorControler->getActualPosition();
        _calibPositiveEndSwitchAtUnits = recalculateStepsToUnits(_calibPositiveEndSwitchAtSteps);
        
        //just some test readouts
        std::cout << "StepperMotor::calibrateMotor : Current motor position is steps: " << _motorControler->getActualPosition() << "\n";
        sleep(1);
        std::cout << "StepperMotor::calibrateMotor : Current motor position is steps 2: " << _motorControler->getActualPosition() << "\n";
        //
        std::cout << "StepperMotor::calibrateMotor : Sending motor to middle of range:  " << _motorControler->getActualPosition() / 2 << "\n";
        float newPos = this->recalculateStepsToUnits(_motorControler->getActualPosition() / 2 );
        std::cout << "StepperMotor::calibrateMotor : Sending motor to middle of range. Position after recalc:  " << newPos << "\n";
        this->moveToPosition(newPos);
        _motorCalibrationStatus = StepperMotorCalibrationStatusType::M_CALIBRATED;
        return _motorCalibrationStatus;
    }
    
    StepperMotorCalibrationStatus StepperMotor::getCalibrationStatus() {
        return _motorCalibrationStatus;
    }

    void StepperMotor::setMotorPosition(float newPosition) {
        float position = newPosition;

        _targetPositionInUnits = position;
        _targetPositionInSteps = this->recalculateUnitsToSteps(position);
        if (_autostartFlag) {
            this->startMotor();
        }

    }

    float StepperMotor::getMotorPosition() {
        _currentPostionsInSteps = _motorControler->getActualPosition();
        _currentPostionsInUnits = this->recalculateStepsToUnits(_currentPostionsInSteps);
        return _currentPostionsInUnits;
    }

    void StepperMotor::setConversionFunctions(int (*UnitToStepsPtr)(float), float (*StepsToUnitsPtr)(int)) {
        unitsToSteps = UnitToStepsPtr;
        stepsToUnits = StepsToUnitsPtr;
    }

    void StepperMotor::startMotor() {
        if (this->getMotorStatus() == StepperMotorStatusTypes::M_OK)
                _motorControler->setTargetPosition(_targetPositionInSteps);
    }

    void StepperMotor::stopMotor() {
        //stopping is done by reading real position and setting it as target one. In reality it can cause that motor will stop and move in reverse direction by couple steps
        //Amount of steps done in reverse direction depends on motor speed and general delay in motor control path
        _stopMotorForBlocking = true;
        int currentPosition = _motorControler->getActualPosition();
        _targetPositionInSteps = currentPosition;
        _targetPositionInUnits = recalculateStepsToUnits(currentPosition);
        _motorControler->setTargetPosition(currentPosition);
        /*
        if (this->getMotorStatus() == StepperMotorStatusTypes::M_IN_MOVE) {
            while (this->getMotorStatus() == StepperMotorStatusTypes::M_IN_MOVE) {
                std::cout << "StepperMotor::stopMotor : Wait to change status for M_IN_MOVE to other.\n" << std::flush;
            }
        }
        */
        
    }

    void StepperMotor::stopMotorEmergency() {
        //Emergency stop is done disabling motor driver. It can cause lost of calibration for the motor. After emergency stop calibration flag will be set to FALSE
        _stopMotorForBlocking = true;
        _motorControler->setEnabled(false);
        // we can execute stopMotor function to stop internal controller counter as close to the emergency stop position. Moreover, it will also interrupt blocking function.
        this->stopMotor();
        this->_motorCalibrationStatus = StepperMotorCalibrationStatusType::M_NOT_CALIBRATED;
    }

    // Speed setting

    void StepperMotor::setMotorSpeed(float newSpeed) {
        //FIX ME
        int ns = static_cast<int> (fabs(newSpeed));
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

        // if flag set to true we need to synchronize target position with from class with one in hardware
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

        _motorStatus = StepperMotorStatusTypes::M_OK;

        _motorControler->isEnabled();
        //first check if disabled
        if (!_motorDriverCardPtr->getMotorControler(_motorDriverId).isEnabled()) {
            _motorStatus = StepperMotorStatusTypes::M_DISABLED;
            return;
        }

        //check end switches
        bool positiveSwitchStatus = _motorControler->getReferenceSwitchData().getPositiveSwitchActive();
        bool negativeSwitchStatus = _motorControler->getReferenceSwitchData().getNegativeSwitchActive();


        //check if both end switches are on in the same time - this means error;    
        if (positiveSwitchStatus && negativeSwitchStatus) {
            _motorStatus = StepperMotorStatusTypes::M_ERROR;
            _motorError = StepperMotorErrorTypes::MOTOR_BOTH_ENDSWICHTED_ON;

            return;
        }

        if (positiveSwitchStatus) {
            _motorStatus = StepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON;
            return;
        }

        if (negativeSwitchStatus) {
            _motorStatus = StepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON;
            return;
        }

        if (_targetPositionInUnits >= _maxPositionLimit) {
            _motorStatus = StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON;
            return;
        }

        if (_targetPositionInUnits <= _minPositionLimit) {
            _motorStatus = StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON;
            return;
        }


        if (_motorControler->getStatus().getStandstillIndicator() == 0) {
            _motorStatus = StepperMotorStatusTypes::M_IN_MOVE;
            return;
        }


        if (_targetPositionInSteps != _motorControler->getActualPosition()) {
            _motorStatus = StepperMotorStatusTypes::M_IN_MOVE;
            return;
        }

    }

    int StepperMotor::recalculateUnitsToSteps(float units) {
        if (unitsToSteps == NULL)
            return static_cast<int> (units);

        return unitsToSteps(units);

    }

    float StepperMotor::recalculateStepsToUnits(int steps) {
        if (stepsToUnits == NULL)
            return static_cast<float> (steps);

        return stepsToUnits(steps);
    }


} 
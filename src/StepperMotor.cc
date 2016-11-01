#include "StepperMotor.h"
#include <cmath>
#include <chrono>
#include "MotorDriverCardFactory.h"
#include "MotorDriverException.h"
#include "StepperMotorException.h"

namespace mtca4u {


  StepperMotor::StepperMotor(std::string const & motorDriverCardDeviceName, std::string const & moduleName,
			     unsigned int motorDriverId, std::string motorDriverCardConfigFileName) 
    : _motorDriverCardDeviceName(motorDriverCardDeviceName),
      _motorDriverId(motorDriverId),
      _motorDriverCard( MotorDriverCardFactory::instance().createMotorDriverCard(
			    motorDriverCardDeviceName, moduleName, motorDriverCardConfigFileName)),
      _motorControler(_motorDriverCard->getMotorControler(_motorDriverId)),
      //_currentPostionsInSteps(0),// position - don't know so set to 0
      //_currentPostionsInUnits(0),
      _stepperMotorUnitsConverter(new StepperMotorUnitsConverterTrivia()),
      _targetPositionInSteps(0),
      //_targetPositionInUnits(0),
      //_maxPositionLimit(std::numeric_limits<float>::max()),
      _maxPositionLimitInSteps(std::numeric_limits<int>::max()),
      //_minPositionLimit(-std::numeric_limits<float>::max()),
      _minPositionLimitInSteps(-std::numeric_limits<int>::max()),
      _autostartFlag(false),
      //_stopMotorForBlocking(false),
      _softwareLimitsEnabled(true),
      _motorError(),
      _motorCalibrationStatus(StepperMotorCalibrationStatusType::M_NOT_CALIBRATED),
      _motorStatus(),
      _currentOperationalState(SYSTEM_IDLE),
      //_blockingFunctionActive(false),
      _logger(),
      _mutex(),
      _operationalStateMutex(),
      _isActionComplete(false),
      _startUserAction("START_USER_ACTION"),
      _startUserActionBlocking("START_USER_ACTION_BLOCKING"),
      _startUserActionAutoStart("START_USER_ACTION_AUTOSTART"),
      _startCalibration("START_CALIBRATION"),
      _stopUserAction("STOP_USER_ACTION"),
      _noEvent("NO_EVENT"),
      _currentEvent(_noEvent),
      _runStateMachine(true),
      _transitionTableThread()
      {
    _transitionTableThread = std::thread(&StepperMotor::transitionTable, this);
    }

    StepperMotor::~StepperMotor() {
    }


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // BLOCKING FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    StepperMotorStatusAndError StepperMotor::moveToPositionInSteps(int newPositionInSteps) {

      StepperMotorStatusAndError statusAndError = this->determineMotorStatusAndError();
      if (statusAndError.status == StepperMotorStatusTypes::M_ERROR || statusAndError.status == StepperMotorStatusTypes::M_DISABLED) {
        return statusAndError;
      }

      if (setUserActionBlockingEvent(newPositionInSteps)){
        while (!isSystemIdle()){}
      }

      return(this->determineMotorStatusAndError());
    }

    StepperMotorStatusAndError StepperMotor::moveToPosition(float newPosition){
      int positionInSteps = this->recalculateUnitsToSteps(newPosition);
      return (this->moveToPositionInSteps(positionInSteps));
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
      std::lock_guard<std::mutex> lock(_operationalStateMutex);
      if (_currentOperationalState == SYSTEM_IDLE && _currentEvent == _noEvent){
        _currentEvent = _startUserAction;
      }
    }

    void StepperMotor::startCalibration() {
      std::lock_guard<std::mutex> lock(_operationalStateMutex);
      if (_currentOperationalState == SYSTEM_IDLE && _currentEvent == _noEvent){
        _currentEvent = _startCalibration;
      }
    }

    void StepperMotor::stop() {
      std::lock_guard<std::mutex> lock(_operationalStateMutex);
        //stopping is done by reading real position and setting it as target
        //one. In reality it can cause that motor will stop and move in reverse
        //direction by couple steps Amount of steps done in reverse direction
        //depends on motor speed and general delay in motor control path.
      _currentEvent = _stopUserAction;

    }

    void StepperMotor::emergencyStop() {
        //Emergency stop is done disabling motor driver. It can cause lost of calibration for the motor. After emergency stop calibration flag will be set to FALSE
        _motorControler->setEnabled(false);
        // we can execute stop function to stop internal controller counter as close to the emergency stop position. Moreover, it will also interrupt blocking function.
        this->stop();
    }

    int StepperMotor::recalculateUnitsToSteps(float units) {
      return _stepperMotorUnitsConverter.get()->unitsToSteps(units);
    }

    float StepperMotor::recalculateStepsToUnits(int steps) {
        return _stepperMotorUnitsConverter.get()->stepsToUnits(steps);
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // END OF GENERAL FUNCTIONS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // GETTERS AND SETTERS
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  

    bool StepperMotor::motorReady(){
      StepperMotorStatusAndError statusAndError = this->determineMotorStatusAndError();
      if (statusAndError.status == StepperMotorStatusTypes::M_ERROR || statusAndError.status == StepperMotorStatusTypes::M_DISABLED) {
        return false;
      }else{
        return true;
      }
    }

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
      setCurrentPositionInStepsAs(recalculateUnitsToSteps(newPosition));
    }

    void StepperMotor::setCurrentPositionInStepsAs(int newPositionSteps){
      std::lock_guard<std::mutex> lock(_operationalStateMutex);
      if (_currentOperationalState == SYSTEM_IDLE && _currentEvent == _noEvent){
        _targetPositionInSteps = newPositionSteps;
        int delta = getCurrentPositionInSteps() - newPositionSteps;
        bool enable = _motorControler->isEnabled();
        _motorControler->setEnabled(false);
        _motorControler->setActualPosition(newPositionSteps);
        _motorControler->setTargetPosition(newPositionSteps);
        _motorControler->setEnabled(enable);
        _maxPositionLimitInSteps -= delta;
        _minPositionLimitInSteps -= delta;
      }
    }

    void StepperMotor::setTargetPosition(float newPosition) {
      setTargetPositionInSteps(this->recalculateUnitsToSteps(newPosition));
    }

    void StepperMotor::setTargetPositionInSteps(int newPositionInSteps){
      std::lock_guard<std::mutex> lock(_operationalStateMutex);
      if (_currentOperationalState == SYSTEM_IDLE && _currentEvent == _noEvent){
        _targetPositionInSteps = newPositionInSteps;
        if (_autostartFlag) {
          _currentEvent = _startUserActionAutoStart;
        }
      }
    }


    float StepperMotor::getTargetPosition(){
        return this->recalculateStepsToUnits(_targetPositionInSteps);
    }

    int StepperMotor::getTargetPositionInSteps(){
      return _targetPositionInSteps;
    }

    float StepperMotor::getCurrentPosition() {
       return this->recalculateStepsToUnits(getCurrentPositionInSteps());
    }

    int StepperMotor::getCurrentPositionInSteps(){
      return (_motorControler->getActualPosition());
    }

    void StepperMotor::setStepperMotorUnitsConverter(boost::shared_ptr<StepperMotorUnitsConverter> stepperMotorUnitsConverter) {
      if (!stepperMotorUnitsConverter){
        throw StepperMotorException(std::string("empty shared pointer for the unit converter"), StepperMotorException::NOT_VALID_CONVERTER);
      }
        _stepperMotorUnitsConverter = stepperMotorUnitsConverter;
    }

    void StepperMotor::setStepperMotorUnitsConverterToDefault(){
      _stepperMotorUnitsConverter.reset(new StepperMotorUnitsConverterTrivia);
    }

    double StepperMotor::setUserSpeedLimit(double newSpeed) {
      return _motorControler->setUserSpeedLimit(newSpeed);
    }

    void StepperMotor::setMaxPositionLimit(float maxPos) {
        setMaxPositionLimitInSteps(recalculateUnitsToSteps(maxPos));
    }

    void StepperMotor::setMaxPositionLimitInSteps(int maxPosInSteps){
      _maxPositionLimitInSteps = maxPosInSteps;
    }

    void StepperMotor::setMinPositionLimit(float minPos) {
       setMinPositionLimitInSteps(recalculateUnitsToSteps(minPos));
    }

    void StepperMotor::setMinPositionLimitInSteps(int minPosInStep){
      _minPositionLimitInSteps = minPosInStep;
    }

    float StepperMotor::getMaxPositionLimit() {
        return recalculateStepsToUnits(_maxPositionLimitInSteps);
    }

    int StepperMotor::getMaxPositionLimitInSteps(){
      return _maxPositionLimitInSteps;
    }

    float StepperMotor::getMinPositionLimit() {
        return recalculateStepsToUnits(_minPositionLimitInSteps);
    }

    int StepperMotor::getMinPositionLimitInSteps(){
      return _minPositionLimitInSteps;
    }

    StepperMotorStatusAndError StepperMotor::getStatusAndError() {
        return determineMotorStatusAndError();
    }
    
    bool StepperMotor::isSystemIdle(){
      std::lock_guard<std::mutex> lock(_operationalStateMutex);
      if (_currentOperationalState == SYSTEM_IDLE && _currentEvent == _noEvent){
        return true;
      }else{
        return false;
      }
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
      boost::lock_guard<boost::mutex> guard(_mutex);
      _motorControler->setMotorCurrentEnabled(enable);
      _motorControler->setEndSwitchPowerEnabled(enable);
    }

    bool StepperMotor::getEnabled() const {
      boost::lock_guard<boost::mutex> guard(_mutex);
      // card is disabled and safe to plug out only when both motor current
      // and endswitch power is killed
      // for the old firmware version isEndSwitchPowerEnabled always returns
      // false (End switch power is not applicable in this scenario).
      return (_motorControler->isEndSwitchPowerEnabled() ||
              _motorControler->isMotorCurrentEnabled());
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

    int StepperMotor::truncateMotorPosition(int newPosition) {
        int position = newPosition;
        
        if (!_softwareLimitsEnabled)
            return position;

        // truncate new position according to the internal limits.        
        if (position > _maxPositionLimitInSteps) {
            position = _maxPositionLimitInSteps;
        } else if (position < _minPositionLimitInSteps) {
            position = _minPositionLimitInSteps;
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


      if (_softwareLimitsEnabled && (_maxPositionLimitInSteps <= _minPositionLimitInSteps)) {
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

      if (_motorControler->getStatus().getStandstillIndicator() == 0) {
        _motorStatus = StepperMotorStatusTypes::M_IN_MOVE;
        return StepperMotorStatusAndError(_motorStatus, _motorError);
      }


      if (_targetPositionInSteps != _motorControler->getActualPosition()) {
        _motorStatus = StepperMotorStatusTypes::M_NOT_IN_POSITION;
        return StepperMotorStatusAndError(_motorStatus, _motorError);
      }

      if (_softwareLimitsEnabled) {
        if (_targetPositionInSteps >= _maxPositionLimitInSteps) {
          _motorStatus = StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON;
          return StepperMotorStatusAndError(_motorStatus, _motorError);
        }

        if (_targetPositionInSteps <= _minPositionLimitInSteps) {
          _motorStatus = StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON;
          return StepperMotorStatusAndError(_motorStatus, _motorError);
        }
      }

      return StepperMotorStatusAndError(_motorStatus, _motorError);
    }

    void StepperMotor::checkTargetPositionAndStartMotor(){
      _targetPositionInSteps = truncateMotorPosition(_targetPositionInSteps);
      _motorControler->setTargetPosition(_targetPositionInSteps);
    }

    bool StepperMotor::setUserActionBlockingEvent(int newPositionInSteps){
      std::lock_guard<std::mutex> lock(_operationalStateMutex);
      if (_currentOperationalState == SYSTEM_IDLE && _currentEvent == _noEvent){
        _targetPositionInSteps = newPositionInSteps;
        _currentEvent = _startUserActionBlocking;
        return true;
      }
      return false;
    }

    void StepperMotor::transitionTable(){
      while (_runStateMachine){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        switch(_currentOperationalState){
          ///////*****SYSTEM_IDLE transitions
          case SYSTEM_IDLE:{
            if (_currentEvent == _noEvent){
              break;
            }else if (_currentEvent == _stopUserAction){
              std::lock_guard<std::mutex> lock(_operationalStateMutex);
              _currentEvent = _noEvent;
              break;
            }else if (_currentEvent == _startUserAction){
              std::lock_guard<std::mutex> lock(_operationalStateMutex);
              _currentOperationalState = USER_ACTION;
              startUserAction();
              break;
            }else if (_currentEvent == _startUserActionBlocking){
              std::lock_guard<std::mutex> lock(_operationalStateMutex);
              _currentOperationalState = USER_ACTION_BLOCKING;
              startUserAction();
            }else if (_currentEvent == _startUserActionAutoStart){
              std::lock_guard<std::mutex> lock(_operationalStateMutex);
              _currentOperationalState = USER_ACTION_AUTOSTART;
              startUserAction();
            }else if (_currentEvent == _startCalibration){
              std::lock_guard<std::mutex> lock(_operationalStateMutex);
              _currentOperationalState = CALIBRATION;
              _isActionComplete = false;
              std::async(std::launch::async, &StepperMotor::startCalibrationThread, this);
              break;
            }else{
              break;
            }
          }
          ///////*****USER_ACTION transitions
          case USER_ACTION:{
            if (_currentEvent == _stopUserAction){
              std::lock_guard<std::mutex> lock(_operationalStateMutex);
              _currentEvent = _noEvent;
              int currentPosition = _motorControler->getActualPosition();
              _motorControler->setTargetPosition(currentPosition);
              _targetPositionInSteps = currentPosition;
              break;
            }else if (!isMoving()){
              std::lock_guard<std::mutex> lock(_operationalStateMutex);
              _currentEvent = _noEvent;
              _currentOperationalState = SYSTEM_IDLE;
              break;
            }
            break;
          }
          ///////*****USER_ACTION_BLOCKING transitions
          case USER_ACTION_BLOCKING:{
            if (_currentEvent == _stopUserAction){
              std::lock_guard<std::mutex> lock(_operationalStateMutex);
              _currentEvent = _noEvent;
              int currentPosition = _motorControler->getActualPosition();
              _motorControler->setTargetPosition(currentPosition);
              _targetPositionInSteps = currentPosition;
              break;
            }else if (!isMoving()){
              std::lock_guard<std::mutex> lock(_operationalStateMutex);
              _currentEvent = _noEvent;
              _currentOperationalState = SYSTEM_IDLE;
              break;
            }
            break;
          }
          ///////*****USER_ACTION_BLOCKING transitions
          case USER_ACTION_AUTOSTART:{
            if (_currentEvent == _stopUserAction){
              std::lock_guard<std::mutex> lock(_operationalStateMutex);
              _currentEvent = _noEvent;
              int currentPosition = _motorControler->getActualPosition();
              _motorControler->setTargetPosition(currentPosition);
              _targetPositionInSteps = currentPosition;
              break;
            }else if (!isMoving()){
              std::lock_guard<std::mutex> lock(_operationalStateMutex);
              _currentEvent = _noEvent;
              _currentOperationalState = SYSTEM_IDLE;
              break;
            }
            break;
          }
          ///////*****CALIBRATION transitions:
          case CALIBRATION:{
            if (_isActionComplete){
              std::lock_guard<std::mutex> lock(_operationalStateMutex);
              _currentEvent = _noEvent;
              _currentOperationalState = SYSTEM_IDLE;
              break;
            }else{
              break;
            }
            break;
          }
        }//Switch
      }//while
    }

    void StepperMotor::startUserAction(){
      if (motorReady()){
        this->checkTargetPositionAndStartMotor();
      }
    }

    void StepperMotor::startCalibrationThread(){
      calibrateMotor();
      _isActionComplete = true;
    }
}

bool mtca4u::StepperMotor::isMoving() {
  return (_motorControler->isMotorMoving());
}

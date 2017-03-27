#include "StepperMotor.h"
#include <cmath>
#include <future>
#include "MotorDriverCardFactory.h"
#include "MotorDriverException.h"
#include "StepperMotorException.h"
#include "StepperMotorStateMachine.h"

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
	_stopMotorForBlocking(false),
	_softwareLimitsEnabled(true),
	_motorError(),
	_motorCalibrationStatus(StepperMotorCalibrationStatusType::M_NOT_CALIBRATED),
	_motorStatus(),
	_blockingFunctionActive(false),
	_logger(),
	_mutex(){
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

    _blockingFunctionActive = true;
    // Ask the hardware to move the motor.
    this->setTargetPositionInSteps(newPositionInSteps);
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
    // allowing it when status is MOTOR_IN_MOVE give a possibility to change
    // target position during motor movement
    _targetPositionInSteps = truncateMotorPosition(_targetPositionInSteps);
    _motorControler->setTargetPosition(_targetPositionInSteps);
    _targetPositionInSteps =  _motorControler->getTargetPosition();
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
    //_targetPositionInUnits = recalculateStepsToUnits(currentPosition);

  }

  void StepperMotor::emergencyStop() {
    //Emergency stop is done disabling motor driver. It can cause lost of calibration for the motor. After emergency stop calibration flag will be set to FALSE
    _motorControler->setEnabled(false);
    // we can execute stop function to stop internal controller counter as close to the emergency stop position. Moreover, it will also interrupt blocking function.
    this->stop();
  }

  int StepperMotor::recalculateUnitsToSteps(float units) {
    //        if (!_stepperMotorUnitsConverter)
    //            return static_cast<int> (units);

    return _stepperMotorUnitsConverter.get()->unitsToSteps(units);
  }

  float StepperMotor::recalculateStepsToUnits(int steps) {

    //        if (!_stepperMotorUnitsConverter)
    //            return static_cast<float> (steps);

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
    setCurrentPositionInStepsAs(recalculateUnitsToSteps(newPosition));
  }

  void StepperMotor::setCurrentPositionInStepsAs(int newPositionSteps){
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

  void StepperMotor::setTargetPosition(float newPosition) {
    setTargetPositionInSteps(this->recalculateUnitsToSteps(newPosition));
  }

  void StepperMotor::setTargetPositionInSteps(int newPositionInSteps){
    _targetPositionInSteps = newPositionInSteps;
    this->determineMotorStatusAndError();
    if (_motorError == StepperMotorErrorTypes::M_NO_ERROR) {
      if (_autostartFlag) {
	this->start();
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

//  unsigned int StepperMotor::getMicroStepCount(){
//    return _motorControler->getMicroStepCount();
//  }

  void StepperMotor::enableFullStepping(bool enable){
    _motorControler->enableFullStepping(enable);
  }

  bool StepperMotor::isFullStepping(){
    return _motorControler->isFullStepping();
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
}

bool mtca4u::StepperMotor::isMoving() {
  return (_motorControler->isMotorMoving());
}

namespace ChimeraTK{

  StepperMotor::StepperMotor(std::string const & motorDriverCardDeviceName,
                             std::string const & moduleName,
                             unsigned int motorDriverId,
                             std::string motorDriverCardConfigFileName):
				       _motorDriverCardDeviceName(motorDriverCardDeviceName),
				       _motorDriverId(motorDriverId),
				       _motorDriverCard( mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(
					   motorDriverCardDeviceName, moduleName, motorDriverCardConfigFileName)),
					   _motorControler(_motorDriverCard->getMotorControler(_motorDriverId)),
					   _stepperMotorUnitsConverter(new mtca4u::StepperMotorUnitsConverterTrivia()),
					   _targetPositionInSteps(0),
					   _maxPositionLimitInSteps(std::numeric_limits<int>::max()),
					   _minPositionLimitInSteps(-std::numeric_limits<int>::max()),
					   _softwareLimitsEnabled(false),
					   _runStateMachine(true),
					   _logger(),
					   _mutex(),
					   _stateMachineThread(),
					   _stateMachine(),
					   _calibrated(false){
    createStateMachine();
  }

  StepperMotor::StepperMotor() :
	 _motorDriverCardDeviceName(""),
	 _motorDriverId(0),
	 _motorDriverCard(),
	 _motorControler(),
	 _stepperMotorUnitsConverter(new mtca4u::StepperMotorUnitsConverterTrivia()),
	 _targetPositionInSteps(0),
	 _maxPositionLimitInSteps(std::numeric_limits<int>::max()),
	 _minPositionLimitInSteps(-std::numeric_limits<int>::max()),
	 _softwareLimitsEnabled(false),
	 _runStateMachine(true),
	 _logger(),
	 _mutex(),
	 _stateMachineThread(),
	 _stateMachine(),
	 _calibrated(false){}

  StepperMotor::~StepperMotor() {
    _runStateMachine = false;
    if (_stateMachineThread.joinable()){
      _stateMachineThread.join();
    }
  }

  void StepperMotor::moveToPositionInSteps(int newPositionInSteps){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    if (!_calibrated){
      throw MotorDriverException("motor not calibrated", MotorDriverException::NOT_IMPLEMENTED);
    }
    if (!limitsOK(newPositionInSteps)){
      throw MotorDriverException("new position out of range of the soft limits", MotorDriverException::OUT_OF_RANGE);
    }
    _targetPositionInSteps = newPositionInSteps;
    _stateMachine->setUserEvent(StepperMotorStateMachine::moveEvent);
  }

  bool StepperMotor::limitsOK(int newPositionInSteps){
    if (_softwareLimitsEnabled && newPositionInSteps >= _minPositionLimitInSteps && newPositionInSteps <= _maxPositionLimitInSteps){
      return true;
    }else if (!_softwareLimitsEnabled){
      return true;
    }else{
      return false;
    }
  }

  void StepperMotor::moveToPosition(float newPosition){
    moveToPositionInSteps(recalculateUnitsInSteps(newPosition));
  }

  void StepperMotor::moveRelativeInSteps(int delta){
    int newPosition = getCurrentPositionInSteps() + delta;
    moveToPositionInSteps(newPosition);
  }

  void StepperMotor::moveRelative(float delta){
    int newPosition = recalculateUnitsInSteps(getCurrentPosition() + delta);
    moveToPositionInSteps(newPosition);
  }

  void StepperMotor::stop(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    _stateMachine->setUserEvent(StepperMotorStateMachine::stopEvent);
  }

  void StepperMotor::emergencyStop(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    _stateMachine->setUserEvent(StepperMotorStateMachine::emergencyStopEvent);
  }

  int StepperMotor::recalculateUnitsInSteps(float units){
    return _stepperMotorUnitsConverter->unitsToSteps(units);
  }

  float StepperMotor::recalculateStepsInUnits(int steps){
    return _stepperMotorUnitsConverter->stepsToUnits(steps);
  }

  void StepperMotor::setSoftwareLimitsEnabled(bool enabled){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    _softwareLimitsEnabled = enabled;
  }

  bool StepperMotor::getSoftwareLimitsEnabled(){
    return _softwareLimitsEnabled;
  }

  void StepperMotor::setSoftwareLimitsInSteps(int minPos, int maxPos){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }else if(minPos >= maxPos){
      throw MotorDriverException("minimum limit not smaller than maximum limit", MotorDriverException::NOT_IMPLEMENTED);
    }
    _minPositionLimitInSteps = minPos;
    _maxPositionLimitInSteps = maxPos;
  }

  void StepperMotor::setSoftwareLimits(float minPos, float maxPos){
    setSoftwareLimitsInSteps(recalculateUnitsInSteps(minPos), recalculateUnitsInSteps(maxPos));
  }

  int StepperMotor::getMaxPositionLimitInSteps(){
    return _maxPositionLimitInSteps;
  }

  float StepperMotor::getMaxPositionLimit(){
    return recalculateStepsInUnits(_maxPositionLimitInSteps);
  }

  int StepperMotor::getMinPositionLimitInSteps(){
    return _minPositionLimitInSteps;
  }

  float StepperMotor::getMinPositionLimit(){
    return recalculateStepsInUnits(_minPositionLimitInSteps);
  }

  void StepperMotor::resetPositionMotorController(int actualPositionInSteps){
    bool enable = _motorControler->isEnabled();
    _motorControler->setEnabled(false);
    _motorControler->setActualPosition(actualPositionInSteps);
    _motorControler->setTargetPosition(actualPositionInSteps);
    _motorControler->setEnabled(enable);
  }

  void StepperMotor::setActualPositionInSteps(int actualPositionInSteps){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    resetPositionMotorController(actualPositionInSteps);
    _calibrated = true;
  }

  void StepperMotor::setActualPosition(float actualPosition){
    setActualPositionInSteps(recalculateUnitsInSteps(actualPosition));
  }

  int StepperMotor::getCurrentPositionInSteps(){
    return (_motorControler->getActualPosition());
  }

  float StepperMotor::getCurrentPosition(){
    return recalculateStepsInUnits(_motorControler->getActualPosition());
  }

  void StepperMotor::setStepperMotorUnitsConverter(boost::shared_ptr<mtca4u::StepperMotorUnitsConverter> stepperMotorUnitsConverter){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    if (stepperMotorUnitsConverter == nullptr){
      throw MotorDriverException("unit converter shared pointer not valid", MotorDriverException::NOT_IMPLEMENTED);
    }
    _stepperMotorUnitsConverter = stepperMotorUnitsConverter;
  }

  void StepperMotor::setStepperMotorUnitsConverterToDefault(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    _stepperMotorUnitsConverter.reset(new StepperMotorUnitsConverterTrivia());
  }

  bool StepperMotor::isSystemIdle(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return stateMachineInIdleAndNoEvent();
  }

  void StepperMotor::waitForIdle(){
    while(1){
      usleep(100);
      //std::cout << "sto controllando" << std::endl;
      if (isSystemIdle()){
	break;
      }
    }
  }

  StepperMotorError StepperMotor::getError(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    StepperMotorError _motorError = NO_ERROR;
    if (stateMachineInIdleAndNoEvent() && (_motorControler->getTargetPosition() != _motorControler->getActualPosition())){
      _motorError = ACTION_ERROR;
    }
    return _motorError;
  }

  bool StepperMotor::isCalibrated(){
    return _calibrated;
  }

  void StepperMotor::setEnabled(bool enable){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (enable){
      _motorControler->setMotorCurrentEnabled(enable);
    }else{
      _stateMachine->setUserEvent(StepperMotorStateMachine::disableEvent);
    }
  }

  bool StepperMotor::getEnabled(){
    return _motorControler->isMotorCurrentEnabled();
  }

  void StepperMotor::setLogLevel(ChimeraTK::Logger::LogLevel newLevel){
    _logger.setLogLevel(newLevel);
  }

  Logger::LogLevel StepperMotor::getLogLevel() {
    return _logger.getLogLevel();
  }

  double StepperMotor::getMaxSpeedCapability() {
    return _motorControler->getMaxSpeedCapability();
  }

  double StepperMotor::getSafeCurrentLimit() {
    return (_motorControler->getMaxCurrentLimit());
  }

  double StepperMotor::setUserCurrentLimit(double currentInAmps) {
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    return (_motorControler->setUserCurrentLimit(currentInAmps));
  }

  double StepperMotor::getUserCurrentLimit() {
    return (_motorControler->getUserCurrentLimit());
  }

  double StepperMotor::setUserSpeedLimit(double newSpeed) {
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    return _motorControler->setUserSpeedLimit(newSpeed);
  }

  double StepperMotor::getUserSpeedLimit() {
    return (_motorControler->getUserSpeedLimit());
  }

  void StepperMotor::enableFullStepping(bool enable){
    _motorControler->enableFullStepping(enable);
  }

  bool StepperMotor::isFullStepping(){
    return _motorControler->isFullStepping();
  }

  bool StepperMotor::stateMachineInIdleAndNoEvent(){
    if (_stateMachine->getUserEvent() == StateMachine::noEvent && _stateMachine->getCurrentState()->getName() == "idleState"){
      //std::cout << "InIdle " << std::string(StateMachine::noEvent) << " " << _stateMachine->getCurrentState()->getName() << std::endl;
      return true;
    }else{
      //std::cout << "NOT in IDLE " << std::string(_stateMachine->getUserEvent()) << " " << _stateMachine->getCurrentState()->getName() << std::endl;
      return false;
    }
  }

  void StepperMotor::stateMachineThreadFunction(){
    while(_runStateMachine){
      usleep(100);
      stateMachinePerformTransition();
    }
  }

  void StepperMotor::stateMachinePerformTransition(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    _stateMachine->processEvent();
  }

  void StepperMotor::createStateMachine(){
    _stateMachine.reset(new StepperMotorStateMachine(*this));
    _stateMachineThread = std::thread(&StepperMotor::stateMachineThreadFunction, this);
    waitForIdle();
  }
}


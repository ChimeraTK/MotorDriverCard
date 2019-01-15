#include "BasicStepperMotor.h"
#include <cmath>
#include <future>
#include <chrono>
#include "MotorDriverCardFactory.h"
#include "MotorDriverException.h"
#include "StepperMotorException.h"
#include "StepperMotorStateMachine.h"


namespace ChimeraTK{

  BasicStepperMotor::BasicStepperMotor(StepperMotorParameters & parameters)
    :
      _motorDriverCard( mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(
                            parameters.deviceName, parameters.moduleName,
                            parameters.configFileName)),
      _motorControler(_motorDriverCard->getMotorControler(parameters.driverId)),
      _stepperMotorUnitsConverter(parameters.motorUnitsConverter),
      _encoderUnitsConverter(parameters.encoderUnitsConverter),
      _encoderPositionOffset(0),
      _targetPositionInSteps(_motorControler->getTargetPosition()),
      _maxPositionLimitInSteps(std::numeric_limits<int>::max()),
      _minPositionLimitInSteps(std::numeric_limits<int>::min()),
      _autostart(false),
      _softwareLimitsEnabled(false),
      _logger(),
      _mutex(),
      _stateMachine(),
      _calibrationMode(StepperMotorCalibrationMode::NONE)
  {
    _stateMachine.reset(new StepperMotorStateMachine(*this));
    initStateMachine();
  }

  BasicStepperMotor::BasicStepperMotor() :
     _motorDriverCard(),
     _motorControler(),
     _stepperMotorUnitsConverter(std::make_shared<StepperMotorUnitsConverterTrivia>()),
     _encoderUnitsConverter(std::make_shared<StepperMotorUtility::EncoderUnitsConverterTrivia>()),
     _encoderPositionOffset(0),
     _targetPositionInSteps(0),
     _maxPositionLimitInSteps(std::numeric_limits<int>::max()),
     _minPositionLimitInSteps(std::numeric_limits<int>::min()),
     _autostart(false),
     _softwareLimitsEnabled(false),
     _logger(),
     _mutex(),
     _stateMachine(),
     _calibrationMode(StepperMotorCalibrationMode::NONE){}

  BasicStepperMotor::~BasicStepperMotor(){}

  void BasicStepperMotor::checkNewPosition(int newPositionInSteps){
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    if (_motorControler->getCalibrationTime() == 0){
      throw MotorDriverException("motor not calibrated", MotorDriverException::NOT_IMPLEMENTED);
    }
    if (!limitsOK(newPositionInSteps)){
      throw MotorDriverException("new position out of range of the soft limits", MotorDriverException::OUT_OF_RANGE);
    }
  }

  bool BasicStepperMotor::limitsOK(int newPositionInSteps){
    if (_softwareLimitsEnabled && newPositionInSteps >= _minPositionLimitInSteps && newPositionInSteps <= _maxPositionLimitInSteps){
      return true;
    }else if (!_softwareLimitsEnabled){
      return true;
    }else{
      return false;
    }
  }

  void BasicStepperMotor::moveRelativeInSteps(int delta){
    boost::lock_guard<boost::mutex> guard(_mutex);

    int newPosition = _motorControler->getActualPosition() + delta;
    checkNewPosition(newPosition);
    _targetPositionInSteps =  newPosition;

    _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::moveEvent);
  }

  void BasicStepperMotor::moveRelative(float delta){
    boost::lock_guard<boost::mutex> guard(_mutex);

    int newPosition = _motorControler->getActualPosition() + _stepperMotorUnitsConverter->unitsToSteps(delta);
    checkNewPosition(newPosition);
    _targetPositionInSteps =  newPosition;

    _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::moveEvent);
  }

  void BasicStepperMotor::setTargetPosition(float newPosition){
    boost::lock_guard<boost::mutex> guard(_mutex);

    int newPositionInSteps = _stepperMotorUnitsConverter->unitsToSteps(newPosition);
    checkNewPosition( newPositionInSteps);
    _targetPositionInSteps =  newPositionInSteps;

    if(_autostart){
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::moveEvent);
    }
  }

  void BasicStepperMotor::setTargetPositionInSteps(int newPositionInSteps){
    boost::lock_guard<boost::mutex> guard(_mutex);

    checkNewPosition(newPositionInSteps);
    _targetPositionInSteps = newPositionInSteps;

    if(_autostart){
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::moveEvent);
    }
  }

  void BasicStepperMotor::start(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::moveEvent);
  }

  void BasicStepperMotor::stop(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::stopEvent);
  }

  void BasicStepperMotor::emergencyStop(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::emergencyStopEvent);
  }

  void BasicStepperMotor::resetError(){
    boost::lock_guard<boost::mutex> guard(_mutex);

    if(!_motorControler->isMotorCurrentEnabled()){
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::resetToDisableEvent);
    }
    else{
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::resetToIdleEvent);
    }
  }

  int BasicStepperMotor::recalculateUnitsInSteps(float units){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _stepperMotorUnitsConverter->unitsToSteps(units);
  }

  float BasicStepperMotor::recalculateStepsInUnits(int steps){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(steps);
  }

  void BasicStepperMotor::setSoftwareLimitsEnabled(bool enabled){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    _softwareLimitsEnabled = enabled;
  }

  bool BasicStepperMotor::getSoftwareLimitsEnabled(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _softwareLimitsEnabled;
  }

  void BasicStepperMotor::setMaxPositionLimitInSteps(int maxPos){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }else if(maxPos <= _minPositionLimitInSteps){
      throw MotorDriverException("minimum limit not smaller than maximum limit", MotorDriverException::NOT_IMPLEMENTED);
    }
    _maxPositionLimitInSteps = maxPos;
  }

  void BasicStepperMotor::setMaxPositionLimit(float maxPos){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }else if(_stepperMotorUnitsConverter->unitsToSteps(maxPos) <= _minPositionLimitInSteps){
      throw MotorDriverException("minimum limit not smaller than maximum limit", MotorDriverException::NOT_IMPLEMENTED);
    }
    _maxPositionLimitInSteps = _stepperMotorUnitsConverter->unitsToSteps(maxPos);
  }

  void BasicStepperMotor::setMinPositionLimitInSteps(int minPos){
      boost::lock_guard<boost::mutex> guard(_mutex);
      if (!stateMachineInIdleAndNoEvent()){
        throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
      }else if(minPos >= _maxPositionLimitInSteps){
        throw MotorDriverException("minimum limit not smaller than maximum limit", MotorDriverException::NOT_IMPLEMENTED);
      }
      _minPositionLimitInSteps = minPos;
    }

  void BasicStepperMotor::setMinPositionLimit(float minPos){
      boost::lock_guard<boost::mutex> guard(_mutex);
      if (!stateMachineInIdleAndNoEvent()){
        throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
      }else if(_stepperMotorUnitsConverter->unitsToSteps(minPos) >= _maxPositionLimitInSteps){
        throw MotorDriverException("minimum limit not smaller than maximum limit", MotorDriverException::NOT_IMPLEMENTED);
      }
      _minPositionLimitInSteps = _stepperMotorUnitsConverter->unitsToSteps(minPos);
    }

  int BasicStepperMotor::getMaxPositionLimitInSteps(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _maxPositionLimitInSteps;
  }

  float BasicStepperMotor::getMaxPositionLimit(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_maxPositionLimitInSteps);
  }

  int BasicStepperMotor::getMinPositionLimitInSteps(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _minPositionLimitInSteps;
  }

  float BasicStepperMotor::getMinPositionLimit(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_minPositionLimitInSteps);
  }

  void BasicStepperMotor::resetMotorControllerPositions(int actualPositionInSteps){
    bool enable = _motorControler->isEnabled();
    _motorControler->setEnabled(false);
    _motorControler->setActualPosition(actualPositionInSteps);
    _motorControler->setTargetPosition(actualPositionInSteps);
    _motorControler->setEnabled(enable);
  }

  void BasicStepperMotor::setActualPositionActions(int actualPositionInSteps){
    resetMotorControllerPositions(actualPositionInSteps);

    _motorControler->setPositiveReferenceSwitchCalibration(std::numeric_limits<int>::max());
    _motorControler->setNegativeReferenceSwitchCalibration(std::numeric_limits<int>::min());
    _calibrationMode.exchange(StepperMotorCalibrationMode::SIMPLE);
    _motorControler->setCalibrationTime(static_cast<uint32_t>(time(nullptr)));
  }

  void BasicStepperMotor::setActualPositionInSteps(int actualPositionInSteps){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    setActualPositionActions(actualPositionInSteps);
  }

  void BasicStepperMotor::setActualPosition(float actualPosition){
    setActualPositionInSteps(_stepperMotorUnitsConverter->unitsToSteps(actualPosition));
  }

  void BasicStepperMotor::translateAxisActions(int translationInSteps){
    int actualPosition = _motorControler->getActualPosition();

    resetMotorControllerPositions(actualPosition+translationInSteps);
    translateLimits(translationInSteps);
  }

  void BasicStepperMotor::translateLimits(int translationInSteps){

    if (checkIfOverflow(_maxPositionLimitInSteps, translationInSteps)){
      _maxPositionLimitInSteps = std::numeric_limits<int>::max();
    }
    else{
      _maxPositionLimitInSteps = _maxPositionLimitInSteps + translationInSteps;
    }
    if (checkIfOverflow(_minPositionLimitInSteps, translationInSteps)){
      _minPositionLimitInSteps = std::numeric_limits<int>::min();
    }
    else{
      _minPositionLimitInSteps = _minPositionLimitInSteps + translationInSteps;
    }
  }

  void BasicStepperMotor::translateAxisInSteps(int translationInSteps){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    translateAxisActions(translationInSteps);
  }

  void BasicStepperMotor::translateAxis(float translationInUnits){
    translateAxisInSteps(_stepperMotorUnitsConverter->unitsToSteps(translationInUnits));
  }

  bool BasicStepperMotor::checkIfOverflow(int termA, int termB){
    int signTermA = (termA > 0) - (termA < 0);
    int signTermB = (termB > 0) - (termB < 0);
    if (signTermA*signTermB <= 0){
      return false;
    }
    else if (signTermA > 0){
      if (termB > std::numeric_limits<int>::max() - termA){
        return true;
      }
      else{
        return false;
      }
    }
    else{
      if (termB < std::numeric_limits<int>::min() - termA){
        return true;
      }
      else{
        return false;
      }
    }
  }

  int BasicStepperMotor::getCurrentPositionInSteps(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return (_motorControler->getActualPosition());
  }

  float BasicStepperMotor::getCurrentPosition(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_motorControler->getActualPosition());
  }

  double BasicStepperMotor::getEncoderPosition(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    //return _encoderUnitToStepsRatio * (static_cast<int>(_motorControler->getDecoderPosition()) + _encoderPositionOffset);
    return _encoderUnitsConverter->stepsToUnits(static_cast<int>(_motorControler->getDecoderPosition()) + _encoderPositionOffset);
  }

  void BasicStepperMotor::setActualEncoderPosition(double referencePosition){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if(!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
//    _encoderPositionOffset = static_cast<int>(referencePosition/_encoderUnitToStepsRatio)
//                             - static_cast<int>(_motorControler->getDecoderPosition());
    _encoderPositionOffset = _encoderUnitsConverter->unitsToSteps(referencePosition)
                             - static_cast<int>(_motorControler->getDecoderPosition());
  }

  int BasicStepperMotor::getTargetPositionInSteps(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _motorControler->getTargetPosition();
  }

  float BasicStepperMotor::getTargetPosition(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_motorControler->getTargetPosition());
  }

  StepperMotorConfigurationResult BasicStepperMotor::setStepperMotorUnitsConverter(std::shared_ptr<StepperMotorUnitsConverter> stepperMotorUnitsConverter){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      return StepperMotorConfigurationResult::ERROR_SYSTEM_IN_ACTION;
    }
    else{
      if (stepperMotorUnitsConverter == nullptr){
        return StepperMotorConfigurationResult::ERROR_INVALID_PARAMETER;
      }
      _stepperMotorUnitsConverter = stepperMotorUnitsConverter;
      return StepperMotorConfigurationResult::SUCCESS;
    }
  }

  void BasicStepperMotor::setStepperMotorUnitsConverterToDefault(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    _stepperMotorUnitsConverter = std::make_unique<StepperMotorUnitsConverterTrivia>();
  }

  std::string BasicStepperMotor::getState(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _stateMachine->getCurrentState()->getName();
  }

  bool BasicStepperMotor::isSystemIdle(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return stateMachineInIdleAndNoEvent();
  }

  void BasicStepperMotor::waitForIdle(){
    while(1){
      usleep(100);
      if (isSystemIdle()){
        break;
      }
    }
  }

  StepperMotorError BasicStepperMotor::getError(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    StepperMotorError _motorError = NO_ERROR;
    if (stateMachineInIdleAndNoEvent() && (_motorControler->getTargetPosition() != _motorControler->getActualPosition())){
      _motorError = ACTION_ERROR;
    }
    //FIXME This is a temporary hack, currently only emergency stops sets error state
    //TODO Change this, so that the StepperMotor has a error property
    else if(_stateMachine->getCurrentState()->getName() == "error"){
      _motorError = EMERGENCY_STOP;
    }
    return _motorError;
  }

  bool BasicStepperMotor::isCalibrated(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    uint32_t calibTime = _motorControler->getCalibrationTime();
    if (calibTime == 0){
      return false;
    }else{
      return true;
    }
  }

  uint32_t BasicStepperMotor::getCalibrationTime(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _motorControler->getCalibrationTime();
  }

  void BasicStepperMotor::setEnabled(bool enable){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (enable){
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::enableEvent);
    }else{
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::disableEvent);
    }
  }

  bool BasicStepperMotor::getEnabled(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    //Note:  For the old firmware version isEndSwitchPowerEnabled always returns
    //       false (End switch power is not applicable in this scenario).
    return (_motorControler->isEndSwitchPowerEnabled() ||
      _motorControler->isMotorCurrentEnabled());
  }

  void BasicStepperMotor::setAutostart(bool autostart){
    boost::lock_guard<boost::mutex> guard(_mutex);
    _autostart = autostart;
  }

  bool BasicStepperMotor::getAutostart(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _autostart;
  }
  void BasicStepperMotor::setLogLevel(ChimeraTK::Logger::LogLevel newLevel){
    boost::lock_guard<boost::mutex> guard(_mutex);
    _logger.setLogLevel(newLevel);
  }

  Logger::LogLevel BasicStepperMotor::getLogLevel() {
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _logger.getLogLevel();
  }

  double BasicStepperMotor::getMaxSpeedCapability() {
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _motorControler->getMaxSpeedCapability();
  }

  double BasicStepperMotor::getSafeCurrentLimit() {
    boost::lock_guard<boost::mutex> guard(_mutex);
    return (_motorControler->getMaxCurrentLimit());
  }

  double BasicStepperMotor::setUserCurrentLimit(double currentInAmps) {
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    return (_motorControler->setUserCurrentLimit(currentInAmps));
  }

  double BasicStepperMotor::getUserCurrentLimit() {
    boost::lock_guard<boost::mutex> guard(_mutex);
    return (_motorControler->getUserCurrentLimit());
  }

  bool BasicStepperMotor::setUserSpeedLimit(double newSpeed) {
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      return false;
    }
    else{
      _motorControler->setUserSpeedLimit(newSpeed);
      return true;
    }
  }

  double BasicStepperMotor::getUserSpeedLimit() {
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _motorControler->getUserSpeedLimit();
  }

  bool BasicStepperMotor::isMoving() {
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _motorControler->isMotorMoving();
  }

  void BasicStepperMotor::enableFullStepping(bool enable){
    boost::lock_guard<boost::mutex> guard(_mutex);
    _motorControler->enableFullStepping(enable);
  }

  bool BasicStepperMotor::isFullStepping(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _motorControler->isFullStepping();
  }

  bool BasicStepperMotor::stateMachineInIdleAndNoEvent(){
    std::string stateName =  _stateMachine->getCurrentState()->getName();
    if (stateName == "disabled" || stateName == "idle"){
      return true;
    }else{
      return false;
    }
  }

  void BasicStepperMotor::initStateMachine(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if(_stateMachine->getCurrentState()->getName() == "initState"){
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::initialEvent);
    }
  }

  bool BasicStepperMotor::hasHWReferenceSwitches(){
      return false;
  }

  void BasicStepperMotor::calibrate(){
    throw MotorDriverException("This routine is not available for the BasicStepperMotor", MotorDriverException::NOT_IMPLEMENTED);
  }

  void BasicStepperMotor::determineTolerance(){
    throw MotorDriverException("This routine is not available for the BasicStepperMotor", MotorDriverException::NOT_IMPLEMENTED);
  }

  float BasicStepperMotor::getPositiveEndReference(){
    throw MotorDriverException("This routine is not available for the BasicStepperMotor", MotorDriverException::NOT_IMPLEMENTED);
  }

  int BasicStepperMotor::getPositiveEndReferenceInSteps(){
    throw MotorDriverException("This routine is not available for the BasicStepperMotor", MotorDriverException::NOT_IMPLEMENTED);
  }

  float BasicStepperMotor::getNegativeEndReference(){
    throw MotorDriverException("This routine is not available for the BasicStepperMotor", MotorDriverException::NOT_IMPLEMENTED);
  }

  int BasicStepperMotor::getNegativeEndReferenceInSteps(){
    throw MotorDriverException("This routine is not available for the BasicStepperMotor", MotorDriverException::NOT_IMPLEMENTED);
  }

  float BasicStepperMotor::getTolerancePositiveEndSwitch(){
    throw MotorDriverException("This routine is not available for the BasicStepperMotor", MotorDriverException::NOT_IMPLEMENTED);
  }

  float BasicStepperMotor::getToleranceNegativeEndSwitch(){
    throw MotorDriverException("This routine is not available for the BasicStepperMotor", MotorDriverException::NOT_IMPLEMENTED);
  }

  bool BasicStepperMotor::isPositiveReferenceActive(){
    throw MotorDriverException("This routine is not available for the BasicStepperMotor", MotorDriverException::NOT_IMPLEMENTED);
  }

  bool BasicStepperMotor::isNegativeReferenceActive(){
    throw MotorDriverException("This routine is not available for the BasicStepperMotor", MotorDriverException::NOT_IMPLEMENTED);
  }

  bool BasicStepperMotor::isPositiveEndSwitchEnabled(){
    throw MotorDriverException("This routine is not available for the BasicStepperMotor", MotorDriverException::NOT_IMPLEMENTED);
  }

  bool BasicStepperMotor::isNegativeEndSwitchEnabled(){
    throw MotorDriverException("This routine is not available for the BasicStepperMotor", MotorDriverException::NOT_IMPLEMENTED);
  }

  StepperMotorCalibrationMode BasicStepperMotor::getCalibrationMode(){
    return _calibrationMode.load();
  }
}


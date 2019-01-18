#include "BasicStepperMotor.h"
#include <cmath>
#include <future>
#include <chrono>
#include "MotorDriverCardFactory.h"
#include "MotorDriverException.h"
#include "StepperMotorException.h"
#include "StepperMotorStateMachine.h"

using LockGuard = boost::lock_guard<boost::mutex>;

namespace ChimeraTK{

  BasicStepperMotor::BasicStepperMotor(const StepperMotorParameters & parameters)
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

  StepperMotorRet BasicStepperMotor::checkNewPosition(int newPositionInSteps){
//    if (motorActive()){
//      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
//    }
    if (_motorControler->getCalibrationTime() == 0){
      throw MotorDriverException("motor not calibrated", MotorDriverException::NOT_IMPLEMENTED);
    }
    if (!limitsOK(newPositionInSteps)){
      throw MotorDriverException("new position out of range of the soft limits", MotorDriverException::OUT_OF_RANGE);
    }
    return StepperMotorRet::SUCCESS;
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

  StepperMotorRet BasicStepperMotor::moveRelative(float delta){
    return moveRelativeInSteps(_stepperMotorUnitsConverter->unitsToSteps(delta));
  }

  StepperMotorRet BasicStepperMotor::moveRelativeInSteps(int delta){
    LockGuard guard(_mutex);

    if(motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }

    int newPosition = _motorControler->getActualPosition() + delta;
    checkNewPosition(newPosition);
    _targetPositionInSteps =  newPosition;

    _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::moveEvent);
    return StepperMotorRet::SUCCESS;
  }

  void BasicStepperMotor::setTargetPosition(float newPosition){
    setTargetPositionInSteps(_stepperMotorUnitsConverter->unitsToSteps(newPosition));
  }

  void BasicStepperMotor::setTargetPositionInSteps(int newPositionInSteps){
    LockGuard guard(_mutex);

    checkNewPosition(newPositionInSteps);
    _targetPositionInSteps = newPositionInSteps;

    if(_stateMachine->getCurrentState()->getName() == "moving"){
      _motorControler->setTargetPosition(_targetPositionInSteps);
    }
    else if(_autostart){
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::moveEvent);
    }
  }

  void BasicStepperMotor::start(){
    LockGuard guard(_mutex);
    _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::moveEvent);
  }

  void BasicStepperMotor::stop(){
    LockGuard guard(_mutex);
    _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::stopEvent);
  }

  void BasicStepperMotor::emergencyStop(){
    LockGuard guard(_mutex);
    _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::emergencyStopEvent);
  }

  void BasicStepperMotor::resetError(){
    LockGuard guard(_mutex);

    if(!_motorControler->isMotorCurrentEnabled()){
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::resetToDisableEvent);
    }
    else{
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::resetToIdleEvent);
    }
  }

  int BasicStepperMotor::recalculateUnitsInSteps(float units){
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->unitsToSteps(units);
  }

  float BasicStepperMotor::recalculateStepsInUnits(int steps){
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(steps);
  }

  StepperMotorRet BasicStepperMotor::setSoftwareLimitsEnabled(bool enabled){
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }
    _softwareLimitsEnabled = enabled;
    return StepperMotorRet::SUCCESS;
  }

  bool BasicStepperMotor::getSoftwareLimitsEnabled(){
    LockGuard guard(_mutex);
    return _softwareLimitsEnabled;
  }

  StepperMotorRet BasicStepperMotor::setMaxPositionLimitInSteps(int maxPos){
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }else if(maxPos <= _minPositionLimitInSteps){
      throw MotorDriverException("minimum limit not smaller than maximum limit", MotorDriverException::NOT_IMPLEMENTED);
    }
    _maxPositionLimitInSteps = maxPos;
    return StepperMotorRet::SUCCESS;
  }

  StepperMotorRet BasicStepperMotor::setMaxPositionLimit(float maxPos){
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }else if(_stepperMotorUnitsConverter->unitsToSteps(maxPos) <= _minPositionLimitInSteps){
      throw MotorDriverException("minimum limit not smaller than maximum limit", MotorDriverException::NOT_IMPLEMENTED);
    }
    _maxPositionLimitInSteps = _stepperMotorUnitsConverter->unitsToSteps(maxPos);
    return StepperMotorRet::SUCCESS;
  }

  StepperMotorRet BasicStepperMotor::setMinPositionLimitInSteps(int minPos){
      LockGuard guard(_mutex);
      if (motorActive()){
        return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
      }else if(minPos >= _maxPositionLimitInSteps){
        throw MotorDriverException("minimum limit not smaller than maximum limit", MotorDriverException::NOT_IMPLEMENTED);
      }
      _minPositionLimitInSteps = minPos;
      return StepperMotorRet::SUCCESS;
    }

  StepperMotorRet BasicStepperMotor::setMinPositionLimit(float minPos){
      LockGuard guard(_mutex);
      if (motorActive()){
        return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
      }else if(_stepperMotorUnitsConverter->unitsToSteps(minPos) >= _maxPositionLimitInSteps){
        throw MotorDriverException("minimum limit not smaller than maximum limit", MotorDriverException::NOT_IMPLEMENTED);
      }
      _minPositionLimitInSteps = _stepperMotorUnitsConverter->unitsToSteps(minPos);
      return StepperMotorRet::SUCCESS;
    }

  int BasicStepperMotor::getMaxPositionLimitInSteps(){
    LockGuard guard(_mutex);
    return _maxPositionLimitInSteps;
  }

  float BasicStepperMotor::getMaxPositionLimit(){
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_maxPositionLimitInSteps);
  }

  int BasicStepperMotor::getMinPositionLimitInSteps(){
    LockGuard guard(_mutex);
    return _minPositionLimitInSteps;
  }

  float BasicStepperMotor::getMinPositionLimit(){
    LockGuard guard(_mutex);
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

  StepperMotorRet BasicStepperMotor::setActualPositionInSteps(int actualPositionInSteps){
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }
    setActualPositionActions(actualPositionInSteps);
    return StepperMotorRet::SUCCESS;
  }

  StepperMotorRet BasicStepperMotor::setActualPosition(float actualPosition){
    return setActualPositionInSteps(_stepperMotorUnitsConverter->unitsToSteps(actualPosition));
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

  StepperMotorRet BasicStepperMotor::translateAxisInSteps(int translationInSteps){
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }
    translateAxisActions(translationInSteps);
    return StepperMotorRet::SUCCESS;
  }

  StepperMotorRet BasicStepperMotor::translateAxis(float translationInUnits){
    return translateAxisInSteps(_stepperMotorUnitsConverter->unitsToSteps(translationInUnits));
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
    LockGuard guard(_mutex);
    return (_motorControler->getActualPosition());
  }

  float BasicStepperMotor::getCurrentPosition(){
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_motorControler->getActualPosition());
  }

  double BasicStepperMotor::getEncoderPosition(){
    LockGuard guard(_mutex);
    //return _encoderUnitToStepsRatio * (static_cast<int>(_motorControler->getDecoderPosition()) + _encoderPositionOffset);
    return _encoderUnitsConverter->stepsToUnits(static_cast<int>(_motorControler->getDecoderPosition()) + _encoderPositionOffset);
  }

  StepperMotorRet BasicStepperMotor::setActualEncoderPosition(double referencePosition){
    LockGuard guard(_mutex);
    if(motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }

    _encoderPositionOffset = _encoderUnitsConverter->unitsToSteps(referencePosition)
                             - static_cast<int>(_motorControler->getDecoderPosition());
    return StepperMotorRet::SUCCESS;
  }

  int BasicStepperMotor::getTargetPositionInSteps(){
    LockGuard guard(_mutex);
    return _motorControler->getTargetPosition();
  }

  float BasicStepperMotor::getTargetPosition(){
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_motorControler->getTargetPosition());
  }

  StepperMotorRet BasicStepperMotor::setStepperMotorUnitsConverter(std::shared_ptr<StepperMotorUnitsConverter> stepperMotorUnitsConverter){
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }
    else{
      if (stepperMotorUnitsConverter == nullptr){
        return StepperMotorRet::ERROR_INVALID_PARAMETER;
      }
      _stepperMotorUnitsConverter = stepperMotorUnitsConverter;
      return StepperMotorRet::SUCCESS;
    }
  }

  StepperMotorRet BasicStepperMotor::setStepperMotorUnitsConverterToDefault(){
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }
    _stepperMotorUnitsConverter = std::make_unique<StepperMotorUnitsConverterTrivia>();
    return StepperMotorRet::SUCCESS;
  }

  std::string BasicStepperMotor::getState(){
    LockGuard guard(_mutex);
    return _stateMachine->getCurrentState()->getName();
  }

  bool BasicStepperMotor::isSystemIdle(){
    LockGuard guard(_mutex);
    return !motorActive();
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
    LockGuard guard(_mutex);
    StepperMotorError _motorError = NO_ERROR;
    if (!motorActive() && (_motorControler->getTargetPosition() != _motorControler->getActualPosition())){
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
    LockGuard guard(_mutex);
    uint32_t calibTime = _motorControler->getCalibrationTime();
    if (calibTime == 0){
      return false;
    }else{
      return true;
    }
  }

  uint32_t BasicStepperMotor::getCalibrationTime(){
    LockGuard guard(_mutex);
    return _motorControler->getCalibrationTime();
  }

  void BasicStepperMotor::setEnabled(bool enable){
    LockGuard guard(_mutex);
    if (enable){
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::enableEvent);
    }else{
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::disableEvent);
    }
  }

  bool BasicStepperMotor::getEnabled(){
    LockGuard guard(_mutex);
    //Note:  For the old firmware version isEndSwitchPowerEnabled always returns
    //       false (End switch power is not applicable in this scenario).
    return (_motorControler->isEndSwitchPowerEnabled() ||
      _motorControler->isMotorCurrentEnabled());
  }

  void BasicStepperMotor::setAutostart(bool autostart){
    LockGuard guard(_mutex);
    _autostart = autostart;
  }

  bool BasicStepperMotor::getAutostart(){
    LockGuard guard(_mutex);
    return _autostart;
  }
  void BasicStepperMotor::setLogLevel(ChimeraTK::Logger::LogLevel newLevel){
    LockGuard guard(_mutex);
    _logger.setLogLevel(newLevel);
  }

  Logger::LogLevel BasicStepperMotor::getLogLevel() {
    LockGuard guard(_mutex);
    return _logger.getLogLevel();
  }

  double BasicStepperMotor::getMaxSpeedCapability() {
    LockGuard guard(_mutex);
    return _motorControler->getMaxSpeedCapability();
  }

  double BasicStepperMotor::getSafeCurrentLimit() {
    LockGuard guard(_mutex);
    return (_motorControler->getMaxCurrentLimit());
  }

  StepperMotorRet BasicStepperMotor::setUserCurrentLimit(double currentInAmps) {
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }
    _motorControler->setUserCurrentLimit(currentInAmps);
    return StepperMotorRet::SUCCESS;
  }

  double BasicStepperMotor::getUserCurrentLimit() {
    LockGuard guard(_mutex);
    return _motorControler->getUserCurrentLimit();
  }

  StepperMotorRet BasicStepperMotor::setUserSpeedLimit(double speedInUstepsPerSec) {
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }
    else{
      _motorControler->setUserSpeedLimit(speedInUstepsPerSec);
      return StepperMotorRet::SUCCESS;
    }
  }

  double BasicStepperMotor::getUserSpeedLimit() {
    LockGuard guard(_mutex);
    return _motorControler->getUserSpeedLimit();
  }

  bool BasicStepperMotor::isMoving() {
    LockGuard guard(_mutex);
    return _motorControler->isMotorMoving();
  }

  void BasicStepperMotor::enableFullStepping(bool enable){
    LockGuard guard(_mutex);
    _motorControler->enableFullStepping(enable);
  }

  bool BasicStepperMotor::isFullStepping(){
    LockGuard guard(_mutex);
    return _motorControler->isFullStepping();
  }

  bool BasicStepperMotor::motorActive(){
    std::string stateName =  _stateMachine->getCurrentState()->getName();
    if (stateName == "moving" || stateName == "calibrating"){
      return true;
    }else{
      return false;
    }
  }

  void BasicStepperMotor::initStateMachine(){
    LockGuard guard(_mutex);
    if(_stateMachine->getCurrentState()->getName() == "initState"){
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::initialEvent);
    }
  }

  bool BasicStepperMotor::hasHWReferenceSwitches(){
      return false;
  }

  StepperMotorRet BasicStepperMotor::calibrate(){
    throw MotorDriverException("This routine is not available for the BasicStepperMotor", MotorDriverException::NOT_IMPLEMENTED);
  }

  StepperMotorRet BasicStepperMotor::determineTolerance(){
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


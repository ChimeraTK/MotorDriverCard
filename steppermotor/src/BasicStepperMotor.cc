#include "BasicStepperMotor.h"
#include <cmath>
#include <future>
#include <chrono>
#include "MotorDriverCardFactory.h"
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
      _errorMode(StepperMotorError::NO_ERROR),
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
     _errorMode(StepperMotorError::NO_ERROR),
     _calibrationMode(StepperMotorCalibrationMode::NONE){}

  BasicStepperMotor::~BasicStepperMotor(){}

  StepperMotorRet BasicStepperMotor::checkNewPosition(int newPositionInSteps){

    if (_calibrationMode.load() == StepperMotorCalibrationMode::NONE){
      return StepperMotorRet::ERR_SYSTEM_NOT_CALIBRATED;
    }
    if (!limitsOK(newPositionInSteps)){
      return StepperMotorRet::ERR_INVALID_PARAMETER;
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
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
    }

    int newPosition = _motorControler->getActualPosition() + delta;
    auto checkResult =  checkNewPosition(newPosition);
    if(checkResult != StepperMotorRet::SUCCESS){
      return checkResult;
    }

    _targetPositionInSteps = newPosition;

    _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::moveEvent);
    return StepperMotorRet::SUCCESS;
  }

  StepperMotorRet BasicStepperMotor::setTargetPosition(float newPosition){
    return setTargetPositionInSteps(_stepperMotorUnitsConverter->unitsToSteps(newPosition));
  }

  StepperMotorRet BasicStepperMotor::setTargetPositionInSteps(int newPositionInSteps){
    LockGuard guard(_mutex);

    auto checkResult = checkNewPosition(newPositionInSteps);
    if(checkResult != StepperMotorRet::SUCCESS){
      return checkResult;
    }
    _targetPositionInSteps = newPositionInSteps;

    if(_stateMachine->getCurrentState()->getName() == "moving"){
      _motorControler->setTargetPosition(_targetPositionInSteps);
    }
    else if(_autostart){
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::moveEvent);
    }
    return StepperMotorRet::SUCCESS;
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

    _errorMode.exchange(StepperMotorError::NO_ERROR);

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
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
    }
    _softwareLimitsEnabled = enabled;
    return StepperMotorRet::SUCCESS;
  }

  bool BasicStepperMotor::getSoftwareLimitsEnabled(){
    LockGuard guard(_mutex);
    return _softwareLimitsEnabled;
  }

  StepperMotorRet BasicStepperMotor::setMaxPositionLimitInSteps(int maxPosInSteps){
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
    }else if(maxPosInSteps <= _minPositionLimitInSteps){
      return StepperMotorRet::ERR_INVALID_PARAMETER;
    }
    _maxPositionLimitInSteps = maxPosInSteps;
    return StepperMotorRet::SUCCESS;
  }

  StepperMotorRet BasicStepperMotor::setMaxPositionLimit(float maxPosInUnits){
    return setMaxPositionLimitInSteps(_stepperMotorUnitsConverter->unitsToSteps(maxPosInUnits));
  }

  StepperMotorRet BasicStepperMotor::setMinPositionLimitInSteps(int minPosInSteps){
      LockGuard guard(_mutex);
      if (motorActive()){
        return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
      }else if(minPosInSteps >= _maxPositionLimitInSteps){
        return StepperMotorRet::ERR_INVALID_PARAMETER;
      }
      _minPositionLimitInSteps = minPosInSteps;
      return StepperMotorRet::SUCCESS;
    }

  StepperMotorRet BasicStepperMotor::setMinPositionLimit(float minPosInUnits){
    return setMinPositionLimitInSteps(_stepperMotorUnitsConverter->unitsToSteps(minPosInUnits));
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
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
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
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
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
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
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
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
    }
    else{
      if (stepperMotorUnitsConverter == nullptr){
        return StepperMotorRet::ERR_INVALID_PARAMETER;
      }
      _stepperMotorUnitsConverter = stepperMotorUnitsConverter;
      return StepperMotorRet::SUCCESS;
    }
  }

  StepperMotorRet BasicStepperMotor::setStepperMotorUnitsConverterToDefault(){
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
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
    //return !motorActive();
    // FIXME Return to this
    std::string state = _stateMachine->getCurrentState()->getName();
    return state == "idle" || state == "disabled";
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
    return _errorMode.load();
  }

  bool BasicStepperMotor::isCalibrated(){
    return _calibrationMode.load() != StepperMotorCalibrationMode::NONE;
//    LockGuard guard(_mutex);
//    uint32_t calibTime = _motorControler->getCalibrationTime();
//    if (calibTime == 0){
//      return false;
//    }else{
//      return true;
//    }
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

    double currentLimit = 0.;
    /*
     * While the MotorControlerDummy does not implement
     * the function, the exception is remapped to not
     * expose the mtca4u namespace in the interface
     */
    try {
      currentLimit = _motorControler->getMaxCurrentLimit();
    } catch (mtca4u::MotorDriverException &e) {
      throw StepperMotorException(e.what(), StepperMotorException::FEATURE_NOT_AVAILABLE);
    }
    return currentLimit;
  }

  StepperMotorRet BasicStepperMotor::setUserCurrentLimit(double currentInAmps) {
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
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
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
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
    if (stateName == "moving"){
      return true;
    }else{
      return false;
    }
  }

  bool BasicStepperMotor::verifyMoveAction(){
    return _motorControler->getTargetPosition() == _motorControler->getActualPosition();
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
    throw StepperMotorException("This routine is not available for the BasicStepperMotor", StepperMotorException::FEATURE_NOT_AVAILABLE);
  }

  StepperMotorRet BasicStepperMotor::determineTolerance(){
    throw StepperMotorException("This routine is not available for the BasicStepperMotor", StepperMotorException::FEATURE_NOT_AVAILABLE);
  }

  float BasicStepperMotor::getPositiveEndReference(){
    throw StepperMotorException("This routine is not available for the BasicStepperMotor",StepperMotorException::FEATURE_NOT_AVAILABLE);
  }

  int BasicStepperMotor::getPositiveEndReferenceInSteps(){
    throw StepperMotorException("This routine is not available for the BasicStepperMotor",StepperMotorException::FEATURE_NOT_AVAILABLE);
  }

  float BasicStepperMotor::getNegativeEndReference(){
    throw StepperMotorException("This routine is not available for the BasicStepperMotor",StepperMotorException::FEATURE_NOT_AVAILABLE);
  }

  int BasicStepperMotor::getNegativeEndReferenceInSteps(){
    throw StepperMotorException("This routine is not available for the BasicStepperMotor",StepperMotorException::FEATURE_NOT_AVAILABLE);
  }

  float BasicStepperMotor::getTolerancePositiveEndSwitch(){
    throw StepperMotorException("This routine is not available for the BasicStepperMotor",StepperMotorException::FEATURE_NOT_AVAILABLE);
  }

  float BasicStepperMotor::getToleranceNegativeEndSwitch(){
    throw StepperMotorException("This routine is not available for the BasicStepperMotor",StepperMotorException::FEATURE_NOT_AVAILABLE);
  }

  bool BasicStepperMotor::isPositiveReferenceActive(){
    throw StepperMotorException("This routine is not available for the BasicStepperMotor",StepperMotorException::FEATURE_NOT_AVAILABLE);
  }

  bool BasicStepperMotor::isNegativeReferenceActive(){
    throw StepperMotorException("This routine is not available for the BasicStepperMotor",StepperMotorException::FEATURE_NOT_AVAILABLE);
  }

  bool BasicStepperMotor::isPositiveEndSwitchEnabled(){
    throw StepperMotorException("This routine is not available for the BasicStepperMotor",StepperMotorException::FEATURE_NOT_AVAILABLE);
  }

  bool BasicStepperMotor::isNegativeEndSwitchEnabled(){
    throw StepperMotorException("This routine is not available for the BasicStepperMotor",StepperMotorException::FEATURE_NOT_AVAILABLE);
  }

  StepperMotorCalibrationMode BasicStepperMotor::getCalibrationMode(){
    return _calibrationMode.load();
  }
}


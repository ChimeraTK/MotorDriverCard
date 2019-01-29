#include "impl/MotorControlerImpl.h"

#include "DFMC_MD22Constants.h"
using namespace mtca4u::dfmc_md22;
#include "TMC429Constants.h"
using namespace mtca4u::tmc429;

#include "impl/MotorDriverCardImpl.h"
#include <ChimeraTK/Device.h>
#include <cmath>

// just save some typing...
#define RAW_ACCESSOR_FROM_SUFFIX( MODULE_NAME, SUFFIX )\
  device->getScalarRegisterAccessor<int32_t>(MODULE_NAME + "/" + createMotorRegisterName(_id, SUFFIX), 0, {ChimeraTK::AccessMode::raw})

// Macros which cover more than one line are not good for the code coverage test, as only the 
// macro call is checked. In this case it is probably ok because we gain a lot of code because
// we have to define many variables, AND there is only one line per function without branching,
// which is covered by the function coverage test (not the line coverage test).
// Otherwise we would need a complete test which checks the macro.
#define DEFINE_GET_SET_VALUE(NAME, IDX)                                        \
  unsigned int MotorControlerImpl::get##NAME() {                               \
    lock_guard guard(_mutex);                                                  \
    return _controlerSPI->read(_id, IDX).getDATA();                            \
  }                                                                            \
  void MotorControlerImpl::set##NAME(unsigned int value) {                     \
    lock_guard guard(_mutex);                                                  \
    _controlerSPI->write(_id, IDX, value);                                     \
  }

#define DEFINE_SIGNED_GET_SET_VALUE(NAME, IDX, CONVERTER)                      \
  int MotorControlerImpl::get##NAME() {                                        \
    lock_guard guard(_mutex);                                                  \
    int readValue = static_cast<int>(_controlerSPI->read(_id, IDX).getDATA()); \
    return CONVERTER.customToThirtyTwo(readValue);                             \
  }                                                                            \
  void MotorControlerImpl::set##NAME(int value) {                              \
    lock_guard guard(_mutex);                                                  \
    unsigned int writeValue =                                                  \
        static_cast<unsigned int>(CONVERTER.thirtyTwoToCustom(value));         \
    _controlerSPI->write(_id, IDX, writeValue);                                \
  }

#define DEFINE_SET_GET_TYPED_CONTROLER_REGISTER(NAME)                          \
  NAME MotorControlerImpl::get##NAME() {                                       \
    lock_guard guard(_mutex);                                                  \
    return readTypedRegister<NAME>();                                          \
  }                                                                            \
  void MotorControlerImpl::set##NAME(NAME const& inputWord) {                  \
    lock_guard guard(_mutex);                                                  \
    writeTypedControlerRegister(inputWord);                                    \
  }

#define DEFINE_SET_GET_TYPED_DRIVER_DATA(NAME, LOCAL_DATA_INSTANCE)            \
  NAME const& MotorControlerImpl::get##NAME() const {                          \
    lock_guard guard(_mutex);                                                  \
    return LOCAL_DATA_INSTANCE;                                                \
  }                                                                            \
  void MotorControlerImpl::set##NAME(NAME const& driverData) {                 \
    lock_guard guard(_mutex);                                                  \
    setTypedDriverData<NAME>(driverData, LOCAL_DATA_INSTANCE);                 \
  }

typedef std::lock_guard<std::mutex> lock_guard;
typedef std::unique_lock<std::mutex> unique_lock;

namespace mtca4u
{
  MotorControlerImpl::MotorControlerImpl( unsigned int ID,
		  boost::shared_ptr< ChimeraTK::Device > const & device,
                  std::string const & moduleName,
                  boost::shared_ptr< TMC429SPI > const & controlerSPI,
		  MotorControlerConfig const & motorControlerConfig ) 
    : _mutex(), _device(device), _id(ID), _controlerConfig(motorControlerConfig),
      _conversionFactor( calculateConversionFactor(motorControlerConfig) ),
      _currentVmax(motorControlerConfig.maximumVelocity),
      _usrSetCurrentScale(_controlerConfig.stallGuardControlData.getCurrentScale()),
      _driverControlData(), //set later in the constructor body
      _chopperControlData(), //set later in the constructor body
      _coolStepControlData(), //set later in the constructor body
      _stallGuardControlData(), //set later in the constructor body
      _driverConfigData(), //set later in the constructor body
      _controlerStatus{device->getScalarRegisterAccessor<int32_t>(moduleName + "/" + CONTROLER_STATUS_BITS_ADDRESS_STRING, 0, {ChimeraTK::AccessMode::raw})},
      _actualPosition{RAW_ACCESSOR_FROM_SUFFIX(moduleName, ACTUAL_POSITION_SUFFIX)},
      _actualVelocity{RAW_ACCESSOR_FROM_SUFFIX(moduleName, ACTUAL_VELOCITY_SUFFIX)},
      _actualAcceleration{RAW_ACCESSOR_FROM_SUFFIX(moduleName, ACTUAL_ACCELETATION_SUFFIX)},
      _microStepCount{RAW_ACCESSOR_FROM_SUFFIX(moduleName, MICRO_STEP_COUNT_SUFFIX)},
      _stallGuardValue{RAW_ACCESSOR_FROM_SUFFIX(moduleName, STALL_GUARD_VALUE_SUFFIX)},
      _coolStepValue{RAW_ACCESSOR_FROM_SUFFIX(moduleName, COOL_STEP_VALUE_SUFFIX)},
      _status{RAW_ACCESSOR_FROM_SUFFIX(moduleName, STATUS_SUFFIX)},
      _motorCurrentEnabled{RAW_ACCESSOR_FROM_SUFFIX(moduleName, MOTOR_CURRENT_ENABLE_SUFFIX)},
      _decoderReadoutMode{RAW_ACCESSOR_FROM_SUFFIX(moduleName, DECODER_READOUT_MODE_SUFFIX)},
      _decoderPosition{RAW_ACCESSOR_FROM_SUFFIX(moduleName, DECODER_POSITION_SUFFIX)},
      _calibrationTime(),
      _calibratedPositiveEndSwitchPos(),
      _calibratedNegativeEndSwitchPos(),
      _endSwithPowerIndicator{},
      _driverSPI( device, moduleName,
                  createMotorRegisterName(ID, SPI_WRITE_SUFFIX ),
      createMotorRegisterName(ID, SPI_SYNC_SUFFIX ),
      motorControlerConfig.driverSpiWaitingTime),
      _controlerSPI(controlerSPI),
      converter24bits(24), converter12bits(12),
      _moveOnlyFullStep(false),
      _userMicroStepSize(0),
      _localTargetPosition(0),
      _calibrationData()
  {
    setAccelerationThresholdData( motorControlerConfig.accelerationThresholdData );
    setChopperControlData( motorControlerConfig.chopperControlData );
    setCoolStepControlData( motorControlerConfig.coolStepControlData );
    setDecoderReadoutMode( motorControlerConfig.decoderReadoutMode );
    setDividersAndMicroStepResolutionData( motorControlerConfig.dividersAndMicroStepResolutionData );
    setDriverConfigData( motorControlerConfig.driverConfigData );
    setDriverControlData( motorControlerConfig.driverControlData );
    setInterruptData( motorControlerConfig.interruptData );
    setMaximumAcceleration( motorControlerConfig.maximumAcceleration );
    setMaximumVelocity(motorControlerConfig.maximumVelocity);
    setMicroStepCount( motorControlerConfig.microStepCount );
    setMinimumVelocity( motorControlerConfig.minimumVelocity );
    setPositionTolerance( motorControlerConfig.positionTolerance );
    setProportionalityFactorData( motorControlerConfig.proportionalityFactorData );
    setReferenceConfigAndRampModeData( motorControlerConfig.referenceConfigAndRampModeData );
    setStallGuardControlData( motorControlerConfig.stallGuardControlData );
    setTargetVelocity( motorControlerConfig.targetVelocity );

    _localTargetPosition = retrieveTargetPositonAndConvert();
    _userMicroStepSize = pow(2, motorControlerConfig.driverControlData.getMicroStepResolution());


    ChimeraTK::ScalarRegisterAccessor<int32_t> firmwareVersion
      = _device->getScalarRegisterAccessor<int32_t>(moduleName + "/" + PROJECT_VERSION_ADDRESS_STRING);
    firmwareVersion.read();

    if(firmwareVersion >= MIN_FW_VERSION_WITH_CALIB_BACKUP){
      _calibrationTime.replace(RAW_ACCESSOR_FROM_SUFFIX(moduleName, CALIBRATION_TIME_SUFFIX));

      _calibratedPositiveEndSwitchPos.replace(RAW_ACCESSOR_FROM_SUFFIX(moduleName, POS_ENDSW_CALIB_SUFFIX));
      _calibratedNegativeEndSwitchPos.replace(RAW_ACCESSOR_FROM_SUFFIX(moduleName, NEG_ENDSW_CALIB_SUFFIX));
    }
    else{
      _calibrationTime.replace(
        device->getScalarRegisterAccessor<int32_t>(moduleName + "/" + PROJECT_USER_REGISTER_ADDRESS_STRING, 0, {ChimeraTK::AccessMode::raw}));
    }

    // enabling the motor is the last step after setting all registers
    setEnabled( motorControlerConfig.enabled );
    try {
      // this must throw on mapfile not having this register
      _endSwithPowerIndicator.replace(RAW_ACCESSOR_FROM_SUFFIX(moduleName, ENDSWITCH_ENABLE_SUFFIX));
    }
    catch (std::exception& a) {
      // ignore exception when creating the accessor with the consequence that
      // _endSwithPowerIndicator stays uninitialized. This happens when the
      // library is used with old firmware that does not have the
      // WORD_M1_VOLTAGE_EN register in the mapfile. Methods using this accessor have to
      // check its isInitialised() method prior to using it.
    }
  }

  unsigned int MotorControlerImpl::getID(){
    lock_guard guard(_mutex);
    return _id;
  }
   
  int MotorControlerImpl::getActualPosition(){
    lock_guard guard(_mutex);
    return readPositionRegisterAndConvert();
  }

  int MotorControlerImpl::readPositionRegisterAndConvert(){
    _actualPosition.read();
    return converter24bits.customToThirtyTwo( _actualPosition );
  }

  unsigned int MotorControlerImpl::readRegisterAccessor( ChimeraTK::ScalarRegisterAccessor<int32_t>& readValue){
    readValue.read();
    return static_cast<unsigned int>(readValue);
  }

  void MotorControlerImpl::setActualPosition(int position){
    lock_guard guard(_mutex);
    _controlerSPI->write( _id, IDX_ACTUAL_POSITION,
			  static_cast<unsigned int>(converter24bits.thirtyTwoToCustom(position)) );
  }

  int MotorControlerImpl::getActualVelocity(){
    lock_guard guard(_mutex);
    _actualVelocity.read();
    return converter12bits.customToThirtyTwo( _actualVelocity );
  }

  void MotorControlerImpl::setActualVelocity(int velocity){
    lock_guard guard(_mutex);
    _controlerSPI->write( _id, IDX_ACTUAL_VELOCITY, 
			  static_cast<unsigned int>(converter12bits.thirtyTwoToCustom(velocity)) );
  }

  unsigned int MotorControlerImpl::getActualAcceleration(){
    lock_guard guard(_mutex);
    return readRegisterAccessor( _actualAcceleration );
  }

  void MotorControlerImpl::setActualAcceleration(unsigned int acceleration){
    lock_guard guard(_mutex);
    _controlerSPI->write( _id, IDX_ACTUAL_ACCELERATION, acceleration );
  }

  unsigned int MotorControlerImpl::getMicroStepCount(){
    lock_guard guard(_mutex);
    return readRegisterAccessor( _microStepCount );
  }

  void MotorControlerImpl::setMicroStepCount(unsigned int microStepCount){
    lock_guard guard(_mutex);
    _controlerSPI->write( _id, IDX_MICRO_STEP_COUNT, microStepCount );
  }

  unsigned int MotorControlerImpl::getCoolStepValue(){
    lock_guard guard(_mutex);
    return readRegisterAccessor( _coolStepValue );
  }
 
  unsigned int MotorControlerImpl::getStallGuardValue(){
    lock_guard guard(_mutex);
    return readRegisterAccessor( _stallGuardValue );
  }
 
  DriverStatusData MotorControlerImpl::getStatus(){
    lock_guard guard(_mutex);
    return DriverStatusData( readRegisterAccessor( _status ) );
  }
 
  void MotorControlerImpl::setDecoderReadoutMode(unsigned int readoutMode){
    lock_guard guard(_mutex);
    _decoderReadoutMode = static_cast<int32_t>(readoutMode);
    _decoderReadoutMode.write();
  }

  unsigned int MotorControlerImpl::getDecoderReadoutMode(){
    lock_guard guard(_mutex);
     return readRegisterAccessor( _decoderReadoutMode );
  }

  unsigned int MotorControlerImpl::getDecoderPosition(){
    lock_guard guard(_mutex);
     return readRegisterAccessor( _decoderPosition );
  }

  void MotorControlerImpl::setEnabled(bool enable) {
    // The mutex is locked in setMotorCurrentEnabled
    setMotorCurrentEnabled(enable);
  }

  bool MotorControlerImpl::isEnabled() {
    // Mutex is locked in isMotorCurrentEnabled
    return isMotorCurrentEnabled();
  }

  void MotorControlerImpl::enableFullStepping(bool enable){
    _moveOnlyFullStep = enable;
  }

  bool MotorControlerImpl::isFullStepping(){
    return _moveOnlyFullStep;
  }

  //DEFINE_SIGNED_GET_SET_VALUE( TargetPosition, IDX_TARGET_POSITION, converter24bits )
  int MotorControlerImpl::getTargetPosition() {
    lock_guard guard(_mutex);
    return retrieveTargetPositonAndConvert();
  }

  int MotorControlerImpl::retrieveTargetPositonAndConvert(){
    int readValue = static_cast<int>(_controlerSPI->read(_id, IDX_TARGET_POSITION).getDATA());
    return converter24bits.customToThirtyTwo(readValue);
  }

  void MotorControlerImpl::setTargetPosition(int value) {
    lock_guard guard(_mutex);

    InterruptData interupts;
    interupts = readTypedRegister<InterruptData>();
    interupts.setMaskFlags(255);
    interupts.setInterruptFlags(255);
    writeTypedControlerRegister(interupts);

    if (_moveOnlyFullStep){
      roundToNextFullStep(value);
    }

    _localTargetPosition = value;

    unsigned int writeValue =
    static_cast<unsigned int>(converter24bits.thirtyTwoToCustom(value));
    _controlerSPI->write(_id, IDX_TARGET_POSITION, writeValue);
  }

  void MotorControlerImpl::roundToNextFullStep(int &targetPosition){ //todo implementing
    int delta = targetPosition - readPositionRegisterAndConvert();
    int deltaMicroStep = delta * _userMicroStepSize;
    unsigned int actualMicroStepCount = readRegisterAccessor( _microStepCount );
    unsigned int newActualMicroStepCount = (actualMicroStepCount + deltaMicroStep) & 0x3FF;
    unsigned int distanceToPreviousFullStep =  (newActualMicroStepCount + 1) % 256;
    if (distanceToPreviousFullStep < 128){
      targetPosition = targetPosition - distanceToPreviousFullStep/_userMicroStepSize;
    }else{
      targetPosition = targetPosition + (256 - distanceToPreviousFullStep)/_userMicroStepSize;
    }
  }

  DEFINE_GET_SET_VALUE( MinimumVelocity, IDX_MINIMUM_VELOCITY )
  DEFINE_SIGNED_GET_SET_VALUE( TargetVelocity, IDX_TARGET_VELOCITY , converter12bits )
  DEFINE_GET_SET_VALUE( MaximumAcceleration, IDX_MAXIMUM_ACCELERATION )
  DEFINE_GET_SET_VALUE( PositionTolerance, IDX_DELTA_X_REFERENCE_TOLERANCE )
  DEFINE_GET_SET_VALUE( PositionLatched, IDX_POSITION_LATCHED )

  unsigned int MotorControlerImpl::getMaximumVelocity() {
    lock_guard guard(_mutex);
    return _controlerSPI->read(_id, IDX_MAXIMUM_VELOCITY).getDATA();
  }

  void MotorControlerImpl::setMaximumVelocity(unsigned int value) {
    lock_guard guard(_mutex);
    _currentVmax = value;
    _controlerSPI->write(_id, IDX_MAXIMUM_VELOCITY, value);
  }

  template<class T>
  T  MotorControlerImpl::readTypedRegister(){
    T typedWord;
    typedWord.setSMDA( _id );
    TMC429OutputWord readbackWord = _controlerSPI->read( _id, typedWord.getIDX_JDX());
    typedWord.setDATA( readbackWord.getDATA() );
    return typedWord;
  }

  double MotorControlerImpl::getMaxSpeedCapability() {
    // Does not really need a mutex; shared state is only from _controlerConfig,
    // which is const - hence no risk of data changing during parallel access.
    auto hardLimitForVmax = _controlerConfig.maximumVelocity;
    return (convertVMaxToUstepsPerSec(hardLimitForVmax));
  }

  double MotorControlerImpl::setUserSpeedLimit(double microStepsPerSecond) {
    auto validatedVmax = validateVMaxForHardware(convertUstepsPerSecToVmax(microStepsPerSecond));
    // This is the critical path; already protected by mutex internally.
    setMaximumVelocity(validatedVmax);
    return convertVMaxToUstepsPerSec(validatedVmax);
  }

  double MotorControlerImpl::getUserSpeedLimit() {
    lock_guard guard(_mutex);
    return convertVMaxToUstepsPerSec(_currentVmax);
  }

  void MotorControlerImpl::writeTypedControlerRegister(
      TMC429InputWord inputWord) {
    // set/overwrite the id with this motors id
    inputWord.setSMDA( _id );
    _controlerSPI->write( inputWord );
  }

  DEFINE_SET_GET_TYPED_CONTROLER_REGISTER( AccelerationThresholdData )
  DEFINE_SET_GET_TYPED_CONTROLER_REGISTER( ProportionalityFactorData )
  DEFINE_SET_GET_TYPED_CONTROLER_REGISTER( ReferenceConfigAndRampModeData )
  DEFINE_SET_GET_TYPED_CONTROLER_REGISTER( InterruptData )
  DEFINE_SET_GET_TYPED_CONTROLER_REGISTER( DividersAndMicroStepResolutionData )

  DEFINE_SET_GET_TYPED_DRIVER_DATA( DriverControlData, _driverControlData)
  DEFINE_SET_GET_TYPED_DRIVER_DATA( ChopperControlData, _chopperControlData)
  DEFINE_SET_GET_TYPED_DRIVER_DATA( CoolStepControlData, _coolStepControlData)
  DEFINE_SET_GET_TYPED_DRIVER_DATA( StallGuardControlData, _stallGuardControlData)
  DEFINE_SET_GET_TYPED_DRIVER_DATA( DriverConfigData, _driverConfigData)

  template <class T>
  void MotorControlerImpl::setTypedDriverData(T const & driverData, T & localDataInstance){
    _driverSPI.write( driverData.getDataWord() );
    // Remember the written word for readback.
    localDataInstance  = driverData;
        
  }

  MotorReferenceSwitchData MotorControlerImpl::getReferenceSwitchData(){
    lock_guard guard(_mutex);
    return retrieveReferenceSwitchStatus();
  }

  MotorReferenceSwitchData MotorControlerImpl::retrieveReferenceSwitchStatus(){
    // the bit pattern for the active flags
    unsigned int bitMask = 0x3 << 2*_id;

    unsigned int commonReferenceSwitchWord = _controlerSPI->read(SMDA_COMMON, JDX_REFERENCE_SWITCH).getDATA();
    unsigned int dataWord = (commonReferenceSwitchWord & bitMask) >> 2*_id;
    // note: the following code uses the implicit bool conversion to/from 0/1 to keep the code short.
    MotorReferenceSwitchData motorReferenceSwitchData(dataWord);

    auto configAndRampModeData = readTypedRegister<ReferenceConfigAndRampModeData>();
    motorReferenceSwitchData.setNegativeSwitchEnabled( !configAndRampModeData.getDISABLE_STOP_L() );
    motorReferenceSwitchData.setPositiveSwitchEnabled( !configAndRampModeData.getDISABLE_STOP_R() );

    return motorReferenceSwitchData;
  }

  void MotorControlerImpl::setNegativeReferenceSwitchEnabled(bool enableStatus){
    lock_guard guard(_mutex);
    auto referenceConfigAndRampModeData = readTypedRegister<ReferenceConfigAndRampModeData>();
    referenceConfigAndRampModeData.setDISABLE_STOP_L(!enableStatus);
    writeTypedControlerRegister(referenceConfigAndRampModeData);
  }

  void MotorControlerImpl::setPositiveReferenceSwitchEnabled(bool enableStatus){
    lock_guard guard(_mutex);
    auto referenceConfigAndRampModeData = readTypedRegister<ReferenceConfigAndRampModeData>();
    referenceConfigAndRampModeData.setDISABLE_STOP_R(!enableStatus);
    // setReferenceConfigAndRampModeData internally obtains mutex
    writeTypedControlerRegister(referenceConfigAndRampModeData);
  }

  void MotorControlerImpl::setCalibrationTime(uint32_t calibrationTime){
    lock_guard guard(_mutex);
    _calibrationTime = static_cast<int32_t>(calibrationTime);
    _calibrationTime.write();
  }

  uint32_t MotorControlerImpl::getCalibrationTime(){
    lock_guard guard(_mutex);
    _calibrationTime.read();
    return static_cast<uint32_t>(_calibrationTime);
  }

  void MotorControlerImpl::setCalibrationData(CalibrationData const & calibData){
    lock_guard guard(_mutex);

    _calibrationTime = static_cast<int32_t>(calibData.calibrationTime);
    _calibrationTime.write();
    _calibratedPositiveEndSwitchPos = calibData.posEndSwitchCalibration;
    _calibratedPositiveEndSwitchPos.write();
    _calibratedNegativeEndSwitchPos = calibData.negendSwitchCalibration;
    _calibratedNegativeEndSwitchPos.write();
    _calibratedPositiveEndSwitchTol = calibData.posEndSwitchTolerance;
    _calibratedPositiveEndSwitchTol.write();
    _calibratedNegativeEndSwitchTol = calibData.negEndSwitchTolerance;
    _calibratedNegativeEndSwitchTol.write();
  }

  CalibrationData const & MotorControlerImpl(){
    lock_guard guard(_mutex);

    return _calibrationData;

  }

  void MotorControlerImpl::setPositiveReferenceSwitchCalibration(int calibratedPosition){

     if(_calibratedPositiveEndSwitchPos.isInitialised()){
       lock_guard guard(_mutex);
       _calibratedPositiveEndSwitchPos = static_cast<int32_t>(calibratedPosition);
       _calibratedPositiveEndSwitchPos.write();
     }
  }

  int  MotorControlerImpl::getPositiveReferenceSwitchCalibration(){
    return 0; //TODO
  }

  void MotorControlerImpl::setNegativeReferenceSwitchCalibration(int calibratedPosition){
    //TODO
    (void)calibratedPosition;
  }

  int  MotorControlerImpl::getNegativeReferenceSwitchCalibration(){
    return 0; //TODO
  }

  bool MotorControlerImpl::targetPositionReached(){
    lock_guard guard(_mutex);
    _controlerStatus.read();
       TMC429StatusWord controlerStatusWord(_controlerStatus);

    return controlerStatusWord.getTargetPositionReached(_id);
  }

  unsigned int MotorControlerImpl::getReferenceSwitchBit(){
    lock_guard guard(_mutex);
    _controlerStatus.read();
    TMC429StatusWord controlerStatusWord(_controlerStatus);

    return controlerStatusWord.getReferenceSwitchBit(_id);
  }

  double MotorControlerImpl::convertVMaxToUstepsPerSec(double vMax) {
    // speedInUstepsPerSec = _conversionFactor * Vmax
    return (_conversionFactor * vMax);
  }

  double MotorControlerImpl::convertUstepsPerSecToVmax(double speedInUstepsPerSec) {
    // Vmax = round(speedInUstepsPerSec / _conversionFactor)
    return speedInUstepsPerSec/_conversionFactor;
  }

  double MotorControlerImpl::calculateConversionFactor(MotorControlerConfig const & motorControlerConfig) {
    // conversionFactor = fclk[Hz] / (2^pulse_div * 2048 *32)
    double systemClockInMhz = MD_22_DEFAULT_CLOCK_FREQ_MHZ;

    auto systemClkInHz = systemClockInMhz * 1000000;
    auto pulse_div = static_cast<double>(motorControlerConfig.dividersAndMicroStepResolutionData.getPulseDivider());

    auto conversionFactor = systemClkInHz/(exp2(pulse_div) * 2048 * 32);
    return conversionFactor;
  }

  double MotorControlerImpl::setUserCurrentLimit(double currentLimit) {
    auto currentScale = limitCurrentScale(convertAmpsToCurrentScale(currentLimit));
    // setCurrent scale internally acquires mutex and is protected against
    // parallel access.
    setCurrentScale(currentScale);
    return convertCurrentScaletoAmps(currentScale);
  }

  double MotorControlerImpl::getUserCurrentLimit() {
    lock_guard guard(_mutex);
    return convertCurrentScaletoAmps(_usrSetCurrentScale);
  }

  double MotorControlerImpl::getMaxCurrentLimit() {
    // Does not really need to be protected with mutex; there is no resource
    // that can be modified on another thread. _controlerConfig is a const
    // object.
    auto configuredCurrentScale = _controlerConfig.stallGuardControlData.getCurrentScale();
    return convertCurrentScaletoAmps(configuredCurrentScale);
  }

  unsigned int MotorControlerImpl::validateVMaxForHardware(
      double calculatedVmax) {
    unsigned int maxLimitForVmax = _controlerConfig.maximumVelocity;
    unsigned int minLimitForVmax = _controlerConfig.minimumVelocity;

    if (calculatedVmax > maxLimitForVmax) { return maxLimitForVmax; }
    if (calculatedVmax < minLimitForVmax) { return minLimitForVmax; }
    return static_cast<unsigned int>(
        std::floor(calculatedVmax)); // floor the VMax value so we stick below
                                     // the requested speed limit.
  }

  double MotorControlerImpl::convertCurrentScaletoAmps(
      unsigned int currentScale) {
    auto currentInAmps = (static_cast<double>(currentScale + 1) /
                          iMaxTMC260C_CURRENT_SCALE_VALUES) *
                         iMaxTMC260C_IN_AMPS;
    return currentInAmps;
  }

  double MotorControlerImpl::convertAmpsToCurrentScale(
      double currentInAmps) {
    auto currentScale = (currentInAmps / iMaxTMC260C_IN_AMPS *
                         iMaxTMC260C_CURRENT_SCALE_VALUES) - 1;
    return currentScale;
  }

  unsigned int MotorControlerImpl::limitCurrentScale(
      double calculatedCurrentScale) {
    auto hardLimitForCurrentScale = _controlerConfig.stallGuardControlData.getCurrentScale();
    if(calculatedCurrentScale < iMaxTMC260C_MIN_CURRENT_SCALE_VALUE){return iMaxTMC260C_MIN_CURRENT_SCALE_VALUE;}
    if(calculatedCurrentScale > hardLimitForCurrentScale){return hardLimitForCurrentScale;}
    return (static_cast<unsigned int>(std::floor(calculatedCurrentScale))); //floor so that we dont exceed the set limt
  }

  bool MotorControlerImpl::isMotorMoving() {
    lock_guard guard(_mutex);
    InterruptData interruptData = readTypedRegister<InterruptData>();
    if (interruptData.getINT_POS_END() || interruptData.getINT_STOP()){
      DriverStatusData status(readRegisterAccessor(_status));
      return !status.getStandstillIndicator();
      //if (motorStandsStill){
      //	std::cout << "INT_POS_END " << interruptData.getINT_POS_END() << std::endl;
      //	std::cout << "INT_REF_WRONG " << interruptData.getINT_REF_WRONG() << std::endl;
      //	std::cout << "INT_REF_MISS " << interruptData.getINT_REF_MISS() << std::endl;
      //	std::cout << "INT_STOP " << interruptData.getINT_STOP() << std::endl;
      //	std::cout << "INT_STOP_LEFT_LOW " << interruptData.getINT_STOP_LEFT_LOW() << std::endl;
      //	std::cout << "INT_STOP_RIGHT_LOW " << interruptData.getINT_STOP_RIGHT_LOW() << std::endl;
      //	std::cout << "INT_STOP_LEFT_HIGH " << interruptData.getINT_STOP_LEFT_HIGH() << std::endl;
      //	std::cout << "INT_STOP_RIGHT_HIGH " << interruptData.getINT_STOP_RIGHT_HIGH() << std::endl;
      //return false;
      //      }else{
      //	return true;
      //      }
    }
    int currentPos = readPositionRegisterAndConvert();

    if (_localTargetPosition == currentPos){
      DriverStatusData status(readRegisterAccessor(_status));
      return !status.getStandstillIndicator();
    }

    MotorReferenceSwitchData referenceSwitchData = retrieveReferenceSwitchStatus();
    if ((referenceSwitchData.getNegativeSwitchEnabled() &&  referenceSwitchData.getNegativeSwitchActive() &&
	_localTargetPosition <= currentPos) ||
	(referenceSwitchData.getPositiveSwitchEnabled() &&  referenceSwitchData.getPositiveSwitchActive() &&
	    _localTargetPosition >= currentPos) ){
      DriverStatusData status(readRegisterAccessor(_status));
      return !status.getStandstillIndicator();
    }
    return true;
  }

  void MotorControlerImpl::setMotorCurrentEnabled(bool enable) {
    lock_guard guard(_mutex);
    _motorCurrentEnabled = (enable ? 1 : 0);
    _motorCurrentEnabled.write();
  }

  bool MotorControlerImpl::isMotorCurrentEnabled() {
    lock_guard guard(_mutex);
    return readRegisterAccessor(_motorCurrentEnabled);
  }

  void MotorControlerImpl::setEndSwitchPowerEnabled(bool enable) {
    lock_guard guard(_mutex);
    if (!_endSwithPowerIndicator.isInitialised()) {
      return; // Meaning we are on an old firmware that does not support this
              // register. Do nothing
    }
    _endSwithPowerIndicator = (enable ? 3 : 0);
    _endSwithPowerIndicator.write();
  }

  bool MotorControlerImpl::isEndSwitchPowerEnabled() {
    lock_guard guard(_mutex);
    if (!_endSwithPowerIndicator.isInitialised()) {
      return false; // taking a convenient default to make logic easier in
      // StepperMotor getEnabled . With the old firmware status of the end
      // switches is a don't care condition anyway
    }
    return readRegisterAccessor(_endSwithPowerIndicator);
  }

  void MotorControlerImpl::setCurrentScale(unsigned int currentScale) {
    auto stallGuardData = _controlerConfig.stallGuardControlData;
    stallGuardData.setCurrentScale(currentScale);
    // mutex is acquired in setStallGuardControlData
    setStallGuardControlData(stallGuardData);
    _usrSetCurrentScale = currentScale;
  }

  } // namespace mtca4u

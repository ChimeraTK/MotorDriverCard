#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include <sstream>
#include <thread>

#include "DFMC_MD22Dummy.h"
#include "impl/MotorDriverCardImpl.h"
#include "impl/MotorControlerImpl.h"
#include "DFMC_MD22Constants.h"
#include "testWordFromPCIeAddress.h"
using namespace mtca4u::dfmc_md22;
#include "TMC429DummyConstants.h"
#include "TMC260DummyConstants.h"
#include "testWordFromSpiAddress.h"
#include <mtca4u/MapFileParser.h>
#include <mtca4u/Device.h>
#include <mtca4u/Utilities.h>

using namespace mtca4u::tmc429;
using namespace mtca4u::tmc260;

#include "MotorControlerConfigDefaults.h"
using namespace mtca4u;

#include "testConfigConstants.h"
#include "MotorDriverCardFactory.h"
#include <memory>
#define DECLARE_GET_SET_TEST( NAME )\
  void testGet ## NAME ();\
  void testSet ## NAME ()

#define ADD_GET_SET_TEST( NAME )\
  test_case* set ## NAME ## TestCase =  BOOST_CLASS_TEST_CASE( &MotorControlerTest::testSet ## NAME , motorControlerTest );\
  test_case* get ## NAME ## TestCase =  BOOST_CLASS_TEST_CASE( &MotorControlerTest::testGet ## NAME , motorControlerTest );\
  set ## NAME ## TestCase->depends_on( get ## NAME ## TestCase );\
  add( get ## NAME ## TestCase );\
  add( set ## NAME ## TestCase )

#define DEFINE_GET_SET_TEST( NAME, IDX, PATTERN )\
void MotorControlerTest::testGet ## NAME (){\
  unsigned int expectedValue = tmc429::testWordFromSpiAddress( _motorControler->getID(),\
                                                       IDX );\
  BOOST_CHECK( _motorControler->get ## NAME () == expectedValue );}\
void MotorControlerTest::testSet ## NAME (){\
  _motorControler->set ## NAME ( PATTERN );\
  BOOST_CHECK( _motorControler->get ## NAME () == (PATTERN & SPI_DATA_MASK) );}

#define DEFINE_SIGNED_GET_SET_TEST( NAME, IDX, NBITS )	\
void MotorControlerTest::testGet ## NAME (){\
  SignedIntConverter converter( NBITS );\
  unsigned int expectedValue = tmc429::testWordFromSpiAddress( _motorControler->getID(),\
                                                       IDX );\
  BOOST_CHECK( _motorControler->get ## NAME () == converter.customToThirtyTwo(expectedValue) ); \
 }\
void MotorControlerTest::testSet ## NAME (){\
  unsigned int positiveMask = (1 << (NBITS-1) ) -1 ;\
  unsigned int negativeBit  = (1 << (NBITS-1) ) ;\
  _motorControler->set ## NAME ( 0xAAAAAAAA & positiveMask );/* something positive */ \
  BOOST_CHECK( _motorControler->get ## NAME () == static_cast<int>(0xAAAAAAAA & positiveMask) );\
  _motorControler->set ## NAME ( (0x55555555 & positiveMask) | negativeBit );/* out of range positive with the custom negative bit active*/ \
  BOOST_CHECK( _motorControler->get ## NAME () == -static_cast<int>( (0xAAAAAAAA +1) & positiveMask) );/* is interpreted as negative*/ \
  _motorControler->set ## NAME ( -1 );/* a normal neagative number */	\
  BOOST_CHECK( _motorControler->get ## NAME () == -1 );\
  _motorControler->set ## NAME ( 0xFFFFFFFF & ~negativeBit );/* something negative with the negative bit of the custom variable not set*/	\
  BOOST_CHECK( _motorControler->get ## NAME () == static_cast<int>(0xFFFFFFFF & positiveMask) );/* is interpreted as positive */ \
}

#define DEFINE_TYPED_GET_SET_TEST( NAME )\
void MotorControlerTest::testGet ## NAME (){\
  testGetTypedData< NAME >( &MotorControlerImpl::get ## NAME );}\
void MotorControlerTest::testSet ## NAME (){\
  testSetTypedData< NAME >( &MotorControlerImpl::get ## NAME ,\
			    &MotorControlerImpl::set ## NAME );}

#define DEFINE_GET_SET_DRIVER_SPI_DATA( NAME, SPI_ADDRESS, CONFIG_DEFAULT, TEST_PATTERN ) \
void MotorControlerTest::testGet ## NAME (){\
  testGetDriverSpiData< NAME >( &MotorControlerImpl::get ## NAME , SPI_ADDRESS, CONFIG_DEFAULT );} \
void MotorControlerTest::testSet ## NAME (){\
  testSetDriverSpiData< NAME >( &MotorControlerImpl::set ## NAME ,\
                                &MotorControlerImpl::get ## NAME ,\
				SPI_ADDRESS,\
                                TEST_PATTERN );}

namespace mtca4u{

class MotorControlerTest{
public:
  MotorControlerTest(boost::shared_ptr<MotorControler> const & motorControler, 
		     boost::shared_ptr<RegisterInfoMap> & registerMapping,
		     boost::shared_ptr<DFMC_MD22Dummy> dummyDevice);
  // getID() is tested in the MotorDriver card, where 
  // different ID are known. This test is for just one
  // Motor with one ID, which is now known to be ok.
  DECLARE_GET_SET_TEST( ActualPosition );
  DECLARE_GET_SET_TEST( ActualVelocity );
  DECLARE_GET_SET_TEST( ActualAcceleration );
  DECLARE_GET_SET_TEST( MicroStepCount );

  // for testing direct read from pcie registers (with default values from the dummy)
  void testReadPCIeRegister( unsigned int(MotorControlerImpl::* readFunction)(void),
			     std::string const & registerSuffix);

  // for reading registers which do not return an int but a MultiVariableWord
  template<class T>
  void testReadTypedPCIeRegister( T (MotorControlerImpl::* readFunction)(void),
				  std::string const & registerSuffix);

  void testSetIsEnabled();

  DECLARE_GET_SET_TEST( DecoderReadoutMode );
  DECLARE_GET_SET_TEST( TargetPosition );
  DECLARE_GET_SET_TEST( MinimumVelocity );
  DECLARE_GET_SET_TEST( MaximumVelocity );
  DECLARE_GET_SET_TEST( TargetVelocity );
  DECLARE_GET_SET_TEST( MaximumAcceleration );
  DECLARE_GET_SET_TEST( PositionTolerance );
  DECLARE_GET_SET_TEST( PositionLatched );

  DECLARE_GET_SET_TEST( AccelerationThresholdData );
  DECLARE_GET_SET_TEST( ProportionalityFactorData );
  DECLARE_GET_SET_TEST( ReferenceConfigAndRampModeData );
  DECLARE_GET_SET_TEST( InterruptData );
  DECLARE_GET_SET_TEST( DividersAndMicroStepResolutionData );

  DECLARE_GET_SET_TEST( DriverControlData );
  DECLARE_GET_SET_TEST( ChopperControlData );
  DECLARE_GET_SET_TEST( CoolStepControlData );
  DECLARE_GET_SET_TEST( StallGuardControlData );
  DECLARE_GET_SET_TEST( DriverConfigData );

  void testGetReferenceSwitchData( boost::shared_ptr<MotorDriverCardImpl> motorDriverCard );
  void testSetReferenceSwitchEnabled();
  
  void testTargetPositionReached();
  void testGetReferenceSwitchBit();

  void testThreadSaftey();
  void testSetEndSwitchPowerEnabled();

private:
  boost::shared_ptr<MotorControlerImpl> _motorControler;
  boost::shared_ptr<RegisterInfoMap> _registerMapping;
  boost::shared_ptr<DFMC_MD22Dummy> _dummyDevice;

  template<class T>
  void testGetTypedData( T (MotorControlerImpl::* getterFunction)() );

  template<class T>
  void testSetTypedData( T (MotorControlerImpl::* getterFunction)(),
			       void (MotorControlerImpl::* setterFunction)(T const &) );
  template <class T>
  void testGetDriverSpiData( T const & (MotorControlerImpl::* getterFunction)() const,
			     unsigned int driverSpiAddress,
			     unsigned int configDefault);
  template <class T>
  void testSetDriverSpiData( void (MotorControlerImpl::* setterFunction)(T const &),
			     T const & (MotorControlerImpl::* getterFunction)() const,
			     unsigned int driverSpiAddress,
			     unsigned int testPattern);

  unsigned int testWordFromPCIeSuffix(std::string const & registerSuffix);
  void listOfMotorControllerPubilcMethods(boost::shared_ptr<MotorControler> controller);
  boost::shared_ptr<MotorControler> createMotorController(const std::string deviceName);

};

class  MotorControlerTestSuite : public test_suite{
private:
    boost::shared_ptr<MotorDriverCardImpl> _motorDriverCard;
public:
  MotorControlerTestSuite(std::string const & mapFileName) 
    : test_suite(" MotorControler test suite"),  _motorDriverCard(){
    //boost::shared_ptr<DFMC_MD22Dummy> dummyDevice( new DFMC_MD22Dummy(MODULE_NAME_0) );
  	std::list<std::string>parameters;
  	parameters.push_back(MODULE_NAME_0);
  	boost::shared_ptr<DFMC_MD22Dummy> dummyDevice (new DFMC_MD22Dummy(mapFileName, MODULE_NAME_0) );
    //dummyDevice->open( mapFileName );
  	//dummyDevice->open();
    MapFileParser fileParser;
    boost::shared_ptr<RegisterInfoMap> registerMapping = fileParser.parse(mapFileName);

    boost::shared_ptr< Device > device(new Device());
    MotorDriverCardConfig  motorDriverCardConfig;
    device->open(dummyDevice, registerMapping);
    //device->open( dummyDevice, registerMapping );
    // We need the motor driver card as private variable because it has to
    // survive the constructor. Otherwise the MotorControlers passed to
    // the tests will be invalid.
    _motorDriverCard.reset( new MotorDriverCardImpl( device,
                                                     MODULE_NAME_0,
    						     motorDriverCardConfig ) );
    boost::shared_ptr<DFMC_MD22Dummy> d = boost::static_pointer_cast<DFMC_MD22Dummy>(dummyDevice);
    dummyDevice->setRegistersForTesting();

    for (unsigned int i = 0; i < N_MOTORS_MAX ; ++i){
      boost::shared_ptr<MotorControlerTest> motorControlerTest( 
	       new MotorControlerTest( _motorDriverCard->getMotorControler( i ) ,
				       registerMapping, 
				       dummyDevice) );

      ADD_GET_SET_TEST( ActualPosition );
      ADD_GET_SET_TEST( ActualVelocity );
      ADD_GET_SET_TEST( ActualAcceleration );
      //      ADD_GET_SET_TEST( AccelerationThreshold );
      ADD_GET_SET_TEST( MicroStepCount );

      
      add( BOOST_TEST_CASE(
	      boost::bind( &MotorControlerTest::testReadPCIeRegister,
			   motorControlerTest,
			   &MotorControlerImpl::getStallGuardValue,  STALL_GUARD_VALUE_SUFFIX ) ));
      add( BOOST_TEST_CASE(
	      boost::bind( &MotorControlerTest::testReadPCIeRegister,
			   motorControlerTest,
			   &MotorControlerImpl::getCoolStepValue,  COOL_STEP_VALUE_SUFFIX ) ));
      add( BOOST_TEST_CASE(
			   boost::bind( &MotorControlerTest::testReadTypedPCIeRegister<DriverStatusData>,
			   motorControlerTest,
			   &MotorControler::getStatus, STATUS_SUFFIX ) ));

      add( BOOST_CLASS_TEST_CASE( &MotorControlerTest::testSetIsEnabled,
				  motorControlerTest ) );
  
      ADD_GET_SET_TEST( DecoderReadoutMode );

      add( BOOST_TEST_CASE(
	      boost::bind( &MotorControlerTest::testReadPCIeRegister,
			   motorControlerTest,
			   &MotorControler::getDecoderPosition, DECODER_POSITION_SUFFIX ) ));

      ADD_GET_SET_TEST( MinimumVelocity );
      ADD_GET_SET_TEST( MaximumVelocity );
      ADD_GET_SET_TEST( TargetVelocity ); 
      ADD_GET_SET_TEST( MaximumAcceleration );
      ADD_GET_SET_TEST( PositionTolerance );
      ADD_GET_SET_TEST( PositionLatched );

      ADD_GET_SET_TEST( AccelerationThresholdData );
      ADD_GET_SET_TEST( ProportionalityFactorData );
      ADD_GET_SET_TEST( ReferenceConfigAndRampModeData );
      ADD_GET_SET_TEST( InterruptData );
      ADD_GET_SET_TEST( TargetPosition );
      ADD_GET_SET_TEST( DividersAndMicroStepResolutionData );

      ADD_GET_SET_TEST( DriverControlData );
      ADD_GET_SET_TEST( ChopperControlData );
      ADD_GET_SET_TEST( CoolStepControlData );
      ADD_GET_SET_TEST( StallGuardControlData );
      ADD_GET_SET_TEST( DriverConfigData );

      add( BOOST_TEST_CASE( boost::bind( &MotorControlerTest::testGetReferenceSwitchData,
					 motorControlerTest,
					 _motorDriverCard) ));
      add( BOOST_CLASS_TEST_CASE( &MotorControlerTest::testSetReferenceSwitchEnabled,
				  motorControlerTest ) );
      add( BOOST_CLASS_TEST_CASE( &MotorControlerTest::testTargetPositionReached,
				  motorControlerTest ) );
      add( BOOST_CLASS_TEST_CASE( &MotorControlerTest::testGetReferenceSwitchBit,
				  motorControlerTest ) );

      add(BOOST_CLASS_TEST_CASE(&MotorControlerTest::testThreadSaftey, motorControlerTest));
      add(BOOST_CLASS_TEST_CASE(&MotorControlerTest::testSetEndSwitchPowerEnabled, motorControlerTest));

   }// for i < N_MOTORS_MAX
  }// constructor
};// test suite


MotorControlerTest::MotorControlerTest(boost::shared_ptr<MotorControler> const & motorControler,
				       boost::shared_ptr<RegisterInfoMap> & registerMapping,
				       boost::shared_ptr<DFMC_MD22Dummy> dummyDevice)
  : _motorControler(boost::dynamic_pointer_cast<MotorControlerImpl>(motorControler)),
    _registerMapping( registerMapping ), _dummyDevice(dummyDevice){
}

DEFINE_SIGNED_GET_SET_TEST( ActualPosition, IDX_ACTUAL_POSITION, 24 )
DEFINE_SIGNED_GET_SET_TEST( ActualVelocity, IDX_ACTUAL_VELOCITY, 12  )
DEFINE_GET_SET_TEST( ActualAcceleration, IDX_ACTUAL_ACCELERATION, 0xFFFFFF ) 
DEFINE_GET_SET_TEST( MicroStepCount, IDX_MICRO_STEP_COUNT, 0xAAAAAA ) 

void MotorControlerTest::testReadPCIeRegister( unsigned int(MotorControlerImpl::* readFunction)(void),
						std::string const & registerSuffix){
  unsigned int expectedValue = testWordFromPCIeSuffix(registerSuffix);
  std::stringstream message;
  message << "read () " <<  ((*_motorControler).*readFunction)() 
	  << ", expected " << expectedValue << std::endl;
  BOOST_CHECK_MESSAGE( ((*_motorControler).*readFunction)() == expectedValue , message.str());
}

void MotorControlerTest::testThreadSaftey() {
//FIXME: these tests may not be complex enough
// These tests are meant to be run with thread sanitizer/ Helgrind
  auto controller = createMotorController("CONTROLLER_TESTS_OLD_MAPFILE");
  auto apiList = std::bind(&MotorControlerTest::listOfMotorControllerPubilcMethods, this, controller);
  std::thread t1(apiList);
  std::thread t2(apiList);
  std::thread t3(apiList);
  std::thread t4(apiList);
  std::thread t5(apiList);
  std::thread t6(apiList);
  std::thread t7(apiList);
  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  t6.join();
  t7.join();
}


void MotorControlerTest::testSetEndSwitchPowerEnabled() {
  // Test with the old firmware mapfile which is lacking this register:
  // (MD22.0.WORD_M1_VOLTAGE_EN)
  auto controller_new = createMotorController("CONTROLLER_TESTS_NEW_MAPFILE");
  controller_new->setEndSwitchPowerEnabled(true);
  BOOST_CHECK(controller_new->isEndSwitchPowerEnabled() == true);
  controller_new->setEndSwitchPowerEnabled(false);
  BOOST_CHECK(controller_new->isEndSwitchPowerEnabled() == false);

  // Test with the newer firmware mapfile that has the WORD_M1_VOLTAGE_EN
  // register
  auto controller_old = createMotorController("CONTROLLER_TESTS_OLD_MAPFILE");
  controller_old->setEndSwitchPowerEnabled(false);
  BOOST_CHECK(controller_old->isEndSwitchPowerEnabled() == false);
  controller_old->setEndSwitchPowerEnabled(true);
  // Endswitch power is not applicable for the old board design and we default
  // to false regardless when working with old firmware
  BOOST_CHECK(controller_old->isEndSwitchPowerEnabled() == false);
}

unsigned int  MotorControlerTest::testWordFromPCIeSuffix(std::string const & registerSuffix){
  std::string registerName = createMotorRegisterName( _motorControler->getID(),
						      registerSuffix );
  RegisterInfoMap::RegisterInfo registerInfo;
  _registerMapping->getRegisterInfo( registerName, registerInfo, MODULE_NAME_0 );
  return testWordFromPCIeAddress( registerInfo.address );
}

template<class T>
void MotorControlerTest::testReadTypedPCIeRegister( T (MotorControlerImpl::* readFunction)(void),
						    std::string const & registerSuffix){
  unsigned int expectedValue = testWordFromPCIeSuffix(registerSuffix);
  std::stringstream message;
  message << "read () " <<  ((*_motorControler).*readFunction)().getDataWord()
	  << ", expected " << expectedValue << std::endl;
  BOOST_CHECK_MESSAGE( ((*_motorControler).*readFunction)().getDataWord() == expectedValue , message.str());
}

void MotorControlerTest::testGetDecoderReadoutMode(){
  testReadPCIeRegister( &MotorControlerImpl::getDecoderReadoutMode,
			DECODER_READOUT_MODE_SUFFIX );
}

void MotorControlerTest::testSetDecoderReadoutMode(){
  unsigned int readoutMode = _motorControler->getDecoderReadoutMode();
  _motorControler->setDecoderReadoutMode( ++readoutMode );
  BOOST_CHECK( _motorControler->getDecoderReadoutMode() == readoutMode );
}

void MotorControlerTest::testSetIsEnabled(){
  BOOST_CHECK( _motorControler->isEnabled() );
  _motorControler->setEnabled( false );
  BOOST_CHECK( _motorControler->isEnabled()==false );
  _motorControler->setEnabled();
  BOOST_CHECK( _motorControler->isEnabled() );
}

DEFINE_SIGNED_GET_SET_TEST( TargetPosition, IDX_TARGET_POSITION, 24 )
DEFINE_GET_SET_TEST( MinimumVelocity, IDX_MINIMUM_VELOCITY, 0xFFFFFF )
DEFINE_GET_SET_TEST( MaximumVelocity, IDX_MAXIMUM_VELOCITY, 0xAAAAAA )
DEFINE_SIGNED_GET_SET_TEST( TargetVelocity, IDX_TARGET_VELOCITY, 12 )
DEFINE_GET_SET_TEST( MaximumAcceleration, IDX_MAXIMUM_ACCELERATION, 0xFFFFFF )
DEFINE_GET_SET_TEST( PositionTolerance, IDX_DELTA_X_REFERENCE_TOLERANCE, 0xAAAAAA )
DEFINE_GET_SET_TEST( PositionLatched, IDX_POSITION_LATCHED, 0x555555 )

template<class T>
void MotorControlerTest::testGetTypedData( T (MotorControlerImpl::* getterFunction)() ){
  T typedWord;
  unsigned int idx = typedWord.getIDX_JDX();
  //std::cout << idx << std::endl;
  unsigned int expectedContent = tmc429::testWordFromSpiAddress( _motorControler->getID(),
								 idx );
  // this assignment checks that the delivered data object actually is the right type of object.
  typedWord = ((*_motorControler).*getterFunction)();
  BOOST_CHECK( typedWord.getDATA() == expectedContent );
  //std::cout << typedWord.getDATA() << " " << expectedContent << std::endl;
  BOOST_CHECK( typedWord.getSMDA() == _motorControler->getID() );
}

template<class T>
void MotorControlerTest::testSetTypedData( T (MotorControlerImpl::* getterFunction)(),
					   void (MotorControlerImpl::* setterFunction)(T const &) ){
  T typedWord = ((*_motorControler).*getterFunction)();
  unsigned int dataContent = typedWord.getDATA();
  typedWord.setDATA(++dataContent);
  ((*_motorControler).*setterFunction)(typedWord);
  BOOST_CHECK( ((*_motorControler).*getterFunction)() == typedWord );    
}

DEFINE_TYPED_GET_SET_TEST( AccelerationThresholdData )
DEFINE_TYPED_GET_SET_TEST( ProportionalityFactorData )
DEFINE_TYPED_GET_SET_TEST( ReferenceConfigAndRampModeData )
DEFINE_TYPED_GET_SET_TEST( InterruptData )
DEFINE_TYPED_GET_SET_TEST( DividersAndMicroStepResolutionData )

template <class T>
void MotorControlerTest::testGetDriverSpiData( T const & (MotorControlerImpl::* getterFunction)() const,
					       unsigned int driverSpiAddress,
					       unsigned int configDefault){
  T expectedWord = tmc260::testWordFromSpiAddress(driverSpiAddress, _motorControler->getID());

  BOOST_CHECK( _dummyDevice->readDriverSpiRegister( _motorControler->getID(), driverSpiAddress )==
	       expectedWord.getPayloadData() );
  BOOST_CHECK( ((*_motorControler).*getterFunction)() == T(configDefault) ); // T is uninitialised at the moment
}

template <class T>
void MotorControlerTest::testSetDriverSpiData( void (MotorControlerImpl::* setterFunction)(T const &),
					       T const & (MotorControlerImpl::* getterFunction)() const,
					       unsigned int driverSpiAddress,
					       unsigned int testPattern){
  T testWord( testPattern );
  ((*_motorControler).* setterFunction)(testWord);

  // the new word must arrive at the spi register in the "hardware" and a copy has to be in the motor controler
  BOOST_CHECK( _dummyDevice->readDriverSpiRegister( _motorControler->getID(), driverSpiAddress )==
	       testWord.getPayloadData() );
  BOOST_CHECK(((*_motorControler).* getterFunction)() == testWord );
}

DEFINE_GET_SET_DRIVER_SPI_DATA( DriverControlData, ADDRESS_DRIVER_CONTROL, DRIVER_CONTROL_DEFAULT , 0x2AAAA)
DEFINE_GET_SET_DRIVER_SPI_DATA( ChopperControlData, ADDRESS_CHOPPER_CONFIG, CHOPPER_CONTROL_DEFAULT , 0x1AAAA)
DEFINE_GET_SET_DRIVER_SPI_DATA( CoolStepControlData, ADDRESS_COOL_STEP_CONFIG, COOL_STEP_CONTROL_DEFAULT , 0x1AAAA)
DEFINE_GET_SET_DRIVER_SPI_DATA( StallGuardControlData, ADDRESS_STALL_GUARD_CONFIG, STALL_GUARD_CONTROL_DEFAULT, 0x1AAAA )
DEFINE_GET_SET_DRIVER_SPI_DATA( DriverConfigData, ADDRESS_DRIVER_CONFIG, DRIVER_CONFIG_DEFAULT , 0x1AAAA )

void MotorControlerTest::testGetReferenceSwitchData( boost::shared_ptr<MotorDriverCardImpl> motorDriverCard ){
  // This approach is intentinally clumsy and manual in order not to use the same algorithm as 
  // in the implementation, but still be felxible if the register content changes.
  unsigned int referenceWord=0; //setting to 0 to avoid compiler warning in default case.
  
  switch (_motorControler->getID()){
  case 0 : referenceWord = motorDriverCard->getReferenceSwitchData().getDATA() & 0x3; break;
  case 1 : referenceWord = ((motorDriverCard->getReferenceSwitchData().getDATA() & 0xC) >> 2); break;
  case 2 : referenceWord = ((motorDriverCard->getReferenceSwitchData().getDATA() & 0x30) >> 4); break;
  default:
    BOOST_FAIL("Invalid MotorID. Either there is something wrong in the code or the test has to be adapted.");
  }

  BOOST_CHECK(_motorControler->getReferenceSwitchData().getSwitchesActiveWord() == referenceWord );
  BOOST_CHECK(_motorControler->getReferenceSwitchData().getPositiveSwitchEnabled() ==
	      !_motorControler->getReferenceConfigAndRampModeData().getDISABLE_STOP_R() );
  BOOST_CHECK(_motorControler->getReferenceSwitchData().getNegativeSwitchEnabled() ==
	      !_motorControler->getReferenceConfigAndRampModeData().getDISABLE_STOP_L() );
}

void MotorControlerTest::testSetReferenceSwitchEnabled(){
  MotorReferenceSwitchData originalSetting = _motorControler->getReferenceSwitchData();

  _motorControler->setPositiveReferenceSwitchEnabled(true);
  _motorControler->setNegativeReferenceSwitchEnabled(true);
  BOOST_CHECK( _motorControler->getReferenceSwitchData().getSwitchesEnabledWord() == 0x3 );

  _motorControler->setPositiveReferenceSwitchEnabled(false);
  _motorControler->setNegativeReferenceSwitchEnabled(true);
  BOOST_CHECK( _motorControler->getReferenceSwitchData().getSwitchesEnabledWord() == 0x2 );

  _motorControler->setPositiveReferenceSwitchEnabled(true);
  _motorControler->setNegativeReferenceSwitchEnabled(false);
  BOOST_CHECK( _motorControler->getReferenceSwitchData().getSwitchesEnabledWord() == 0x1 );

  _motorControler->setPositiveReferenceSwitchEnabled(false);
  _motorControler->setNegativeReferenceSwitchEnabled(false);
  BOOST_CHECK( _motorControler->getReferenceSwitchData().getSwitchesEnabledWord() == 0x0 );
  
  _motorControler->setPositiveReferenceSwitchEnabled( originalSetting.getPositiveSwitchEnabled() );
  _motorControler->setNegativeReferenceSwitchEnabled( originalSetting.getNegativeSwitchEnabled() );

}

void MotorControlerTest::testTargetPositionReached(){
  RegisterInfoMap::RegisterInfo registerInfo;
  _registerMapping->getRegisterInfo( CONTROLER_STATUS_BITS_ADDRESS_STRING, registerInfo, MODULE_NAME_0 );

  TMC429StatusWord expectedControlerStatus( testWordFromPCIeAddress( registerInfo.address ) );

  BOOST_CHECK( expectedControlerStatus.getTargetPositionReached( _motorControler->getID() ) ==
	       _motorControler->targetPositionReached() );
}

void MotorControlerTest::testGetReferenceSwitchBit(){
  RegisterInfoMap::RegisterInfo registerInfo;
  _registerMapping->getRegisterInfo( CONTROLER_STATUS_BITS_ADDRESS_STRING, registerInfo, MODULE_NAME_0 );

  TMC429StatusWord expectedControlerStatus( testWordFromPCIeAddress( registerInfo.address ) );

  BOOST_CHECK( expectedControlerStatus.getReferenceSwitchBit( _motorControler->getID() ) ==
	       _motorControler->getReferenceSwitchBit() );
}

void MotorControlerTest::listOfMotorControllerPubilcMethods(
    boost::shared_ptr<MotorControler> controller) {

  auto controllerImpl = boost::dynamic_pointer_cast<MotorControlerImpl>(controller);
  controllerImpl->getID();
  controllerImpl->getStatus();

  controllerImpl->getStallGuardValue();
  controllerImpl->getCoolStepValue();
  controllerImpl->getDecoderPosition();
  controllerImpl->targetPositionReached();
  controllerImpl->getReferenceSwitchBit();

  controllerImpl->setActualVelocity(controllerImpl->getActualVelocity());
  controllerImpl->setActualAcceleration(controllerImpl->getActualAcceleration());
  controllerImpl->setMicroStepCount(controllerImpl->getMicroStepCount());
  controllerImpl->setEnabled();
  controllerImpl->setDecoderReadoutMode(controllerImpl->getDecoderReadoutMode());
  controllerImpl->setUserSpeedLimit(controllerImpl->getUserSpeedLimit());
  controllerImpl->setActualPosition(controllerImpl->getActualPosition());

  controllerImpl->getMaxSpeedCapability();
  controllerImpl->setUserCurrentLimit(controllerImpl->getUserCurrentLimit());
  controllerImpl->getMaxCurrentLimit();
  controllerImpl->isEnabled();
  controllerImpl->isMotorMoving();

  controllerImpl->setPositiveReferenceSwitchEnabled(
      controllerImpl->getReferenceSwitchData().getPositiveSwitchEnabled());
  controllerImpl->setNegativeReferenceSwitchEnabled(
      controllerImpl->getReferenceSwitchData().getNegativeSwitchEnabled());

  controllerImpl->setTargetPosition(controllerImpl->getTargetPosition());
  controllerImpl->setMinimumVelocity(controllerImpl->getActualVelocity());

  controllerImpl->setMaximumVelocity(controllerImpl->getMaximumVelocity());

  controllerImpl->setTargetVelocity(controllerImpl->getTargetVelocity());

  controllerImpl->setMaximumAcceleration(controllerImpl->getMaximumAcceleration());

  controllerImpl->setPositionTolerance(controllerImpl->getPositionTolerance());

  controllerImpl->setPositionLatched(controllerImpl->getPositionLatched());

  controllerImpl->setAccelerationThresholdData(
      controllerImpl->getAccelerationThresholdData());
  controllerImpl->setProportionalityFactorData(
      controllerImpl->getProportionalityFactorData());

  controllerImpl->setReferenceConfigAndRampModeData(
      controllerImpl->getReferenceConfigAndRampModeData());

  controllerImpl->setInterruptData(controllerImpl->getInterruptData());

  controllerImpl->setDividersAndMicroStepResolutionData(
      controllerImpl->getDividersAndMicroStepResolutionData());

  controllerImpl->setDriverControlData(controllerImpl->getDriverControlData());
  controllerImpl->setChopperControlData(controllerImpl->getChopperControlData());
  controllerImpl->setCoolStepControlData(controllerImpl->getCoolStepControlData());
  controllerImpl->setStallGuardControlData(controllerImpl->getStallGuardControlData());
  controllerImpl->setDriverConfigData(controllerImpl->getDriverConfigData());
}


boost::shared_ptr<MotorControler>
MotorControlerTest::createMotorController(const std::string deviceName) {
  mtca4u::setDMapFilePath("./dummies.dmap");
  boost::shared_ptr<mtca4u::Device> device(new mtca4u::Device());
  device->open(deviceName);
  boost::shared_ptr<MotorDriverCardImpl> driver(new MotorDriverCardImpl(device, "MD22_0", MotorDriverCardConfig()));
  return driver->getMotorControler(0);
}

} // namespace mtca4u

test_suite*
init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
   framework::master_test_suite().p_name.value = "MotorControler test suite";

   return new mtca4u::MotorControlerTestSuite(MAP_FILE_NAME);
}


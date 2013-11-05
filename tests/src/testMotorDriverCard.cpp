#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "DummyDevice.h"
#include "MotorDriverCardImpl.h"
#include <MtcaMappedDevice/devMap.h>
#include <MtcaMappedDevice/libmap.h>

#define MAP_FILE_NAME "DFMC_MD22_test.map"
#define BROKEN_MAP_FILE_NAME "DFMC_MD22_broken.map"

using namespace mtca4u;

class MotorDriverCardTest{
public:
  MotorDriverCardTest(std::string const & mapFileName);

  // the constructor might throw, better test it.
  void testConstructor();
private:
  boost::shared_ptr<MotorDriverCardImpl> motorDriverCard;
  std::string _mapFileName;
};

class  MotorDriverCardTestSuite : public test_suite{
 public:
  MotorDriverCardTestSuite() : test_suite(" MultiVariableWord test suite"){
    boost::shared_ptr<MotorDriverCardTest> motorDriverCardTest( new MotorDriverCardTest(MAP_FILE_NAME) );
    
    test_case* constrtuctorTestCase = BOOST_CLASS_TEST_CASE( &MotorDriverCardTest::testConstructor, motorDriverCardTest );
    //    test_case* writeTestCase = BOOST_CLASS_TEST_CASE( &MyClassTest::testWrite, motorDriverCardTest );

    //    writeTestCase->depends_on( readTestCase );

    add( constrtuctorTestCase );

  }
};

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
   framework::master_test_suite().p_name.value = "MotorDriverCard test suite";

  return new MotorDriverCardTestSuite;
}

MotorDriverCardTest::MotorDriverCardTest(std::string const & mapFileName)
  : _mapFileName(mapFileName){
}

void MotorDriverCardTest::testConstructor(){
  ref_count_pointer<devBase> dummyDevice( new DummyDevice );
  dummyDevice->openDev( _mapFileName );
 
  mapFileParser fileParser;
  ref_count_pointer<mapFile> registerMapping = fileParser.parse(_mapFileName);

  boost::shared_ptr< devMap<devBase> > mappedDevice(new devMap<devBase>);
  MotorDriverCardImpl::MotorDriverConfiguration  motorDriverConfiguration;

  // has to throw because the device is not open
  BOOST_CHECK_THROW( motorDriverCard = boost::shared_ptr<MotorDriverCardImpl>(
		       new MotorDriverCardImpl( mappedDevice, motorDriverConfiguration ) ),
		     //FIXME: create a DeviceException. Has to work for real and dummy devices
		     exBase );

  ref_count_pointer<mapFile> brokenRegisterMapping = fileParser.parse(BROKEN_MAP_FILE_NAME);
  // try opening with bad mapping, also has to throw
  mappedDevice->openDev( dummyDevice, brokenRegisterMapping );
  BOOST_CHECK_THROW( motorDriverCard = boost::shared_ptr<MotorDriverCardImpl>(
		       new MotorDriverCardImpl( mappedDevice, motorDriverConfiguration ) ),
		     exLibMap );
  
  mappedDevice->closeDev();
  mappedDevice->openDev( dummyDevice, registerMapping );
  
  BOOST_CHECK_NO_THROW(  motorDriverCard = boost::shared_ptr<MotorDriverCardImpl>(new MotorDriverCardImpl( mappedDevice, motorDriverConfiguration )) );
  
}

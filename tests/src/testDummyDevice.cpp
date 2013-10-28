#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#define TEST_MAPPING_FILE "utcadummy.map"
#define FIRMWARE_REGISTER_STRING "WORD_FIRMWARE"
#define STATUS_REGISTER_STRING "WORD_STATUS"
#define USER_REGISTER_STRING "WORD_USER"
#define CLOCK_MUX_REGISTER_STRING "WORD_CLK_MUX"

#include "DummyDevice.h"
using namespace mtca4u;

// forward declaration so we can declare it friend
// of TestableDummyDevice.
class DummyDeviceTest;

/** The TestableDummyDevice is derived from 
 *  DummyDevice to get access to the protected members.
 *  This is done by declaring DummyDeviceTest as a friend.
 */
class TestableDummyDevice : public DummyDevice
{
  friend class DummyDeviceTest;
};

class DummyDeviceTest{
public:
  DummyDeviceTest(){}

  static void testCalculateVirtualRegisterAddress();
  static void testCheckSizeIsMultipleOfWordSize();
  void testOpenClose();
  void testReadWriteSingleWordRegister();

private:
  TestableDummyDevice _dummyDevice;
};

class  DummyDeviceTestSuite : public test_suite{
 public:
  DummyDeviceTestSuite() : test_suite("DummyDevice test suite"){
    boost::shared_ptr<DummyDeviceTest> dummyDeviceTest( new DummyDeviceTest );
    
    test_case* openCloseTestCase = BOOST_CLASS_TEST_CASE( &DummyDeviceTest::testOpenClose, dummyDeviceTest );

    add( BOOST_TEST_CASE( DummyDeviceTest::testCalculateVirtualRegisterAddress ) );
    add( BOOST_TEST_CASE( DummyDeviceTest::testCheckSizeIsMultipleOfWordSize ) );
    add( openCloseTestCase );
    add( BOOST_CLASS_TEST_CASE( &DummyDeviceTest::testReadWriteSingleWordRegister, dummyDeviceTest ) );
  }
};

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
   framework::master_test_suite().p_name.value = "DummyDevice test suite";
   framework::master_test_suite().add(new DummyDeviceTestSuite);

   return NULL;
}

void DummyDeviceTest::testCalculateVirtualRegisterAddress(){
  BOOST_CHECK(  DummyDevice::calculateVirtualRegisterAddress( 0, 0 ) == 0UL );
  BOOST_CHECK(  DummyDevice::calculateVirtualRegisterAddress( 0x35, 0 ) == 0x35UL );
  BOOST_CHECK(  DummyDevice::calculateVirtualRegisterAddress( 0x67875, 0x3 ) == 0x3000000000067875UL );
  BOOST_CHECK(  DummyDevice::calculateVirtualRegisterAddress( 0, 0x4 ) == 0x4000000000000000UL );

  // the first bit of the bar has to be cropped
  BOOST_CHECK(  DummyDevice::calculateVirtualRegisterAddress( 0x123, 0xD ) == 0x5000000000000123UL );
}

void DummyDeviceTest::testCheckSizeIsMultipleOfWordSize(){
  // just some arbitrary numbers to test %4 = 0, 1, 2, 3
  BOOST_CHECK_NO_THROW(  DummyDevice::checkSizeIsMultipleOfWordSize(24) );
  BOOST_CHECK_THROW(  DummyDevice::checkSizeIsMultipleOfWordSize(25), DummyDeviceException );
  BOOST_CHECK_THROW(  DummyDevice::checkSizeIsMultipleOfWordSize(26), DummyDeviceException );
  BOOST_CHECK_THROW(  DummyDevice::checkSizeIsMultipleOfWordSize(27), DummyDeviceException );  
}

void DummyDeviceTest::testOpenClose(){
  _dummyDevice.openDev(TEST_MAPPING_FILE);

  // there has to be bars 0 and 2  with sizes 0x48 and 0x400
  BOOST_CHECK( _dummyDevice._barContents.size() == 2 );
  std::map< uint8_t, std::vector<int32_t> >::const_iterator bar0Iter =
    _dummyDevice._barContents.find(0);
  BOOST_REQUIRE( bar0Iter != _dummyDevice._barContents.end() );
  BOOST_CHECK( bar0Iter->second.size() == 0x12 );

  std::map< uint8_t, std::vector<int32_t> >::const_iterator bar2Iter =
    _dummyDevice._barContents.find(2);
  BOOST_REQUIRE( bar2Iter != _dummyDevice._barContents.end() );
  BOOST_CHECK( bar2Iter->second.size() == 0x400 );
  
  // the "prtmapFile" has an implicit converion to bool to check 
  // if it points to NULL
  BOOST_CHECK( _dummyDevice._registerMapping );
  BOOST_CHECK( _dummyDevice.isOpen() );

  BOOST_CHECK_THROW( _dummyDevice.openDev(TEST_MAPPING_FILE), DummyDeviceException );

  _dummyDevice.closeDev();
  // The map has to be empty and the reference counting pointer to the 
  // mapping has to point to NULL.
  BOOST_CHECK( _dummyDevice._barContents.empty() );
  BOOST_CHECK( _dummyDevice._registerMapping == false );
  BOOST_CHECK( _dummyDevice.isOpen() == false );

  BOOST_CHECK_THROW( _dummyDevice.closeDev(), DummyDeviceException );
}

void DummyDeviceTest::testReadWriteSingleWordRegister(){
  try{
    _dummyDevice.openDev(TEST_MAPPING_FILE);
  }
  catch(DummyDeviceException &){
    // make sure the device was freshly opened, so
    // registers are set to 0.
    _dummyDevice.closeDev();
    _dummyDevice.openDev(TEST_MAPPING_FILE);
  }

  mapFile::mapElem mappingElement;
  _dummyDevice._registerMapping->getRegisterInfo (FIRMWARE_REGISTER_STRING, mappingElement);

  uint32_t offset =  mappingElement.reg_address;
  uint8_t bar =  mappingElement.reg_bar;
  int32_t dataContent = -1;

  BOOST_CHECK_NO_THROW( _dummyDevice.readReg( offset, 
					      &dataContent,
					      bar ) );
  BOOST_CHECK( dataContent == 0 );

  dataContent = 47;
  BOOST_CHECK_NO_THROW( _dummyDevice.writeReg( offset, 
					       dataContent,
					       bar ) );
  dataContent = -1; // make sure the value is really being read
  // no need to test NO_THROW on the same register twice
  _dummyDevice.readReg( offset, &dataContent, bar );
  BOOST_CHECK( dataContent == 47 );

  // the size as index is invalid, allowed range is 0..size-1 included.
  BOOST_CHECK_THROW( _dummyDevice.readReg(  _dummyDevice._barContents[bar].size(),
					    &dataContent, bar),
		     DummyDeviceException );
  BOOST_CHECK_THROW( _dummyDevice.writeReg(  _dummyDevice._barContents[bar].size(),
					     dataContent, bar),
		     DummyDeviceException );
}

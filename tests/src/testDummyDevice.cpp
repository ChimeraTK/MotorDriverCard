#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;
#include <boost/lambda/lambda.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "DummyDevice.h"
#include "NotImplementedException.h"
using namespace mtca4u;

#define TEST_MAPPING_FILE "utcadummy.map"
#define FIRMWARE_REGISTER_STRING "WORD_FIRMWARE"
#define STATUS_REGISTER_STRING "WORD_STATUS"
#define USER_REGISTER_STRING "WORD_USER"
#define CLOCK_MUX_REGISTER_STRING "WORD_CLK_MUX"
#define CLOCK_RESET_REGISTER_STRING "WORD_CLK_RST"

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
  DummyDeviceTest(): a(0), b(0), c(0) {}

  static void testCalculateVirtualAddress();
  static void testCheckSizeIsMultipleOfWordSize();
  void testOpenClose();
  void testReadWriteSingleWordRegister();
  /// The read function can be readDMA or readArea, the writeFunction is always writeArea.
  /// WriteDMA probably does not make sense at all. 
  void testReadWriteMultiWordRegister(void(DummyDevice::* readFunction)(uint32_t, int32_t*, size_t, uint8_t));
  void testWriteDMA();
  void testReadDeviceInfo();
  void testReadOnly();
  void testWriteCallbackFunctions();

private:
  TestableDummyDevice _dummyDevice;
  void freshlyOpenDevice();
  friend class DummyDeviceTestSuite;

  // stuff for the callback function test
  int a, b, c;
  void increaseA(){ ++a; }
  void increaseB(){ ++b; }
  void increaseC(){ ++c; }
};

class  DummyDeviceTestSuite : public test_suite{
 public:
  DummyDeviceTestSuite() : test_suite("DummyDevice test suite"){
    boost::shared_ptr<DummyDeviceTest> dummyDeviceTest( new DummyDeviceTest );
    
    test_case* openCloseTestCase = BOOST_CLASS_TEST_CASE( &DummyDeviceTest::testOpenClose, dummyDeviceTest );

    boost::function<void(void)> testReadWriteArea 
      = boost::bind( &DummyDeviceTest::testReadWriteMultiWordRegister,
		     dummyDeviceTest, &DummyDevice::readArea);

    boost::function<void(void)> testReadWriteDMA 
      = boost::bind( &DummyDeviceTest::testReadWriteMultiWordRegister,
		     dummyDeviceTest, &DummyDevice::readDMA);

    add( BOOST_TEST_CASE( DummyDeviceTest::testCalculateVirtualAddress ) );
    add( BOOST_TEST_CASE( DummyDeviceTest::testCheckSizeIsMultipleOfWordSize ) );
    add( openCloseTestCase );
    add( BOOST_CLASS_TEST_CASE( &DummyDeviceTest::testReadWriteSingleWordRegister, dummyDeviceTest ) );
    add( BOOST_TEST_CASE( testReadWriteArea ) );
    add( BOOST_TEST_CASE( testReadWriteDMA ) );
    add( BOOST_CLASS_TEST_CASE( &DummyDeviceTest::testWriteDMA, dummyDeviceTest ) );
    add( BOOST_CLASS_TEST_CASE( &DummyDeviceTest::testReadDeviceInfo, dummyDeviceTest ) );
    add( BOOST_CLASS_TEST_CASE( &DummyDeviceTest::testReadOnly, dummyDeviceTest ) );
    add( BOOST_CLASS_TEST_CASE( &DummyDeviceTest::testWriteCallbackFunctions, dummyDeviceTest ) );
  }
};

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
   framework::master_test_suite().p_name.value = "DummyDevice test suite";
   framework::master_test_suite().add(new DummyDeviceTestSuite);

   return NULL;
}

void DummyDeviceTest::testCalculateVirtualAddress(){
  BOOST_CHECK(  DummyDevice::calculateVirtualAddress( 0, 0 ) == 0UL );
  BOOST_CHECK(  DummyDevice::calculateVirtualAddress( 0x35, 0 ) == 0x35UL );
  BOOST_CHECK(  DummyDevice::calculateVirtualAddress( 0x67875, 0x3 ) == 0x3000000000067875UL );
  BOOST_CHECK(  DummyDevice::calculateVirtualAddress( 0, 0x4 ) == 0x4000000000000000UL );

  // the first bit of the bar has to be cropped
  BOOST_CHECK(  DummyDevice::calculateVirtualAddress( 0x123, 0xD ) == 0x5000000000000123UL );
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
  freshlyOpenDevice();

  mapFile::mapElem mappingElement;
  _dummyDevice._registerMapping->getRegisterInfo (CLOCK_RESET_REGISTER_STRING, mappingElement);

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
  BOOST_CHECK_THROW( _dummyDevice.readReg(  _dummyDevice._barContents[bar].size()*sizeof(int32_t),
					    &dataContent, bar),
		     DummyDeviceException );
  BOOST_CHECK_THROW( _dummyDevice.writeReg(  _dummyDevice._barContents[bar].size()*sizeof(int32_t),
					     dataContent, bar),
		     DummyDeviceException );
}

//void DummyDeviceTest::testReadWriteMultiWordRegister(boost::function<void(uint32_t, int32_t*, size_t, uint8_t)> readFunction){
void DummyDeviceTest::testReadWriteMultiWordRegister(void(DummyDevice::* readFunction)(uint32_t, int32_t*, size_t, uint8_t)){
  freshlyOpenDevice();

  mapFile::mapElem mappingElement;
  _dummyDevice._registerMapping->getRegisterInfo (CLOCK_MUX_REGISTER_STRING, mappingElement);

  uint32_t offset =  mappingElement.reg_address;
  uint8_t bar =  mappingElement.reg_bar;
  size_t sizeInBytes =  mappingElement.reg_size;
  size_t sizeInWords =  mappingElement.reg_size / sizeof(int32_t);
  std::vector<int32_t> dataContent(sizeInWords, -1);

  BOOST_CHECK_NO_THROW( (_dummyDevice.*readFunction)( offset, 
						      &(dataContent[0]),
						      sizeInBytes,
						      bar ) );
  for( std::vector<int32_t>::iterator dataIter = dataContent.begin();
       dataIter != dataContent.end(); ++dataIter){
    std::stringstream errorMessage;
    errorMessage << "*dataIter = " <<  *dataIter;
    BOOST_CHECK_MESSAGE( *dataIter == 0, errorMessage.str() );
  }

  for( unsigned int index = 0; index < dataContent.size(); ++index ){
    dataContent[index] = static_cast<int32_t>((index+1) * (index+1));
  }
  BOOST_CHECK_NO_THROW( _dummyDevice.writeArea( offset, 
						&(dataContent[0]),
						sizeInBytes,
						bar ) );  
  // make sure the value is really being read
  std::for_each( dataContent.begin(), dataContent.end(), boost::lambda::_1 = -1);

  // no need to test NO_THROW on the same register twice
  _dummyDevice.readArea( offset, &(dataContent[0]), sizeInBytes, bar );

   // make sure the value is really being read
  for( unsigned int index = 0; index < dataContent.size(); ++index ){
    BOOST_CHECK( dataContent[index] == static_cast<int32_t>((index+1) * (index+1)) );
  }

  // tests for exceptions
  // 1. base address too large
  BOOST_CHECK_THROW( _dummyDevice.readArea( _dummyDevice._barContents[bar].size()*sizeof(int32_t),
					    &(dataContent[0]), sizeInBytes, bar),
		     DummyDeviceException );
  BOOST_CHECK_THROW( _dummyDevice.writeArea( _dummyDevice._barContents[bar].size()*sizeof(int32_t),
					     &(dataContent[0]), sizeInBytes, bar),
		     DummyDeviceException );
  // 2. size too large (works because the target register is not at offfset 0)
  // resize the data vector for this test
  dataContent.resize(_dummyDevice._barContents[bar].size());
  BOOST_CHECK_THROW( _dummyDevice.readArea( offset, &(dataContent[0]),
					    _dummyDevice._barContents[bar].size()*sizeof(int32_t),
					    bar),
		     DummyDeviceException );
  BOOST_CHECK_THROW( _dummyDevice.writeArea( offset, &(dataContent[0]),
					     _dummyDevice._barContents[bar].size()*sizeof(int32_t),
					     bar),
		     DummyDeviceException );
  // 3. size not multiple of 4
  BOOST_CHECK_THROW( _dummyDevice.readArea( offset, &(dataContent[0]),
					    sizeInBytes-1,
					    bar),
		     DummyDeviceException );
  BOOST_CHECK_THROW( _dummyDevice.writeArea( offset, &(dataContent[0]),
					     sizeInBytes-1,
					     bar),
		     DummyDeviceException );
}

void DummyDeviceTest::freshlyOpenDevice(){
  try{
    _dummyDevice.openDev(TEST_MAPPING_FILE);
  }
  catch(DummyDeviceException &){
    // make sure the device was freshly opened, so
    // registers are set to 0.
    _dummyDevice.closeDev();
    _dummyDevice.openDev(TEST_MAPPING_FILE);
  }
}

void DummyDeviceTest::testWriteDMA(){
  // will probably never be implemented
  BOOST_CHECK_THROW( _dummyDevice.writeDMA( 0, NULL, 0, 0 ), 
		     NotImplementedException );
}

void DummyDeviceTest::testReadDeviceInfo(){
  std::string deviceInfo;
  _dummyDevice.readDeviceInfo(&deviceInfo);
  BOOST_CHECK( deviceInfo == (std::string("DummyDevice with mapping file ") 
			      + TEST_MAPPING_FILE) );
}

void DummyDeviceTest::testReadOnly(){
  freshlyOpenDevice();

  mapFile::mapElem mappingElement;
  _dummyDevice._registerMapping->getRegisterInfo (CLOCK_MUX_REGISTER_STRING, mappingElement);

  uint32_t offset =  mappingElement.reg_address;
  uint8_t bar =  mappingElement.reg_bar;
  size_t sizeInBytes =  mappingElement.reg_size;
  size_t sizeInWords =  mappingElement.reg_size / sizeof(int32_t);
  std::stringstream errorMessage;
  errorMessage << "This register should have 4 words. "
	       << "If you changed your mapping you have to adapt "
	       << "the testReadOnly() test.";
  BOOST_REQUIRE_MESSAGE(sizeInWords == 4, errorMessage.str());

  std::vector<int32_t> dataContent(sizeInBytes);
  for( unsigned int index = 0; index < dataContent.size(); ++index ){
    dataContent[index] = static_cast<int32_t>((index+1) * (index+1));
  }
  _dummyDevice.writeArea(offset, &(dataContent[0]), sizeInBytes, bar);
  _dummyDevice.setReadOnly( offset, bar, 1);
  
  // the actual test: write 42 to all registers, register 0 must not change, all others have to
  std::for_each( dataContent.begin(), dataContent.end(), boost::lambda::_1 = 42);
  _dummyDevice.writeArea(offset, &(dataContent[0]), sizeInBytes, bar);
  std::for_each( dataContent.begin(), dataContent.end(), boost::lambda::_1 = -1);
  _dummyDevice.readArea(offset, &(dataContent[0]), sizeInBytes, bar);
  BOOST_CHECK( dataContent[0] == 1 );
  BOOST_CHECK( dataContent[1] == 42 );
  BOOST_CHECK( dataContent[2] == 42 );
  BOOST_CHECK( dataContent[3] == 42 );

  // also set the last two words to read only. Now only the second word has to change.
  _dummyDevice.setReadOnly( offset + 2*sizeof(int32_t),  bar, 2);
   std::for_each( dataContent.begin(), dataContent.end(), boost::lambda::_1 = 29);
   // also test with single write operations
   for(size_t index = 0; index < sizeInWords; ++index){
     _dummyDevice.writeReg(offset + index*sizeof(int32_t), dataContent[index], bar);
   }

  std::for_each( dataContent.begin(), dataContent.end(), boost::lambda::_1 = -1);
  _dummyDevice.readArea(offset, &(dataContent[0]), sizeInBytes, bar);
  BOOST_CHECK( dataContent[0] == 1 );
  BOOST_CHECK( dataContent[1] == 29 );
  BOOST_CHECK( dataContent[2] == 42 );
  BOOST_CHECK( dataContent[3] == 42 );  
}

void DummyDeviceTest::testWriteCallbackFunctions(){
  BOOST_FAIL("test not impemented yet");
}

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#define TEST_MAPPING_FILE "utcadummy.map"

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
  void testOpenClose();

private:
  TestableDummyDevice _dummyDevice;
};

class FakeRegisterTest{
public:
  FakeRegisterTest() : _dataWord(0) {}
  static void testConstructor();
  static void testReadWrite();
  static void testReadOnly();
  void testWriteCallbackFunction();

private:
  static const size_t _seven = 7;

  /// The data word is a class variable which can
  /// be modified by increaseDataWord
  int _dataWord;
  /// Increases the DataWord. This function is intended
  /// as a callback function for the FakeRegister.
  void increaseDataWord();
};

class  FakeRegisterTestSuite : public test_suite{
 public:
  FakeRegisterTestSuite() : test_suite("DummyDevice::fakeRegister test suite"){
    boost::shared_ptr<FakeRegisterTest> fakeRegisterTest( new FakeRegisterTest );
    
    add( BOOST_TEST_CASE( &FakeRegisterTest::testConstructor ) );
    add( BOOST_TEST_CASE( &FakeRegisterTest::testReadWrite ) );
    add( BOOST_TEST_CASE( &FakeRegisterTest::testReadOnly ) );
    add( BOOST_CLASS_TEST_CASE( &FakeRegisterTest::testWriteCallbackFunction, fakeRegisterTest ) );

    //add( BOOST_TEST_CASE( DummyDeviceTest::testCalculateVirtualRegisterAddress ) );
    //add( openCloseTestCase );
  }
};

class  DummyDeviceTestSuite : public test_suite{
 public:
  DummyDeviceTestSuite() : test_suite("DummyDevice test suite"){
    boost::shared_ptr<DummyDeviceTest> dummyDeviceTest( new DummyDeviceTest );
    
    test_case* openCloseTestCase = BOOST_CLASS_TEST_CASE( &DummyDeviceTest::testOpenClose, dummyDeviceTest );

    add( BOOST_TEST_CASE( DummyDeviceTest::testCalculateVirtualRegisterAddress ) );
    add( openCloseTestCase );
  }
};

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
   framework::master_test_suite().p_name.value = "DummyDevice test suite";
   framework::master_test_suite().add(new FakeRegisterTestSuite);
   //   framework::master_test_suite().add(new DummyDeviceTestSuite);

   return NULL;
}

void FakeRegisterTest::testConstructor(){
  // create a register of size seven
  DummyDevice::FakeRegister fakeRegister(_seven);
  // the register size must be seven words, and the register must be writeable
  BOOST_CHECK( fakeRegister.sizeInWords() == _seven);
  BOOST_CHECK( fakeRegister.isReadOnly() == false);  

  // all words have to be 0
  for( size_t offset = 0; offset <  fakeRegister.sizeInWords(); ++offset ){
    BOOST_CHECK( fakeRegister.read(offset) == 0 );
  }
  
}

void FakeRegisterTest::testReadWrite(){
  // prepare the test. seven words with content offset^2
  DummyDevice::FakeRegister fakeRegister(_seven);
  for( size_t offset = 0; offset <  fakeRegister.sizeInWords(); ++offset ){
    fakeRegister.write( static_cast<int32_t>(offset*offset), offset);
  }

  BOOST_CHECK_THROW( fakeRegister.write(static_cast<int32_t>(_seven*_seven), _seven), std::out_of_range );
  
  for( size_t offset = 0; offset <  fakeRegister.sizeInWords(); ++offset ){
    BOOST_CHECK( fakeRegister.read( offset) == static_cast<int32_t>(offset*offset) );
  }
  
  BOOST_CHECK_THROW( fakeRegister.read(_seven), std::out_of_range );  
}

void FakeRegisterTest::testReadOnly(){
  DummyDevice::FakeRegister fakeRegister(1);
  
  fakeRegister.write(42);
  fakeRegister.setReadOnly();
  
  // overwriting should be ignored
  fakeRegister.write(43);
  BOOST_CHECK( fakeRegister.read() == 42 );

  // unset readOnly and try again
  fakeRegister.setReadOnly(false);
  fakeRegister.write(43);
  BOOST_CHECK( fakeRegister.read() == 43 );
}

void FakeRegisterTest::testWriteCallbackFunction(){
  DummyDevice::FakeRegister fakeRegister(1);
  fakeRegister.setWriteCallbackFunction( 
	 boost::bind( &FakeRegisterTest::increaseDataWord, this ) );
  _dataWord = 42;
  
  fakeRegister.write( 39 );// the 39 is arbitrary, what matters is that write is called.
  // if increaseDataWord has been executed as a callback, the dataWord has to be
  // increased
  BOOST_CHECK( _dataWord == 43 );
}

void FakeRegisterTest::increaseDataWord(){
  ++_dataWord;
}

void DummyDeviceTest::testOpenClose(){
  _dummyDevice.openDev(TEST_MAPPING_FILE);
  // there have to be eight entries in the registers map
  BOOST_CHECK( _dummyDevice._registers.size() == 8 );
  // the "prtmapFile" has an implicit converion to bool to check 
  // if it points to NULL
  BOOST_CHECK( _dummyDevice._registerMapping );
  BOOST_CHECK( _dummyDevice.isOpen() );

  _dummyDevice.closeDev();
  // The map has to be empty and the reference counting pointer to the 
  // mapping has to point to NULL.
  BOOST_CHECK( _dummyDevice._registers.empty() );
  BOOST_CHECK( _dummyDevice._registerMapping == false );
  BOOST_CHECK( _dummyDevice.isOpen() == false );
}

void DummyDeviceTest::testCalculateVirtualRegisterAddress(){
  BOOST_CHECK(  DummyDevice::calculateVirtualRegisterAddress( 0, 0 ) == 0UL );
  BOOST_CHECK(  DummyDevice::calculateVirtualRegisterAddress( 0x35, 0 ) == 0x35UL );
  BOOST_CHECK(  DummyDevice::calculateVirtualRegisterAddress( 0x67875, 0x3 ) == 0x3000000000067875UL );
  BOOST_CHECK(  DummyDevice::calculateVirtualRegisterAddress( 0, 0x4 ) == 0x4000000000000000UL );

  // the first bit of the bar has to be cropped
  BOOST_CHECK(  DummyDevice::calculateVirtualRegisterAddress( 0x123, 0xD ) == 0x5000000000000123UL );
}

#include <sstream>
#define BOOST_TEST_MODULE TMC429WordsTest
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "TMC429Words.h"
using namespace mtca4u;

/** This function is used to evaluate that the bit pattern given in
 *  ADD_VARIABLE is correct. It takes an empty word and sets all bits described in the setVARIABLE function.
 *  Afterwards it returns the whole data word, which can be compared to the correct bit pattern with only the
 *  bits according to the pattern are on.
 *  By using the bit numbers in the definition and the bit pattern in the evaluation the chance of 
 *  having the same error in these two independent implementations is minimised.
 * 
 *  As setVARIABLE and getVARIABLE are both generated from the same macro, which has been tested independently,
 *  just using the setter is sufficient.
 */
template<class T> unsigned int createDataWordUsingSetterFunction( void (T::* setterFunction )(unsigned int) ){
  T t;
  (t.*setterFunction)(0xFFFFFFFF);
  return t.getDataWord();
}

BOOST_AUTO_TEST_SUITE( TMC429WordsTestSuite )

BOOST_AUTO_TEST_CASE( testTMC429InputWord ){
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429InputWord> ( &TMC429InputWord::setRRS )     == 0x80000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429InputWord> ( &TMC429InputWord::setSMDA )    == 0x60000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429InputWord> ( &TMC429InputWord::setIDX_JDX ) == 0x1E000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429InputWord> ( &TMC429InputWord::setADDRESS ) == 0x7E000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429InputWord> ( &TMC429InputWord::setRW )      == 0x01000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429InputWord> ( &TMC429InputWord::setDATA )    == 0x00FFFFFF );
}

BOOST_AUTO_TEST_CASE( testTMC429OutputWord ){
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setSTATUS_BITS ) == 0xFF000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setINT )         == 0x80000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setCDGW )        == 0x40000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setSM3 )         == 0x30000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setRS3 )         == 0x20000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setxEQt3 )       == 0x10000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setSM2 )         == 0x0C000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setRS2 )         == 0x08000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setxEQt2 )       == 0x04000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setSM1 )         == 0x03000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setRS1 )         == 0x02000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setxEQt1 )       == 0x01000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setDATA )        == 0x00FFFFFF );
}

BOOST_AUTO_TEST_CASE( testReferenceSwitchData ){
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceSwitchData> ( &ReferenceSwitchData::setRight1 ) == 0x7C000001 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceSwitchData> ( &ReferenceSwitchData::setLeft1 )  == 0x7C000002 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceSwitchData> ( &ReferenceSwitchData::setRight2 ) == 0x7C000004 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceSwitchData> ( &ReferenceSwitchData::setLeft2 )  == 0x7C000008 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceSwitchData> ( &ReferenceSwitchData::setRight3 ) == 0x7C000010 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceSwitchData> ( &ReferenceSwitchData::setLeft3 )  == 0x7C000020 );
}

BOOST_AUTO_TEST_CASE( testCoverPositionAndLength ){
  BOOST_CHECK( createDataWordUsingSetterFunction<CoverPositionAndLength> ( &CoverPositionAndLength::setCoverLength )   == 0x6400001F );  
  BOOST_CHECK( createDataWordUsingSetterFunction<CoverPositionAndLength> ( &CoverPositionAndLength::setCoverPosition ) == 0x64003F00 );  
  BOOST_CHECK( createDataWordUsingSetterFunction<CoverPositionAndLength> ( &CoverPositionAndLength::setCoverWaiting )  == 0x64800000 );  
}

BOOST_AUTO_TEST_SUITE_END()

//class ExampleClassTestSuite : public test_suite {
//public:
//  ExampleClassTestSuite(): test_suite("ExampleClass test suite") {
//    add( BOOST_TEST_CASE( &testTMC429InputWord ) );
//    add( BOOST_TEST_CASE( &testTMC429OutputWord ) );
//    add( BOOST_TEST_CASE( &testReferenceSwitchData ) );
//    add( BOOST_TEST_CASE( &testCoverPositionAndLength ) );
//  }
//};
//
//// Although the compiler complains that argc and argv are not used they 
//// cannot be commented out. This somehow changes the signature and linking fails.
//test_suite*
//init_unit_test_suite( int argc, char* argv[] )
//{
//  framework::master_test_suite().p_name.value = "ExampleClass test suite";
//  return new ExampleClassTestSuite;
//}

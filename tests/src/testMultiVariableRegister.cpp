#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "MultiVariableRegister.h"

/* The test is also supposed to test the  ADD_VARIABLE makro,
 * so we have to define a couple of classes with different variable patters.
 */

/** Class with two variables, not connected and not at bytes 0 and 31.
 */
class TwoVariablesNotConnectedRegister : public MultiVariableRegister {
public:
   ADD_VARIABLE(First, 2, 15);
   ADD_VARIABLE(Second, 18, 28);
};

/** Class with two variables, not connected but including bytes 0 and 31.
 */
class TwoVariablesNotConnectedEdgesRegister : public MultiVariableRegister {
public:
   ADD_VARIABLE(First, 0, 13);
   ADD_VARIABLE(Second, 21, 31);
};

/** Class with three variables, contiguous, not bytes 0 and 31.
 */
class ThreeVariablesContiguousRegister : public MultiVariableRegister {
public:
  ADD_VARIABLE(First, 3, 13);
  ADD_VARIABLE(Second, 14, 17);
  ADD_VARIABLE(Third, 18, 30);
};

/** Class with three variables, contiguous, bytes 0 and 31.
 */
class ThreeVariablesContiguousEdgesRegister : public MultiVariableRegister {
public:
   ADD_VARIABLE(First, 0, 14);
   ADD_VARIABLE(Second, 15, 18);
   ADD_VARIABLE(Third, 19, 31);
};

/** Class with single and multi bit variables, contiguous, bytes 0 and 31.
 */
class SingleMultiBitContiguousRegister : public MultiVariableRegister {
public:
   ADD_VARIABLE(FirstBit, 0, 0);
   ADD_VARIABLE(SecondBit, 1, 1);
   ADD_VARIABLE(Alpha, 2, 17);
   ADD_VARIABLE(ThirdBit, 18, 18);
   ADD_VARIABLE(Beta, 19, 30);
   ADD_VARIABLE(FourthBit, 31, 31);
};


/** Class with single bit variables, not contiguous, bytes 0 and 31.
 */
class SingleBitNotConnectedRegister : public MultiVariableRegister {
public:
   ADD_VARIABLE(FirstBit, 0, 0);
   ADD_VARIABLE(SecondBit, 15, 15);
   ADD_VARIABLE(ThirdBit, 31, 31);
};


/** The unit tests for the MultiVariableRegister class, based on the 
 *  boost unit test library. We use a class which holds a private 
 *  instance of the MultiVariableRegister, which avoids code duplication.
 * 
 *  The tests should be run in the order stated here.
 *  Order dependencies are implemented in the teste suite.
 */
class MultiVariableRegisterTest
{
 public:
  MultiVariableRegisterTest();

  /** Testing the input mask is static because it does not require an instance of
   *  the DoubleVariableRegister.
   */
  static void testInputMask();

  /** Testing the output mask is static because it does not require an instance of
   *  the DoubleVariableRegister.
   */
  static void testOutputMask();

  void testGetSetDataWord();

  void testSetSubWord();
  
  void testGetSubWord();

 private:
  //  DoubleVariableRegister _doubleVariableRegister;
};

class MultiVariableRegisterTestSuite : public test_suite {
public:
  MultiVariableRegisterTestSuite()
    : test_suite(" MultiVariableRegister test suite") {
        // add member function test cases to a test suite
        boost::shared_ptr<MultiVariableRegisterTest> multiVariableRegister( new MultiVariableRegisterTest );

	test_case*  inputMaskTestCase = BOOST_TEST_CASE( &MultiVariableRegisterTest::testInputMask );
	test_case*  outputMaskTestCase = BOOST_TEST_CASE( &MultiVariableRegisterTest::testOutputMask );

        test_case* getSetTestCase = BOOST_CLASS_TEST_CASE( &MultiVariableRegisterTest::testGetSetDataWord, multiVariableRegister );
        test_case* setSubWordTestCase = BOOST_CLASS_TEST_CASE( &MultiVariableRegisterTest::testSetSubWord, multiVariableRegister );
        test_case* getSubWordTestCase = BOOST_CLASS_TEST_CASE( &MultiVariableRegisterTest::testGetSubWord, multiVariableRegister );

	setSubWordTestCase->depends_on( getSetTestCase );
	setSubWordTestCase->depends_on( inputMaskTestCase );

	getSubWordTestCase->depends_on( getSetTestCase );
	getSubWordTestCase->depends_on( outputMaskTestCase );

	add( inputMaskTestCase );
        add( outputMaskTestCase );

        add( getSetTestCase );
        add( setSubWordTestCase );
        add( getSubWordTestCase );
  }
private:
  
};

// Although the compiler complains that argc and argv are not used they 
// cannot be commented out. This somehow changes the signature and linking fails.
//test_suite*
//init_unit_test_suite( int argc, char* argv[] )
test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  framework::master_test_suite().p_name.value = "MultiVariableRegisterTest";

  return new MultiVariableRegisterTestSuite;
}

//test_suite*
//init_unit_test_suite( int argc, char* argv[] )
//{
//  std::cout << "hello, it's the old style" << std::endl;
//  return new MultiVariableRegisterTestSuite;
//}

// The implementations of the individual tests

void MultiVariableRegisterTest::testInputMask() {
  // we test some start values somewhere 'in the middle' plus 0 and 31 (edge values, 
  // special cases) plus 1 and 30 (to avoid off by one errors), in all combinations
  BOOST_CHECK( INPUT_MASK(4, 14)  == 0x7FF );
  BOOST_CHECK( INPUT_MASK(4, 15)  == 0xFFF );
  BOOST_CHECK( INPUT_MASK(4, 16)  == 0x1FFF );
  BOOST_CHECK( INPUT_MASK(5, 16)  == 0xFFF );

  BOOST_CHECK( INPUT_MASK(0, 23)  == 0xFFFFFF );
  BOOST_CHECK( INPUT_MASK(1, 23)  == 0x7FFFFF );

  BOOST_CHECK( INPUT_MASK(17, 30)  == 0x3FFF );
  BOOST_CHECK( INPUT_MASK(17, 31)  == 0x7FFF );
  
  BOOST_CHECK( INPUT_MASK(0, 31)  == 0xFFFFFFFF );
  BOOST_CHECK( INPUT_MASK(1, 31)  == 0x7FFFFFFF );
  BOOST_CHECK( INPUT_MASK(0, 30)  == 0x7FFFFFFF );
  BOOST_CHECK( INPUT_MASK(1, 30)  == 0x3FFFFFFF );
}

void MultiVariableRegisterTest::testOutputMask() {
  // Test the same cases as input mask
  BOOST_CHECK( OUTPUT_MASK(4, 14)  == 0x7FF0 );
  BOOST_CHECK( OUTPUT_MASK(4, 15)  == 0xFFF0 );
  BOOST_CHECK( OUTPUT_MASK(4, 16)  == 0x1FFF0 );
  BOOST_CHECK( OUTPUT_MASK(5, 16)  == 0x1FFE0 );

  BOOST_CHECK( OUTPUT_MASK(0, 23)  == 0xFFFFFF );
  BOOST_CHECK( OUTPUT_MASK(1, 23)  == 0xFFFFFE );

  BOOST_CHECK( OUTPUT_MASK(17, 30) == 0x7FFE0000 );
  BOOST_CHECK( OUTPUT_MASK(17, 31) == 0xFFFE0000 );

  BOOST_CHECK( OUTPUT_MASK(0, 31)  == 0xFFFFFFFF );
  BOOST_CHECK( OUTPUT_MASK(1, 31)  == 0xFFFFFFFE );
  BOOST_CHECK( OUTPUT_MASK(0, 30)  == 0x7FFFFFFF );
  BOOST_CHECK( OUTPUT_MASK(1, 30)  == 0x7FFFFFFE );
}

MultiVariableRegisterTest::MultiVariableRegisterTest() {
}

void MultiVariableRegisterTest::testGetSetDataWord() {
}

void MultiVariableRegisterTest::testSetSubWord() {
}

void MultiVariableRegisterTest::testGetSubWord() {
}

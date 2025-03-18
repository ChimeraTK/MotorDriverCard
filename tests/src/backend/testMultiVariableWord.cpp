// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <boost/test/included/unit_test.hpp>

#include <sstream>
using namespace boost::unit_test_framework;

#include "MultiVariableWord.h"

#include <ChimeraTK/Exception.h>

using namespace ChimeraTK;

/* The test is also supposed to test the  ADD_VARIABLE makro,
 * so we have to define a couple of classes with different variable patters.
 * We always use three variables and call them First, Second and Third so the
 * tests can be templated.
 */

/** Class with three variables, not connected and not at bytes 0 and 31.
 */
class ThreeVariablesNotConnectedWord : public MultiVariableWord {
 public:
  ADD_VARIABLE(First, 2, 15);
  ADD_VARIABLE(Second, 18, 23);
  ADD_VARIABLE(Third, 26, 28);
};

///** Class with three variables, not connected but including bytes 0 and 31.
// */
// class ThreeVariablesNotConnectedEdgesWord : public MultiVariableWord {
// public:
//  ADD_VARIABLE(First, 0, 13);
//  ADD_VARIABLE(Second, 15,20);
//  ADD_VARIABLE(Third, 21, 31);
//};

///** Class with three variables, contiguous, not bytes 0 and 31.
// */
// class ThreeVariablesContiguousWord : public MultiVariableWord {
// public:
//  ADD_VARIABLE(First, 3, 13);
//  ADD_VARIABLE(Second, 14, 17);
//  ADD_VARIABLE(Third, 18, 30);
//};

/** Class with three variables, contiguous, bytes 0 and 31.
 */
class ThreeVariablesContiguousEdgesWord : public MultiVariableWord {
 public:
  ADD_VARIABLE(First, 0, 14);
  ADD_VARIABLE(Second, 15, 18);
  ADD_VARIABLE(Third, 19, 31);
};

// not needed as we are testing with patterns as initial values for the data
// word
///** Class with single and multi bit variables, contiguous, bytes 0 and 31.
// */
// class SingleMultiBitContiguousWord : public MultiVariableWord {
// public:
//   ADD_VARIABLE(FirstBit, 0, 0);
//   ADD_VARIABLE(SecondBit, 1, 1);
//   ADD_VARIABLE(Alpha, 2, 17);
//   ADD_VARIABLE(ThirdBit, 18, 18);
//   ADD_VARIABLE(Beta, 19, 30);
//   ADD_VARIABLE(FourthBit, 31, 31);
//};

/** Class with single bit variables, not contiguous, bytes 0 and 31.
 */
class SingleBitNotConnectedWord : public MultiVariableWord {
 public:
  ADD_VARIABLE(First, 0, 0);
  ADD_VARIABLE(Second, 15, 15);
  ADD_VARIABLE(Third, 31, 31);
};

/** The unit tests for the MultiVariableWord class, based on the
 *  boost unit test library. We use a class which holds a private
 *  instance of the MultiVariableWord, which avoids code duplication.
 *
 *  The tests should be run in the order stated here.
 *  Order dependencies are implemented in the teste suite.
 */
template<class T>
class MultiVariableWordTest {
 public:
  MultiVariableWordTest();

  /** Testing the input mask is static because it does not require an instance
   * of the test words.
   */
  static void testInputMask();

  /** Testing the output mask is static because it does not require an instance
   * of the test words.
   */
  static void testOutputMask();

  static void testEqualsOperator();

  // FIXME: these could also be static
  void testGetSetDataWord();

  void testSetSubWord();

  void testGetSubWord();

 private:
  T _testWord;
  std::list<unsigned int> patterns;

  void testSetSubWordWithPattern(unsigned int initalDataWord, unsigned int first, unsigned int second,
      unsigned int third, unsigned int expectedDataWord);

  void testGetSubWordWithPattern(
      unsigned int dataWord, unsigned int expectedFirst, unsigned int expectedSecond, unsigned int expectedThird);
};

template<class T>
class MultiVariableWordTestSuite : public test_suite {
 public:
  MultiVariableWordTestSuite(std::string testSuiteName) : test_suite(testSuiteName) {
    // add member function test cases to a test suite
    boost::shared_ptr<MultiVariableWordTest<T>> multiVariableWord(new MultiVariableWordTest<T>);

    test_case* inputMaskTestCase = BOOST_TEST_CASE(&MultiVariableWordTest<T>::testInputMask);
    test_case* outputMaskTestCase = BOOST_TEST_CASE(&MultiVariableWordTest<T>::testOutputMask);
    test_case* equalsOperatorTestCase = BOOST_TEST_CASE(&MultiVariableWordTest<T>::testEqualsOperator);

    test_case* getSetTestCase = BOOST_CLASS_TEST_CASE(&MultiVariableWordTest<T>::testGetSetDataWord, multiVariableWord);
    test_case* setSubWordTestCase = BOOST_CLASS_TEST_CASE(&MultiVariableWordTest<T>::testSetSubWord, multiVariableWord);
    test_case* getSubWordTestCase = BOOST_CLASS_TEST_CASE(&MultiVariableWordTest<T>::testGetSubWord, multiVariableWord);

    setSubWordTestCase->depends_on(getSetTestCase);
    setSubWordTestCase->depends_on(inputMaskTestCase);

    getSubWordTestCase->depends_on(getSetTestCase);
    getSubWordTestCase->depends_on(outputMaskTestCase);

    add(inputMaskTestCase);
    add(outputMaskTestCase);
    add(equalsOperatorTestCase);

    add(getSetTestCase);
    add(setSubWordTestCase);
    add(getSubWordTestCase);
  }
};

test_suite* init_unit_test_suite(int /*argc*/, char* /*argv*/[]) {
  framework::master_test_suite().p_name.value = "MultiVariableWordTest";

  framework::master_test_suite().add(
      new MultiVariableWordTestSuite<ThreeVariablesNotConnectedWord>("ThreeVariablesNotConnectedWord test suite"));
  framework::master_test_suite().add(new MultiVariableWordTestSuite<ThreeVariablesContiguousEdgesWord>(
      "ThreeVariablesContiguousEdgesWord test suite"));
  framework::master_test_suite().add(
      new MultiVariableWordTestSuite<SingleBitNotConnectedWord>("SingleBitNotConnectedWord test suite"));

  return 0;
}

// The implementations of the individual tests

template<class T>
void MultiVariableWordTest<T>::testInputMask() {
  // we test some start values somewhere 'in the middle' plus 0 and 31 (edge
  // values, special cases) plus 1 and 30 (to avoid off by one errors), in all
  // combinations
  BOOST_CHECK(INPUT_MASK(4, 14) == 0x7FF);
  BOOST_CHECK(INPUT_MASK(4, 15) == 0xFFF);
  BOOST_CHECK(INPUT_MASK(4, 16) == 0x1FFF);
  BOOST_CHECK(INPUT_MASK(5, 16) == 0xFFF);

  BOOST_CHECK(INPUT_MASK(0, 23) == 0xFFFFFF);
  BOOST_CHECK(INPUT_MASK(1, 23) == 0x7FFFFF);

  BOOST_CHECK(INPUT_MASK(17, 30) == 0x3FFF);
  BOOST_CHECK(INPUT_MASK(17, 31) == 0x7FFF);

  BOOST_CHECK(INPUT_MASK(0, 31) == 0xFFFFFFFF);
  BOOST_CHECK(INPUT_MASK(1, 31) == 0x7FFFFFFF);
  BOOST_CHECK(INPUT_MASK(0, 30) == 0x7FFFFFFF);
  BOOST_CHECK(INPUT_MASK(1, 30) == 0x3FFFFFFF);
}

template<class T>
void MultiVariableWordTest<T>::testOutputMask() {
  // Test the same cases as input mask
  BOOST_CHECK(OUTPUT_MASK(4, 14) == 0x7FF0);
  BOOST_CHECK(OUTPUT_MASK(4, 15) == 0xFFF0);
  BOOST_CHECK(OUTPUT_MASK(4, 16) == 0x1FFF0);
  BOOST_CHECK(OUTPUT_MASK(5, 16) == 0x1FFE0);

  BOOST_CHECK(OUTPUT_MASK(0, 23) == 0xFFFFFF);
  BOOST_CHECK(OUTPUT_MASK(1, 23) == 0xFFFFFE);

  BOOST_CHECK(OUTPUT_MASK(17, 30) == 0x7FFE0000);
  BOOST_CHECK(OUTPUT_MASK(17, 31) == 0xFFFE0000);

  BOOST_CHECK(OUTPUT_MASK(0, 31) == 0xFFFFFFFF);
  BOOST_CHECK(OUTPUT_MASK(1, 31) == 0xFFFFFFFE);
  BOOST_CHECK(OUTPUT_MASK(0, 30) == 0x7FFFFFFF);
  BOOST_CHECK(OUTPUT_MASK(1, 30) == 0x7FFFFFFE);
}

template<class T>
void MultiVariableWordTest<T>::testEqualsOperator() {
  T firstWord;
  firstWord.setDataWord(25);
  T secondWord;
  secondWord.setDataWord(25);
  T thirdWord;
  thirdWord.setDataWord(26);

  BOOST_CHECK((firstWord == secondWord) == true);
  BOOST_CHECK((firstWord == thirdWord) == false);
}

template<class T>
MultiVariableWordTest<T>::MultiVariableWordTest() : _testWord(), patterns() {}

template<class T>
void MultiVariableWordTest<T>::testGetSetDataWord() {
  // this test has to be the first one, so we can check on the initial value 0
  // as a first step.
  BOOST_CHECK(_testWord.getDataWord() == 0);

  // test different changing bit patterns
  // bit pattern 1111...
  _testWord.setDataWord(0xFFFFFFFF);
  BOOST_CHECK(_testWord.getDataWord() == 0xFFFFFFFF);

  // bit patter 0101....
  _testWord.setDataWord(0x55555555);
  BOOST_CHECK(_testWord.getDataWord() == 0x55555555);

  // bit pattern 1010...
  _testWord.setDataWord(0xAAAAAAAA);
  BOOST_CHECK(_testWord.getDataWord() == 0xAAAAAAAA);

  // bit pattern 0000...
  _testWord.setDataWord(0x0);
  BOOST_CHECK(_testWord.getDataWord() == 0x0);
}

// the test are always the same:
// initial pattern 0, first = 0, second= ..0101 , third = 111..
// initial pattern FFF.., first = 111..., second= ..1010 , third = 0
// initial pattern 0101..., first = ...1010, second = 0, third = ...0101
// initial pattern 1010..., first = ...0101, second = 1111..., third = ...1010
// they have to be hand-coded for each class implementation
// for the last two tests we put in the full 32 bit pattern (too long for the
// variable content) to test the cropping (only the bits belonging to the
// variable must be overwritten)

template<>
void MultiVariableWordTest<ThreeVariablesNotConnectedWord>::testSetSubWord() {
  BOOST_CHECK_THROW(_testWord.setSecond(0x40), ChimeraTK::logic_error);
  BOOST_CHECK_NO_THROW(_testWord.setSecond(0x3F));
  testSetSubWordWithPattern(0x00000000, 0, 0x15, 0x7, 0x1C540000);
  testSetSubWordWithPattern(0xFFFFFFFF, 0x3FFF, 0x2A, 0, 0xE3ABFFFF);
  testSetSubWordWithPattern(0x55555555, 0x2AAA, 0, 0x5, 0x5501AAA9);
  testSetSubWordWithPattern(0xAAAAAAAA, 0x1555, 0x3F, 0x2, 0xAAFE5556);
}

template<>
void MultiVariableWordTest<ThreeVariablesContiguousEdgesWord>::testSetSubWord() {
  BOOST_CHECK_THROW(_testWord.setSecond(0x10), ChimeraTK::logic_error);
  BOOST_CHECK_NO_THROW(_testWord.setSecond(0xF));
  testSetSubWordWithPattern(0x00000000, 0, 0x5, 0x1FFF, 0xFFFA8000);
  testSetSubWordWithPattern(0xFFFFFFFF, 0x7FFF, 0xA, 0, 0x00057FFF);
  testSetSubWordWithPattern(0x55555555, 0x2AAA, 0, 0x1555, 0xAAA82AAA);
  testSetSubWordWithPattern(0xAAAAAAAA, 0x5555, 0xF, 0x0AAA, 0x5557D555);
}

template<>
void MultiVariableWordTest<SingleBitNotConnectedWord>::testSetSubWord() {
  BOOST_CHECK_THROW(_testWord.setSecond(0x2), ChimeraTK::logic_error);
  BOOST_CHECK_NO_THROW(_testWord.setSecond(0x1));
  testSetSubWordWithPattern(0x00000000, 0, 0x1, 0x1, 0x80008000);
  testSetSubWordWithPattern(0xFFFFFFFF, 0x1, 0x0, 0x0, 0x7FFF7FFF);
  testSetSubWordWithPattern(0x55555555, 0x0, 0, 0x1, 0xD5555554);
  testSetSubWordWithPattern(0xAAAAAAAA, 0x1, 0x1, 0x0, 0x2AAAAAAB);
}

// we start from the initial patterns and extraxt the sub words
template<>
void MultiVariableWordTest<ThreeVariablesNotConnectedWord>::testGetSubWord() {
  testGetSubWordWithPattern(0, 0, 0, 0);
  testGetSubWordWithPattern(0xFFFFFFFF, 0x3FFF, 0x3F, 0x7);
  testGetSubWordWithPattern(0x55555555, 0x1555, 0x15, 0x5);
  testGetSubWordWithPattern(0xAAAAAAAA, 0x2AAA, 0x2A, 0x2);
}

template<>
void MultiVariableWordTest<ThreeVariablesContiguousEdgesWord>::testGetSubWord() {
  testGetSubWordWithPattern(0, 0, 0, 0);
  testGetSubWordWithPattern(0xFFFFFFFF, 0x7FFF, 0xF, 0x1FFF);
  testGetSubWordWithPattern(0x55555555, 0x5555, 0xA, 0xAAA);
  testGetSubWordWithPattern(0xAAAAAAAA, 0x2AAA, 0x5, 0x1555);
}

template<>
void MultiVariableWordTest<SingleBitNotConnectedWord>::testGetSubWord() {
  testGetSubWordWithPattern(0, 0, 0, 0);
  testGetSubWordWithPattern(0xFFFFFFFF, 0x1, 0x1, 0x1);
  testGetSubWordWithPattern(0x55555555, 0x1, 0x0, 0x0);
  testGetSubWordWithPattern(0xAAAAAAAA, 0x0, 0x1, 0x1);
}

template<class T>
void MultiVariableWordTest<T>::testSetSubWordWithPattern(unsigned int initalDataWord, unsigned int first,
    unsigned int second, unsigned int third, unsigned int expectedDataWord) {
  _testWord.setDataWord(initalDataWord);
  _testWord.setFirst(first);
  _testWord.setSecond(second);
  _testWord.setThird(third);

  std::stringstream errorMessage;
  errorMessage << std::hex << "0x" << _testWord.getDataWord() << " != " << expectedDataWord;
  BOOST_CHECK_MESSAGE(_testWord.getDataWord() == expectedDataWord, errorMessage.str());
}

template<class T>
void MultiVariableWordTest<T>::testGetSubWordWithPattern(
    unsigned int dataWord, unsigned int expectedFirst, unsigned int expectedSecond, unsigned int expectedThird) {
  _testWord.setDataWord(dataWord);

  std::stringstream errorMessage;
  errorMessage << std::hex << "test pattern 0x" << dataWord;
  BOOST_CHECK_MESSAGE(_testWord.getFirst() == expectedFirst, errorMessage.str());
  BOOST_CHECK_MESSAGE(_testWord.getSecond() == expectedSecond, errorMessage.str());
  BOOST_CHECK_MESSAGE(_testWord.getThird() == expectedThird, errorMessage.str());
}

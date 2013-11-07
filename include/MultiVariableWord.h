#ifndef MTAC4U_MULTI_VARIABLE_WORD_H
#define MTAC4U_MULTI_VARIABLE_WORD_H

/** Generate an input mask from the position of the first and the last bit 
 *  of the mask. The input mask is the mask applied to an input word which is 
 *  going to be written. As these are always the last bits of the input,
 *  the mask is only dependent on the width of the sub word.<br> Example:
 *  bits 12 to 15 describe a 4 bit wide sub word with an input mask of \c 0x0000000F.
 *  This macro is mainly for internal usage in the ADD_VARIABLE() macro.
 *  @attention There is no range check for the bits. The valid bit range is [0:31]
 *  for both first and last bit.
 */
// The bit shift is performed on a 64 bit unsigned int (UL) to avoid
// buffer overflow for FIRST_BIT=0 and LAST_BIT=31.
// The result is cast back to 32 bit.
#define INPUT_MASK(FIRST_BIT, LAST_BIT)\
  static_cast<unsigned int>( (1UL<< (LAST_BIT-FIRST_BIT+1)) -1 )

/** Generate an outpu mask from the position of the first and the last bit 
 *  of the mask. The output mask is the mask applied to the combined word.
 *  <br> Example:
 *  bits 12 to 15 describe a 4 bit wide sub word with an output mask of \c 0x0000F000.
 *  This macro is mainly for internal usage in the ADD_VARIABLE() macro.
 *  @attention There is no range check for the bits. The valid bit range is [0:31]
 *  for both first and last bit.
 */
#define OUTPUT_MASK(FIRST_BIT, LAST_BIT)\
  static_cast<unsigned int>( ( (1UL<< (LAST_BIT-FIRST_BIT+1)) -1) << FIRST_BIT )

/** \def ADD_VARIABLE(VAR_NAME,FIRST_BIT,LAST_BIT)
 *  The ADD_VARIABLE macro allows to add a getter and a setter function for a
 *  variable with the correct input and output masks. The user specifies
 *  the variable name and the first and last bit of the sub word.
 *
 *  \verbatim
ADD_VARIABLE( Voltage, 12, 15 )
    \endverbatim
 *  expands to
 *  \verbatim
void setVoltage(unsigned int);
unsigned int getVoltage(); 
     \endverbatim
 *  including the implementation for a 4 bit wide sub word
 *  (12 through 15, both bits included, mask 0xF000)
 *
 */
#define ADD_VARIABLE(VAR_NAME, FIRST_BIT, LAST_BIT)\
  unsigned int get ## VAR_NAME () const{ \
    return getSubWord( OUTPUT_MASK(FIRST_BIT, LAST_BIT), FIRST_BIT ); } \
  void set ## VAR_NAME (unsigned int word){ \
    return setSubWord( word, INPUT_MASK(FIRST_BIT, LAST_BIT), FIRST_BIT ); }
  
namespace mtca4u{

/** The MultiVariableWord encodes and decodes multiple short
 *  words into one unsigned int.
 *
 *  Example: A 32 bit word is interpreted as two values.
 *  Bits 0 to 10 are the first value, bits 13 to 23 are the second
 *  value. All other bits are empty/unused.
 *
 *  This class is used as a base class. The derived class can conveniently be populated
 *  with the ADD_VARIABLE() macro.<br>
 *  Example for a 32 bit treshold register which holds an upper
 *  and a lower threshold value.
 */
// doxygen description in several blocks to work around a bug in doxygen.
/**
\verbatim
class TwoThresholdsWord: public MultiVariableWord {
    ADD_VARIABLE(LowerThreshold, 0, 10);
    ADD_VARIABLE(UpperThreshold, 13, 23);
};
\endverbatim
 
 * This defines the methods
\verbatim
void setLowerThreshold(unsigned int);
unsigned int getLowerThreshold();

void setUpperThreshold(unsigned int);
unsigned int getUpperThreshold();
\endverbatim

 * and implements them, calling setSubWord() and getSubWord() with the correct masks
 * and offsets as specified by the bit ranges. All masks and offsets are hard coded
 * at compile time for maximum performance. They are not needed in the further code,
 * which only uses the generated getter and setter functions.
 */
class MultiVariableWord{
 public:
  /** The constructor allows to set the data word on construction.
   */
  MultiVariableWord(unsigned int dataWord = 0);

  /** Get the encoded 32 bit data word. */
  unsigned int getDataWord() const;

  /** Set the whole 32 bit data word. */
  void setDataWord(unsigned int dataWord);

  bool operator==(MultiVariableWord const & right);

 protected:
  /** Get the sub word from the data word as specified by the output mask and offset.
   *  The output mask is the mask at the position of the data word
   *  in the 32 bit word.
   *
   *  N.B. The offset information is redundant and could be extracted from the mask.
   *  For performance reasons, and as this code usually is only called from
   *  macro-generated code which avoids inconsistencies, this additional information
   *  is used to avoid unnecessary computations.
   */
  unsigned int getSubWord(unsigned int outputMask,
			  unsigned char offset) const;
  
  /** Set the sub word at the width and position defined by input mask and offset.
   *  The input mask is the mask at the end of the data word
   *  with the length of the sub word.
   *
   *  @attention This function trunkates the incoming sub word to the length
   *  as specified by the mask. There is no warning or exception if this is done.
   *  The user has to perform this test beforehand, if required.
   *
   *  N.B. The offset information is redundant and could be extracted from the mask.
   *  For performance reasons, and as this code usually is only called from
   *  macro-generated code which avoids inconsistencies, this additional information
   *  is used to avoid unnecessary computations.
   */
  void setSubWord(unsigned int subWord, unsigned int inputMask,
		  unsigned char offset);

  
 private:
  unsigned int _dataWord;
};


}// namespace mtca4u
#endif // MTAC4U_MULTI_VARIABLE_WORD_H

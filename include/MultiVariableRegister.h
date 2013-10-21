#ifndef MTAC4U_MULTI_VARIABLE_REGISTER_H
#define MTAC4U_MULTI_VARIABLE_REGISTER_H

/** The MultiVariableRegister encodes and decodes multiple short
 *  words into one unsigned int.
 *
 *  Examples: A 32 bit word is interpreted as two values. 
 *  bits 0 to 10 are the first value, bits 13 to 18 are the second
 *  value. All other bits are empty.
 *
 *  @attention This class does not perform range checks on the bit values for
 *  performance reasons. 
 */
class MultiVariableRegister;

#define INPUT_MASK(FIRST_BIT, LAST_BIT)\
  (LAST_BIT==31 && FIRST_BIT==0 ? \
   0xFFFFFFFF : ( (1U<< (LAST_BIT-FIRST_BIT+1)) -1 ) )

#define OUTPUT_MASK(FIRST_BIT, LAST_BIT)\
  (LAST_BIT==31 && FIRST_BIT==0 ? \
   0xFFFFFFFF : ( (1U<< (LAST_BIT-FIRST_BIT+1)) -1) << FIRST_BIT )

#define ADD_VARIABLE(VAR_NAME, FIRST_BIT, LAST_BIT)\
  unsigned int get ## VAR_NAME (){ \
    return getSubWord( OUTPUT_MASK(FIRST_BIT, LAST_BIT), FIRST_BIT ); } \
  void set ## VAR_NAME (unsigned int word){ \
    return setSubWord( word, INPUT_MASK(FIRST_BIT, LAST_BIT), FIRST_BIT ); }
  
class MultiVariableRegister{
 public:
  /** Get the encoded 32 bit data word. */
  unsigned int getDataWord();

  /** Set the whole 32 bit data word. */
  void setDataWord(unsigned int dataWord);

  MultiVariableRegister(unsigned int dataWord = 0);

 protected:
  /** The output mask is the mask at the position of the data word
   *  in the 23 bit word.
   */
  unsigned int getSubWord(unsigned int outputMask,
			  unsigned char offset);
  
  /** The input mask is the mask at the end of the data word
   *  with the length of the sub word.
   */
  void setSubWord(unsigned int subWord, unsigned int inputMask,
		  unsigned char offset);

  
 private:
  unsigned int _dataWord;
};

#endif // MTAC4U_MULTI_VARIABLE_REGISTER_H

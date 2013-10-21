#include "MultiVariableRegister.h"

MultiVariableRegister::MultiVariableRegister(unsigned int dataWord)
  : _dataWord(dataWord)
{}

unsigned int MultiVariableRegister::getDataWord()
{
  return _dataWord;
}

void MultiVariableRegister::setDataWord(unsigned int dataWord)
{
  _dataWord = dataWord;
}

unsigned int MultiVariableRegister::getSubWord(unsigned int outputMask,
					       unsigned char offset)
{
  return (_dataWord & outputMask) >> offset;
}

void MultiVariableRegister::setSubWord(unsigned int subWord,
				       unsigned int inputMask,
				       unsigned char offset)
{
  _dataWord = (_dataWord & ~(inputMask << offset)) | ((subWord & inputMask) << offset);
}

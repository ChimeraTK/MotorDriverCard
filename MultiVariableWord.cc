#include "MultiVariableWord.h"

namespace mtca4u{

MultiVariableWord::MultiVariableWord(unsigned int dataWord)
  : _dataWord(dataWord)
{}

unsigned int MultiVariableWord::getDataWord() const
{
  return _dataWord;
}

void MultiVariableWord::setDataWord(unsigned int dataWord)
{
  _dataWord = dataWord;
}

unsigned int MultiVariableWord::getSubWord(unsigned int outputMask,
					       unsigned char offset) const
{
  return (_dataWord & outputMask) >> offset;
}

void MultiVariableWord::setSubWord(unsigned int subWord,
				       unsigned int inputMask,
				       unsigned char offset)
{
  _dataWord = (_dataWord & ~(inputMask << offset)) | ((subWord & inputMask) << offset);
}

}// namespace mtca4u

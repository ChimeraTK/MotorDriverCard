#include "MultiVariableWord.h"

#include "ChimeraTK/Exception.h"

#include <sstream>

namespace mtca4u {

  MultiVariableWord::MultiVariableWord(unsigned int dataWord) : _dataWord(dataWord) {}

  MultiVariableWord::~MultiVariableWord() {}

  unsigned int MultiVariableWord::getDataWord() const { return _dataWord; }

  void MultiVariableWord::setDataWord(unsigned int dataWord) { _dataWord = dataWord; }

  bool MultiVariableWord::operator==(MultiVariableWord const& right) const { return _dataWord == right._dataWord; }

  bool MultiVariableWord::operator!=(MultiVariableWord const& right) const { return _dataWord != right._dataWord; }

  unsigned int MultiVariableWord::getSubWord(unsigned int outputMask, unsigned char offset) const {
    return (_dataWord & outputMask) >> offset;
  }

  void MultiVariableWord::setSubWord(unsigned int subWord, unsigned int inputMask, unsigned char offset) {
    if(subWord & ~inputMask) {
      std::stringstream message;
      message << "Sub word is too large. Input mask is 0x" << std::hex << inputMask << ", sub word is 0x" << subWord
              << "=" << std::dec << subWord;
      throw ChimeraTK::logic_error(message.str());
    }
    _dataWord = (_dataWord & ~(inputMask << offset)) | ((subWord & inputMask) << offset);
  }

} // namespace mtca4u

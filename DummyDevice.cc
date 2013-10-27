#include <algorithm>

#include "DummyDevice.h"
#include "NotImplementedException.h"
#include <MtcaMappedDevice/mapFileParser.h>

// Valid bar numbers are 0 to 5 , so they must be contained
// in three bits.
#define BAR_MASK 0x7
// the bar number is stored in bits 60 to 62
#define BAR_POSITION_IN_VIRTUAL_REGISTER 60

namespace mtca4u{

  DummyDevice::FakeRegister::FakeRegister(size_t sizeInWords_) : _dataContent(sizeInWords_, 0),
								_isReadOnly(false)
  {}

  size_t DummyDevice::FakeRegister::sizeInWords(){
    return _dataContent.size();
  }

  void DummyDevice::FakeRegister::setReadOnly(bool isReadOnly_){
    _isReadOnly = isReadOnly_;
  }

  bool  DummyDevice::FakeRegister::isReadOnly(){
    return _isReadOnly;
  }

  int32_t DummyDevice::FakeRegister::read(size_t offset){
    // we intentionally use at() and not [] to have a range check
    return _dataContent.at(offset);
  }

  void DummyDevice::FakeRegister::write(int32_t dataWord, size_t offset){
    if (_isReadOnly){
      return;
    }
    
    // we intentionally use at() and not [] to have a range check
    _dataContent.at(offset) = dataWord;

    // call the writeCallbackFunction, if it has been set
    if (_writeCallbackFunction){
      _writeCallbackFunction();
    }
  }

  void DummyDevice::FakeRegister::setWriteCallbackFunction(
	    boost::function<void(void)> writeCallbackFunction ){
    _writeCallbackFunction =  writeCallbackFunction;
  }

  //Noting to do, intentionally empty.
  DummyDevice::DummyDevice(){
  }

  //Nothing to clean up, all objects clean up for themselves when
  //they go out of scope.
  DummyDevice::~DummyDevice(){
  }

  void DummyDevice::openDev(const std::string &mappingFileName,
			    int /* perm */, devConfigBase* /* pConfig */){
    _registerMapping = mapFileParser().parse(mappingFileName);
    resizeBarContents();
    opened=true;
  }

  void DummyDevice::resizeBarContents(){
    std::map< uint8_t, size_t > barSizes = getBarSizesInBytesFromRegisterMapping();
    for (std::map< uint8_t, size_t >::const_iterator barSizeIter = barSizes.begin();
	 barSizeIter != barSizes.end(); ++barSizeIter){
      _barContents[barSizeIter->first].resize(barSizeIter->second / sizeof(int32_t));
    }
  }

  std::map< uint8_t, size_t > DummyDevice::getBarSizesInBytesFromRegisterMapping(){
    std::map< uint8_t, size_t > barSizes;
    for (mapFile::const_iterator mappingElementIter = _registerMapping->begin();
	 mappingElementIter <  _registerMapping->end(); ++mappingElementIter ) {
      barSizes[mappingElementIter->reg_bar] = 
	std::max( barSizes[mappingElementIter->reg_bar],
		  static_cast<size_t> (mappingElementIter->reg_address +
				       mappingElementIter->reg_size ) );
    }
    return barSizes;
  }

  void DummyDevice::closeDev(){
    _registerMapping = ptrmapFile(0);
    _barContents.clear();
    opened=false;
  }

  void DummyDevice::readReg(uint32_t regOffset, int32_t* data, uint8_t bar){
    throw NotImplementedException("DummyDevice::readReg is not implemented yet.");
  }

  void DummyDevice::writeReg(uint32_t regOffset, int32_t data, uint8_t bar){
    throw NotImplementedException("DummyDevice::writeReg is not implemented yet.");
  }

  void DummyDevice::readArea(uint32_t regOffset, int32_t* data, size_t size,
			     uint8_t bar){
    throw NotImplementedException("DummyDevice::readArea is not implemented yet.");
  }

  void DummyDevice::writeArea(uint32_t regOffset, int32_t* data, size_t size,
			      uint8_t bar){
    throw NotImplementedException("DummyDevice::writeArea is not implemented yet.");
  }

  void DummyDevice::readDMA(uint32_t regOffset, int32_t* data, size_t size,
			    uint8_t bar){
    throw NotImplementedException("DummyDevice::readDMA is not implemented yet.");
  }

  void DummyDevice::writeDMA(uint32_t regOffset, int32_t* data, size_t size,
			     uint8_t bar){
    throw NotImplementedException("DummyDevice::writeDMA is not implemented yet.");
  }

  void DummyDevice::readDeviceInfo(std::string* devInfo){
    throw NotImplementedException("DummyDevice::readDeviceInfo is not implemented yet.");
  }

  uint64_t DummyDevice::calculateVirtualRegisterAddress( uint32_t registerOffsetInBar,
							 uint8_t bar){
    return (static_cast<uint64_t>(bar & BAR_MASK) << BAR_POSITION_IN_VIRTUAL_REGISTER) | 
           (static_cast<uint64_t> (registerOffsetInBar));
  }
  
}// namespace mtca4u

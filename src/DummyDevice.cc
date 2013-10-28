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

//  void DummyDevice::FakeRegisterContainer::populate(ptrmapFile const & registerMapping){
//    for (mapFile::const_iterator mappingElementIter = registerMapping->begin();
//	 mappingElementIter <  registerMapping->end(); ++mappingElementIter ) {
//      
//      // check if there is a register which already contains the address range of this mapping element
//      std::map< uint64_t, FakeRegister >::iterator possibleTargetRegisterIter = 
//	findRegisterWithAddress(  mappingElementIter->reg_address );
//
//      if ( possibleTargetRegisterIter ==  _registers.end() ) {
//	// no target register, add one
//	_registers[mappingElementIter->reg_address] = 
//	  FakeRegister( mappingElementIter->reg_size/sizeof(int32_t) );
//      }
//      else {
//	extendTargetRegisterIfNecessary( possibleTargetRegisterIter,  *mappingElementIter );
//      }
//    }// for mappingElementIter 
//  }
//
//  DummyDevice::FakeRegisterContainer::iterator DummyDevice::FakeRegisterContainer::findRegisterWithAddress(
//										 uint64_t virtualAddress){
//    if (_registers.empty()){
//      return _registers.end();
//    }
//
//    iterator iteratorAfterPossibleTargetRegister = _registers.upper_bound(virtualAddress);
//    if (iteratorAfterPossibleTargetRegister == _registers.begin()){
//      // no target register. The positon after is begin(), so there is nothing valid before
//      return _registers.end(); // end() is not found
//    }
//
//    // Now we are allowed to go back one iterator and know it is still valid (not empty and at before begin())
//    iterator possibleTargetRegisterIter = iteratorAfterPossibleTargetRegister; // first we have  to copy
//    --possibleTargetRegisterIter; // then we have to do the step back (there is no -n operator)
//
//    // We now know that the target register has a base address which is smaller or equal to the virtual address
//    // Let's check if it is long 
//    uint64_t firstAddressAfterPossibleTargetRegister = getFirstAddressAfterRegister( targetRegisterIter );
//
//    if (virtualAddress >=  firstAddressAfterPossibleTargetRegister){
//      return _registers.end();
//    }
//
//    return possibleTargetRegisterIter;
//  }
//
//  void DummyDevice::FakeRegisterContainer::extendTargetRegisterIfNecessary( iterator & targetRegisterIter,
//         mapFile::mapElem const & mappingElement ){
//    uint64_t firstAddressAfterTargetRegister = getFirstAddressAfterRegister( targetRegisterIter );
//    uint64_t firstAddressAfterMappingElement = mappingElement.reg_address +  mappingElement.reg_size();
//
//    if (firstAddressAfterMappingElement <= firstAddressAfterTargetRegister){
//      // mapping element is completely contained in target register, nothing to do
//      return;
//    }
//
//    size_t numberOfWordsNeeded = 
//      (firstAddressAfterMappingElement - possibleTargetRegisterIter->first) / sizeof(int32_t);
//
//    possibleTargetRegisterIter->second.assureMinimumSize(numberOfWordsNeeded);
//    firstAddressAfterTargetRegister = firstAddressAfterMappingElement;
//
//    //Check that the now larger register does not overlap with the next entries.
//    //Merge them if necessary.
//    iterator nextRegisterIter = targetRegisterIter;
//    ++nextRegisterIter;
//    for (; (nextRegisterIter != _registers.end()) && (firstAddressAfterTargetRegister < nextRegisterIter->first);){
//      mergeSecondIntoFirstRegister( targetRegisterIter, nextRegisterIter);
//      _registers.erase( nextRegisterIter++ );
//    }
//  }
//
//  void DummyDevice::FakeRegisterContainer::mergeSecondIntoFirstRegister(
//	    DummyDevice::FakeRegisterContainer::iterator firstRegisterIter,
//	    DummyDevice::FakeRegisterContainer::iterator secondRegisterIter){
//    uint64_t firstAddressAfterSecondRegister = getFirstAddressAfterRegister( secondRegisterIter );
//
//    size_t numberOfWordsNeeded = calculateNumberOfWordsFromAddressRange( firstRegisterIter.first,
//									 firstAddressAfterSecondRegister );
// 
//
//    if (firstAddressAfterFirstRegister < firstAddressAfterSecondRegister)
//    {
//      //the first register has to be extended
//      
//    }
//  }

//  size_t calculateNumberOfWordsFromAddressRange( uint64_t startAddress, uint64_t stopAddress ){
//    return (stopAddress - startAddress) / sizeof(int32_t);
//  }
  
//  uint64_t DummyDevice::FakeRegisterContainer::getFirstAddressAfterRegister( 
//		DummyDevice::FakeRegisterContainer::iterator registerIter){
//    return possibleTargetRegisterIter->first + //the base address
//      possibleTargetRegisterIter->second.getSizeInWords() * sizeof(int32_t); // size of the fake register in bytes;
//  }

//  void DummyDevice::addOrResizeRegister( mapFile::mapElem const & mappingElement ){
//    checkSizeIsMultipleOfFour( mappingElement.reg_size );
//    uint64_t targetAddress = calculateVirtualRegisterAddress( mappingElement.reg_address,
//							      mappingElement.reg_bar );
//
//    _registers[targetAddress].ensureMinimumSize( mappingElement.reg_size / sizeof(int32_t) );
//  }

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
    std::map< uint8_t, size_t > barSizesInBytes 
      = getBarSizesInBytesFromRegisterMapping();

    for (std::map< uint8_t, size_t >::const_iterator barSizeInBytesIter 
	   = barSizesInBytes.begin();
	 barSizeInBytesIter != barSizesInBytes.end(); ++barSizeInBytesIter){
      
      //the size of the vector is in words, not in bytes -> convert fist
      _barContents[barSizeInBytesIter->first].resize(
	barSizeInBytesIter->second / sizeof(int32_t));
    }
  }

  std::map< uint8_t, size_t > DummyDevice::getBarSizesInBytesFromRegisterMapping() const{
    std::map< uint8_t, size_t > barSizesInBytes;
    for (mapFile::const_iterator mappingElementIter = _registerMapping->begin();
	 mappingElementIter <  _registerMapping->end(); ++mappingElementIter ) {
      barSizesInBytes[mappingElementIter->reg_bar] = 
	std::max( barSizesInBytes[mappingElementIter->reg_bar],
		  static_cast<size_t> (mappingElementIter->reg_address +
				       mappingElementIter->reg_size ) );
    }
    return barSizesInBytes;
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

  void DummyDevice::checkSizeIsMultipleOfWordSize(size_t size){
    if (size % sizeof(int32_t) ){	
      throw( DummyDeviceException("Read/write size has to be a multiple of 4", DummyDeviceException::WRONG_SIZE) );
    }
  }
  
}// namespace mtca4u

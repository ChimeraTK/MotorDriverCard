#ifndef MTCA4U_DUMMY_DEVICE_H
#define MTCA4U_DUMMY_DEVICE_H

#include <vector>
#include <map>
#include <list>
#include <set>

#include <boost/function.hpp>

#include <MtcaMappedDevice/devBase.h>
#include <MtcaMappedDevice/exBase.h>
#include <MtcaMappedDevice/mapFile.h>

namespace mtca4u{
  
  ///fixme: there should only be one type of exception for all devices. Otherwise you will never be able to
  /// interpret the enum in an exception from a pointer to exBase.
  class DummyDeviceException : public exBase {
  public:
    enum {WRONG_SIZE};
    DummyDeviceException(const std::string &message, unsigned int exceptionID)
      : exBase( message, exceptionID ){}
  };


  /** The dummy device opens a mapping file instead of a device, and
   *  implements all registers defined in the mapping file im memory.
   *  Like this it mimiks the real PCIe device.
   * 
   *  Deriving from this class, you can write dedicated implementations
   *  with special functionality.
   *  For this purpose one can register write callback function which are
   *  executed if a certain register (or range of registers) is written.
   *  For instance: Writing to a START_DAQ register
   *  can fill a data buffer with dummy values which can be read back.
   *  For each call of writeReg or writeArea the callback function is called once.
   *  If you require the callback function to be executed after each
   *  register change, use writeReg multiple times instead of writeArea.
   *  
   *  Registers can be set read only. In this
   *  case a write operation will just be ignored and no callback
   *  function is executed.
   */
  class DummyDevice : public devBase
  {
  public:

    DummyDevice();
    virtual ~DummyDevice();
             
    virtual void openDev(const std::string &mappingFileName,
			 int perm = O_RDWR, devConfigBase* pConfig = NULL);
    virtual void closeDev();
    
    virtual void readReg(uint32_t regOffset, int32_t* data, uint8_t bar);
    virtual void writeReg(uint32_t regOffset, int32_t data, uint8_t bar);
    
    virtual void readArea(uint32_t regOffset, int32_t* data, size_t size,
			  uint8_t bar);
    virtual void writeArea(uint32_t regOffset, int32_t* data, size_t size,
			   uint8_t bar);
    
    virtual void readDMA(uint32_t regOffset, int32_t* data, size_t size,
			 uint8_t bar);
    virtual void writeDMA(uint32_t regOffset, int32_t* data, size_t size,
			  uint8_t bar) ; 
    
    virtual void readDeviceInfo(std::string* devInfo);

    static uint64_t calculateVirtualRegisterAddress(
	uint32_t registerOffsetInBar,
	uint8_t bar);
							 
  protected:
    typedef std::pair<uint64_t, uint64_t>  AddressRange;

    std::map< uint8_t, std::vector<int32_t> > _barContents;
    std::set< uint64_t > _writeOnlyAddresses;
    std::multimap< AddressRange, boost::function<void(void)> > _writeCallbackFunctions;
    ptrmapFile _registerMapping;

    void resizeBarContents();
    std::map< uint8_t, size_t > getBarSizesInBytesFromRegisterMapping() const;
    void runWriteCallbackFunctionsForAddressRange( AddressRange addressRange ) const;
    std::list< boost::function<void(void)> > findCallbackFunctionsForAddressRange(AddressRange addressRange);
    void setReadOnly( AddressRange addressRange );
    void setWriteCallbackFunction( AddressRange addressRange,
				   boost::function<void(void)>  const & writeCallbackFunction );
    static void checkSizeIsMultipleOfWordSize(size_t size);
  };

}//namespace mtca4u

#endif // MTCA4U_DUMMY_DEVICE_H

#ifndef MTCA4U_DUMMY_DEVICE_H
#define MTCA4U_DUMMY_DEVICE_H

#include <vector>
#include <map>

#include <boost/function.hpp>

#include <MtcaMappedDevice/devBase.h>
#include <MtcaMappedDevice/mapFile.h>

namespace mtca4u{
  
  /** The dummy device opens a mapping file instead of a device, and
   *  implements all registers defined in the mapping file im memory.
   *  Like this it mimiks the real PCIe device.
   * 
   *  Deriving from this class, you can write dedicated implementations
   *  with special functionality.
   */
  class DummyDevice : public devBase
  {
  public:

    /** Implementation of a (set of) registers in memory. 
     *  The data content is stored as a vector, so we can do
     *  range checking.
     *
     *  The register can be set read only. In this
     *  case a write operation will just be ignored.
     *
     *  The write callback function provides an interface to implement
     *  functionality. For instance: Writing to a START_DAQ register
     *  can fill a data buffer with dummy values which can be read back.
     */
    class FakeRegister{
      std::vector<int> _dataContent;
      bool _isReadOnly;
      boost::function<void(void)> _writeCallbackFunction;

    public:
      FakeRegister(size_t sizeInWords_);
      void write(int32_t dataWord, size_t offsetInWords = 0);
      int32_t read(size_t offsetInWords = 0);
      void setReadOnly(bool isReadOnly_ = true);
      bool isReadOnly();
      void setWriteCallbackFunction(
	    boost::function<void(void)> _writeCallbackFunction );
      size_t sizeInWords();
    };

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
    std::map< uint64_t, FakeRegister > _registers;
    ptrmapFile _registerMapping;

    void populateRegisterMap();
    void addOrResizeRegister( mapFile::mapElem const & mappingElement );
  };

}//namespace mtca4u

#endif // MTCA4U_DUMMY_DEVICE_H

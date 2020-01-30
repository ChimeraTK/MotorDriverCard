#include "impl/TMC429SPI.h"
#include "TMC429Constants.h"

namespace mtca4u {

  // TMC429SPI::TMC429SPI(  boost::shared_ptr< Device<BaseDevice> > const &
  // device,
  TMC429SPI::TMC429SPI(boost::shared_ptr<ChimeraTK::Device> const& device,
      std::string const& moduleName,
      std::string const& writeRegisterName,
      std::string const& syncRegisterName,
      std::string const& readbackRegisterName,
      unsigned int spiWaitingTime)
  : _spiViaPCIe(device, moduleName, writeRegisterName, syncRegisterName, readbackRegisterName, spiWaitingTime) {}

  TMC429OutputWord TMC429SPI::read(unsigned int smda, unsigned int idx_jdx) {
    // Although the first half is almost identical to write,
    // the preparation of the TMC429InputWord is different. So we accept a little
    // code duplication here.
    TMC429InputWord readRequest;
    readRequest.setSMDA(smda);
    readRequest.setIDX_JDX(idx_jdx);
    readRequest.setRW(tmc429::RW_READ);

    return TMC429OutputWord(_spiViaPCIe.read(readRequest.getDataWord()));
  }

  void TMC429SPI::write(unsigned int smda, unsigned int idx_jdx, unsigned int data) {
    TMC429InputWord writeMe;
    writeMe.setSMDA(smda);
    writeMe.setIDX_JDX(idx_jdx);
    writeMe.setRW(tmc429::RW_WRITE);
    writeMe.setDATA(data);

    write(writeMe);
  }

  void TMC429SPI::write(TMC429InputWord const& writeWord) { _spiViaPCIe.write(writeWord.getDataWord()); }

} // namespace mtca4u

#include "SPIviaPCIe.h"

namespace mtca4u{

  SPIviaPCIe::SPIviaPCIe( boost::shared_ptr< devMap<devBase> > & mappedDevice,
			  std::string const & writeRegisterName, std::string const & syncRegisterName )
    : _writeRegister( mappedDevice->getRegObject( writeRegisterName ) ),
      _synchronisationRegister(  mappedDevice->getRegObject( syncRegisterName ) ),
      // will always return the last written word
      _readbackRegister(  mappedDevice->getRegObject( writeRegisterName ) )
  {}

  SPIviaPCIe::SPIviaPCIe( boost::shared_ptr< devMap<devBase> > & mappedDevice,
			  std::string const & writeRegisterName, std::string const & syncRegisterName,
			  std::string const & readbackRegisterName )
    : _writeRegister( mappedDevice->getRegObject( writeRegisterName ) ),
      _synchronisationRegister(  mappedDevice->getRegObject( syncRegisterName ) ),
      _readbackRegister(  mappedDevice->getRegObject(readbackRegisterName ) )
  {}


}// namespace mtca4u

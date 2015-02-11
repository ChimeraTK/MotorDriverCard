#ifndef MTCA4U_TMC429_SPI_H
#define MTCA4U_TMC429_SPI_H

#include "SPIviaPCIe.h"
#include "TMC429Words.h"

namespace mtca4u{
  /** This class implements the special format of the SPI regsiters in the TPC429 chip.
   */
  class TMC429SPI{
  public:
    TMC429SPI(  boost::shared_ptr< devMap<devBase> > const & mappedDevice,
		std::string const & writeRegisterName,
		std::string const & syncRegisterName,
		std::string const & readbackRegisterName,
		unsigned int spiWaitingTime = SPIviaPCIe::SPI_DEFAULT_WAITING_TIME );

    TMC429OutputWord read( unsigned int smda, unsigned int idx_jdx );
    void write( unsigned int smda, unsigned int idx_jdx, unsigned int data );
    void write( TMC429InputWord const & writeWord );

  private:
    SPIviaPCIe _spiViaPCIe;
  };

}//namespace mtca4u

#endif// MTCA4U_TMC429_SPI_H

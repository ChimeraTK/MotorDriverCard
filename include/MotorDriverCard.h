#ifndef MTCA4U_MOTOR_DRIVER_CARD_H
#define MTCA4U_MOTOR_DRIVER_CARD_H

#include "TMC429Words.h"
#include <boost/shared_ptr.hpp>

namespace mtca4u
{
  class MotorControler;
  class PowerMonitor;

  /**
   * A class to access the DFMC-MD22 motor driver card, which provides two 
   * MotorControlers.
   */
  class MotorDriverCard{
  public:
    /** Get acces to one of the two motor controlers on this board. 
     *  Valid IDs are 0 and 1.
     *  Throws a MotorDriverException if the ID is invalid.
     */
    virtual boost::shared_ptr<MotorControler> getMotorControler(unsigned int motorControlerID) = 0;
    
    /// Get a reference to the power monitor.
    virtual PowerMonitor & getPowerMonitor() = 0;
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_CARD_H

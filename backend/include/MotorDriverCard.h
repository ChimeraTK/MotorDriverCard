#ifndef MTCA4U_MOTOR_DRIVER_CARD_H
#define MTCA4U_MOTOR_DRIVER_CARD_H

#include "TMC429Words.h"

#include <boost/shared_ptr.hpp>

namespace mtca4u {
  class MotorControler;
  class PowerMonitor;

  /**
   * A class to access the DFMC-MD22 motor driver card, which provides two
   * MotorControlers.
   */
  class MotorDriverCard {
   public:
    /** Get acces to one of the two motor controlers on this board.
     *  Valid IDs are 0 and 1.
     *  Throws a MotorDriverException if the ID is invalid.
     */
    virtual boost::shared_ptr<MotorControler> getMotorControler(unsigned int motorControlerID) = 0;

    /// Get a reference to the power monitor.
    virtual PowerMonitor& getPowerMonitor() = 0;

    /** Report whether the underlying device is functional.
     *  This is the device state, not a card readiness signal: it can be true
     *  while the accessors are still stale after a device re-open, until
     *  reconfigure() has run.
     */
    [[nodiscard]] virtual bool isFunctional() const = 0;

    /** Restore the card after its device was closed and re-opened.
     *
     *  Re-creates the register accessors, re-applies the stored configuration
     *  and resets cached runtime state and settings to the configured values.
     *  The actual position and the state cached in BasicStepperMotor objects are
     *  not touched, and the device is neither opened nor closed.
     *
     *  The caller must not use the card while this runs. If it throws, the card
     *  must not be used until a later call after the device was re-opened.
     *
     *  @param enableMotor Enable the motor current after the configuration was
     *      applied. Defaults to disabled.
     */
    virtual void reconfigure(bool enableMotor = false) = 0;

    virtual ~MotorDriverCard() {}
  };

} // namespace mtca4u

#endif // MTCA4U_MOTOR_DRIVER_CARD_H

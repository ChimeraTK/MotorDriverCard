namespace mtca4u
{
  class MotorControler;

  /**
   * A class to access the DFMC-MD22 motor driver card, which prividers two 
   * MotorControlers.
   */
  class MotorDriverCard{
  public:

    /** Get acces to one of the two motor controlers on this board. 
     *  Valid IDs are 0 and 1.
     *  Throws a MotorDriverException if the ID is invalid.
     */
    virtual MotorControler & getMotorControler(unsigned int motorControlerID) = 0;
    
 
    /// FIXME: expert or not?
    virtual unsigned int getControlerChipVersion() = 0;
 
    /// Get a reference to the power monitor.
    virtual PowerMonitor & getPowerMonitor() = 0;
  };
  
}// namespace mtca4u

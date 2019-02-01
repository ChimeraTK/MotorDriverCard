/**
 * File:   StepperMotorUtil.h
 * Author: ckampm
 *
 * Utility classes fot the StepperMotor.
 * This header is used to share classes between the mtca4u
 * and ChimeraTK implementations.
 */

#ifndef CHIMERATK_STEPPER_MOTOR_UTIL_H
#define	CHIMERATK_STEPPER_MOTOR_UTIL_H


namespace ChimeraTK {
namespace motordriver{

  /**
   *  @brief Enum type describing how the system is calibrated:
   *
   *  0 - NONE:   No calibration has been determined or it has been lost.\n
   *  1 - SIMPLE: The motor position has been defined by setActualPosition().\n
   *              No valid position for the end switches can be determined. Useful if driving in full range between the end switches is possible.\n
   *  2 - FULL:   The motor has been calibrated by calling calibrate(). The end switch position has been determined.\n
   */
  enum class StepperMotorCalibrationMode{
    NONE = 0,
    SIMPLE = 1 << 0,
    FULL = 1 << 1\
  };

  /**
   * Enum type to select the StepperMotor variant when
   * created by the StepperMotorFactory
   *
   * BASIC:  A basic stepper motor
   * LINEAR: A linear stage, supports hardware end switches
   */
  enum class StepperMotorType{BASIC, LINEAR};


  enum class StepperMotorRet{
    SUCCESS,
    ERR_SYSTEM_IN_ACTION,
    ERR_INVALID_PARAMETER,
    ERR_SYSTEM_NOT_CALIBRATED
  };


  enum class StepperMotorError{
    NO_ERROR = 0,
    ACTION_ERROR = 1 << 0,
    CALIBRATION_ERROR =  1 << 1,
    BOTH_END_SWITCHES_ON = 1 << 2,
    MOVE_INTERRUPTED = 1 << 3,
    EMERGENCY_STOP = 1 << 4
  };

  /**
   * @brief Namespace for utility classes related to the StepperMotor
   */
  namespace utility{

    /**
     * @brief A generic abstract units converter between an integer value (steps)\n
     *        and a user-defined unit.
     * @tparam T Type of the user-unit
     */
    template<typename T>
    class UnitsConverter{
    public:
      virtual T stepsToUnits(int steps) = 0;
      virtual int unitsToSteps(T units) = 0;
      virtual ~UnitsConverter(){}
    };

    /**
     * @brief A 1:1 units converter between an integer value (steps) and a user-defined unit.
     * @tparam T Type of the user-unit
     */
    template<typename T>
    class UnitsConverterTrivia : public UnitsConverter<T>{
    public:
      UnitsConverterTrivia(){}

      virtual T stepsToUnits(int steps){
        return static_cast<T> (steps);
      }
      virtual int unitsToSteps(T units){
        return static_cast<int> (units);
      }
    };

    /**
     * @brief Implementation of scaling UnitsConverter.\n
     *        Provides simple mapping: steps = ratio * userUnit.
     * @tparam T Type of the user-defined unit
     */
    template<typename T>
    class ScalingUnitsConverter : public UnitsConverter<T>{
      T _userUnitToStepsRatio;
    public:
      ScalingUnitsConverter(const T userUnitToStepsRatio)
        : _userUnitToStepsRatio(userUnitToStepsRatio){}
      virtual T stepsToUnits(int steps){
        return _userUnitToStepsRatio * steps;
      }
      virtual int unitsToSteps(T units){
       return units/_userUnitToStepsRatio;
      }
    };

    /**
     * @class StepperMotorUnitsConverter
     * @details Abstract class which creates interface. It contains two abstract methods which should be implemented in derived class. \n
     *          Methods allows to recalculate steps to arbitrary units and arbitrary units into steps.
     */
    using StepperMotorUnitsConverter = UnitsConverter<float>;

    /**
     * @class StepperMotorUnitsScalingConverter
     * @details A convenience implementation for a scaling units converter
     */
    using StepperMotorUnitsScalingConverter = ScalingUnitsConverter<float>;


    /**
     * @class StepperMotorUnitsConveterTrivia
     * @details Trivial implementation of StepperMotorUnitsConverter. 1:1 conversion between units and steps
     *          Used as default conversion if nothing else is specified
     */
    using StepperMotorUnitsConverterTrivia = UnitsConverterTrivia<float>;

    /**
     * @class EncoderUnitsConverter
     * @details Abstract class which creates interface. It contains two abstract methods which should be implemented in derived class.
     */
    using EncoderUnitsConverter        = UnitsConverter<double>;
    using EncoderUnitsScalingConverter = ScalingUnitsConverter<double>;
    using EncoderUnitsConverterTrivia  = UnitsConverterTrivia<double>;
  }

  // Make available in parent namespace for compatiblity to mtca4u interface
  //using utility::StepperMotorUnitsConverter;
  //using utility::StepperMotorUnitsConverterTrivia;
} //namespace motordriver
} //namespace ChimeraTK
#endif // CHIMERATK_STEPPER_MOTOR_UTIL_H

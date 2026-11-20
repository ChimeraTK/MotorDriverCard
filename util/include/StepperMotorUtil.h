// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <string>
namespace ChimeraTK::MotorDriver {

  /**
   *  Enum type describing how the system is calibrated:
   *
   *  0 - NONE:   No calibration has been determined or it has been lost.\n
   *  1 - SIMPLE: The motor position has been defined by setActualPosition().\n
   *              No valid position for the end switches can be determined. Useful
   * if driving in full range between the end switches is possible.\n 2 - FULL:
   * The motor has been calibrated by calling calibrate(). The end switch position
   * has been determined.\n
   */
  enum class CalibrationMode { NONE = 0, SIMPLE = 1U << 0U, FULL = 1U << 1U };

  /**
   * Enum type to select the StepperMotor variant when
   * created by the StepperMotorFactory
   *
   * BASIC:  A basic stepper motor
   * LINEAR: A linear stage, supports hardware end switches
   */
  enum class StepperMotorType { BASIC, LINEAR, ROTARY };

  /**
   * Enum describing the exit status of setting member functions
   *
   * The member functions for setting parameters to the StepperMotor
   * object return this to provide user feedback in case an input can
   * not be applied due to the system's current state or an invalid value.
   */
  enum class ExitStatus { SUCCESS, ERR_SYSTEM_IN_ACTION, ERR_INVALID_PARAMETER, ERR_SYSTEM_NOT_CALIBRATED };

  std::string toString(ExitStatus& status);

  /**
   * Enum class describing errors
   *
   * This is returned by getError() and differs from NO_ERROR
   * when the motor's StateMachine is in state "error"
   */
  enum class Error {
    NO_ERROR = 0,
    ACTION_ERROR = 1U << 0U,
    CALIBRATION_ERROR = 1U << 1U,
    BOTH_END_SWITCHES_ON = 1U << 2U,
    MOVE_INTERRUPTED = 1U << 3U,
    EMERGENCY_STOP = 1U << 4U
  };

  std::string toString(Error& error);

  /// Namespace for utility classes related to the StepperMotor
  namespace utility {

    bool checkIfOverflow(int termA, int termB);

    /**
     * @brief A generic abstract units converter between an integer value (steps)\n
     *        and a user-defined unit.
     * @tparam T Type of the user-unit
     */
    template<typename T>
    class UnitsConverter {
     public:
      virtual T stepsToUnits(int steps) = 0;
      virtual int unitsToSteps(T units) = 0;
      virtual ~UnitsConverter() = default;
    };

    /**
     * @brief A 1:1 units converter between an integer value (steps) and a
     * user-defined unit.
     * @tparam T Type of the user-unit
     */
    template<typename T>
    class UnitsConverterTrivia : public UnitsConverter<T> {
     public:
      UnitsConverterTrivia() = default;

      T stepsToUnits(int steps) override { return static_cast<T>(steps); }
      int unitsToSteps(T units) override { return static_cast<int>(units); }
    };

    /**
     * @brief Implementation of scaling UnitsConverter.\n
     *        Provides simple mapping: steps = ratio * userUnit.
     * @tparam T Type of the user-defined unit
     */
    template<typename T>
    class ScalingUnitsConverter : public UnitsConverter<T> {
      T _userUnitToStepsRatio;

     public:
      explicit ScalingUnitsConverter(const T userUnitToStepsRatio) : _userUnitToStepsRatio(userUnitToStepsRatio) {}
      T stepsToUnits(int steps) override { return _userUnitToStepsRatio * steps; }
      int unitsToSteps(T units) override { return units / _userUnitToStepsRatio; }
    };

    /**
     * @class StepperMotorUnitsConverter
     * @details Abstract class which creates interface. It contains two abstract
     * methods which should be implemented in derived class. \n Methods allows to
     * recalculate steps to arbitrary units and arbitrary units into steps.
     */
    using MotorStepsConverter = UnitsConverter<float>;

    /**
     * @class StepperMotorUnitsScalingConverter
     * @details A convenience implementation for a scaling units converter
     */
    using ScalingMotorStepsConverter = ScalingUnitsConverter<float>;

    /**
     * @class StepperMotorUnitsConveterTrivia
     * @details Trivial implementation of StepperMotorUnitsConverter. 1:1 conversion
     * between units and steps Used as default conversion if nothing else is
     * specified
     */
    using MotorStepsConverterTrivia = UnitsConverterTrivia<float>;

    /**
     * @class EncoderUnitsConverter
     * @details Abstract class which creates interface. It contains two abstract
     * methods which should be implemented in derived class.
     */
    using EncoderStepsConverter = UnitsConverter<double>;
    using ScalingEncoderStepsConverter = ScalingUnitsConverter<double>;
    using EncoderStepsConverterTrivia = UnitsConverterTrivia<double>;

  } // namespace utility

} // namespace ChimeraTK::MotorDriver

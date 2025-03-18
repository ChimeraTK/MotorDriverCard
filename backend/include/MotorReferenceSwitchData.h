// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "MultiVariableWord.h"

namespace ChimeraTK {

  /** A class containing an unigned int which is
   *  used as a bit field holding the status bits
   *  of both end switches.
   *
   *  There are two types of information available
   *  \li If a switch is active
   *  \li If a switch is enabled
   *
   *  In case a switch is disabled the motor will not stop at the corresponding
   * mechanical position and the switch is always reported to be inactive.
   *
   * If the switch is enabled the switch is inactive if the end positon is not
   * reached and becomes active if the switch position is reached. In case of an
   * "active open" switch
   *
   * @attention This class is only a container and does not check for data
   * consistency. It is possible to set switches active, even though the
   * corresponding  enabled bits are off.
   *
   */
  class MotorReferenceSwitchData : public MultiVariableWord {
   public:
    /// The default constructor initialises everyting to 0 (all switches inactive
    /// and disabled);
    MotorReferenceSwitchData(unsigned int dataWord = 0);

    /** The switch which is reached when moving in positive direction.
     *  \li 0 = inactive
     *  \li 1 = active
     */
    ADD_VARIABLE(PositiveSwitchActive, 0, 0);

    /** The switch which is reached when moving in negative direction.
     *  \li 0 = inactive
     *  \li 1 = active
     */
    ADD_VARIABLE(NegativeSwitchActive, 1, 1);

    /// A word containing the active flags for all (both) switches.
    ADD_VARIABLE(SwitchesActiveWord, 0, 1);

    /** The switch which is reached when moving in positive direction.
     *  \li 0 = disabled
     *  \li 1 = enabled
     */
    ADD_VARIABLE(PositiveSwitchEnabled, 16, 16);

    /** The switch which is reached when moving in negative direction.
     *  \li 0 = disabled
     *  \li 1 = enabled
     */
    ADD_VARIABLE(NegativeSwitchEnabled, 17, 17);

    /// A word containing the enabled flags for all (both) switches.
    ADD_VARIABLE(SwitchesEnabledWord, 16, 17);

    /** Operator to allow conversion to bool for convenient checking if any of the
     * switches is active. Example: if (motor->getReferenceSwitchData()) { cout <<
     * "one or both end switches are active" << endl; }
     */
    operator bool() const;
  };

} // namespace ChimeraTK

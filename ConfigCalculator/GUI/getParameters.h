// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

class ParametersPanel;
#include "MotorControlerConfig.h"
#include "MotorDriverCardConfig.h"

/** Get the motor parameters from the motor panel.
 *  It cannot be a member funcion because the panel knows nothing
 *  about being a motor panel. It just has named widgets.
 */
ChimeraTK::MotorControlerConfig getMotorParameters(ParametersPanel* parametersPanel);

/** Get the motor driver card parameters. The object is returning with default
 * MotorCotrolerConfigs (FIXME: does this make sense?)
 */
ChimeraTK::MotorDriverCardConfig getMotorDriverCardParameters(ParametersPanel* parametersPanel);

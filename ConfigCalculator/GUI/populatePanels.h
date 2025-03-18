// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

class ParametersPanel;

/// add parameters to the motor expert panel (helper function instead of
/// defining a new class)
void populateMotorExpertPanel(ParametersPanel* parametersPanel);

/// add parameters to the driver card expert panel (helper function instead of
/// defining
void populateDriverCardExpertPanel(ParametersPanel* parametersPanel);

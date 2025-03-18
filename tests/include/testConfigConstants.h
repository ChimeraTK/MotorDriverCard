// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <ChimeraTK/RegisterPath.h>

const ChimeraTK::RegisterPath MODULE_NAME_0("MD22_0");
const ChimeraTK::RegisterPath MODULE_NAME_1("MD22_1");

const std::string MAP_FILE_NAME("newer_firmware_mapfile.map");
const std::string DFMC_ALIAS("DFMC_MD22");
const std::string DFMC_ALIAS2("DFMC_MD22_PERSISTENT_BACKEND");
const std::string BROKEN_MAP_FILE_NAME("DFMC_MD22_broken.mapp");
const std::string DUMMY_MOC_MAP("mtcadummy_DFMC_MD22_mock.mapp");

const std::string CONFIG_FILE("VT21-MotorDriverCardConfig.xml");

const std::string BROKEN_DUMMY_DEV_ALIAS("BROKEN_PLAIN_DUMMY");

static const std::string DUMMY_DEVICE_FILE_NAME{"/dummy/MotorDriverCard"};

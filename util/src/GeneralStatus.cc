// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "GeneralStatus.h"

#include <ostream>
#include <sstream>

namespace ChimeraTK {

  GeneralStatus::GeneralStatus(int id) : _id(id) {}

  GeneralStatus::~GeneralStatus() {}

  bool GeneralStatus::operator==(GeneralStatus const& right) const {
    if(_id != right._id) {
      return false;
    }

    return true;
  }

  bool GeneralStatus::operator!=(GeneralStatus const& right) const {
    if(_id == right._id) {
      return false;
    }

    return true;
  }

  GeneralStatus::operator int() const {
    return _id;
  }

  std::string GeneralStatus::asString() const {
    std::ostringstream stream;
    stream << "No name available in GeneralStatus object. Status id: "
           << "(" << _id << ")";
    return stream.str();
  }

  std::ostream& operator<<(std::ostream& out, const GeneralStatus& status) {
    out << status.asString();
    return out;
  }

  int GeneralStatus::getId() const {
    return _id;
  }

} // namespace ChimeraTK

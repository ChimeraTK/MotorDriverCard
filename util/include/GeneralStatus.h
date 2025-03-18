// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <boost/thread.hpp>

#include <string>

namespace ChimeraTK {

  class GeneralStatus {
   protected:
    int _id;
    // std::string name;
    // boost::mutex mutex;
   public:
    GeneralStatus(int id = 0);
    virtual ~GeneralStatus();

    bool operator==(GeneralStatus const& right) const;

    bool operator!=(GeneralStatus const& right) const;

    virtual std::string asString() const;

    int getId() const;

    operator int() const;

    friend std::ostream& operator<<(std::ostream& out, const GeneralStatus& status);
  };

} // namespace ChimeraTK

#ifndef MTCA4U_STATUS_GENERAL_H
#define MTCA4U_STATUS_GENERAL_H

#include <boost/thread.hpp>
#include <string>

namespace ChimeraTK {

class GeneralStatus {
protected:
  int _id;
  // std::string name;
  // boost::mutex mutex;
public:
  GeneralStatus();
  GeneralStatus(int id);
  // copy constructor
  GeneralStatus(const GeneralStatus &status);
  // assigment operator
  GeneralStatus &operator=(const GeneralStatus &other);
  virtual ~GeneralStatus();

  bool operator==(GeneralStatus const &right) const;

  bool operator!=(GeneralStatus const &right) const;

  virtual std::string asString() const;

  int getId() const;

  operator int() const;

  friend std::ostream &operator<<(std::ostream &out,
                                  const GeneralStatus &status);
};

} // namespace ChimeraTK

namespace mtca4u {
using namespace ChimeraTK;
}

#endif // MTCA4U_STATUS_GENERAL_H

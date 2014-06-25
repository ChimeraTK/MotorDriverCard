#include "GeneralStatus.h"
#include <ostream>
#include <sstream>


namespace mtca4u {

    GeneralStatus::GeneralStatus() : id(0), name("OK") {
    }

    GeneralStatus::GeneralStatus(int itemId, std::string itemName) : id(itemId), name(itemName) {
    }

    //copy constructor

    GeneralStatus::GeneralStatus(const GeneralStatus &status) : id(status.id), name(status.name) {
    }

    GeneralStatus& GeneralStatus::operator=(const GeneralStatus& other) {
        boost::lock_guard<boost::mutex> lock_guard(mutex);
        if (this != &other) {
            id = other.id;
            name = other.name;
        }
   
        return *this;
    }

    bool GeneralStatus::operator==(GeneralStatus const& right) const {
        if (id != right.id) {
            return false;
        }

        return true;
    }

    bool GeneralStatus::operator!=(GeneralStatus const& right) const {
        if (id == right.id) {
            return false;
        }

        return true;
    }

    std::string GeneralStatus::asString() const {
        std::ostringstream stream;
        stream << name << "(" << id << ")";
        return stream.str();
    }

    std::ostream &operator<<(std::ostream &out, const GeneralStatus &status) {
        out << status.asString();
        return out;
    }

    int GeneralStatus::getId() const {
        return id;
    }


}
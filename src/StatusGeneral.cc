#include "StatusGeneral.h"
#include <ostream>
#include <sstream>


namespace mtca4u {

    StatusGeneral::StatusGeneral() : id(0), name("OK") {
    }

    StatusGeneral::StatusGeneral(int itemId, std::string itemName) : id(itemId), name(itemName) {
    }

    //copy constructor

    StatusGeneral::StatusGeneral(const StatusGeneral &status) : id(status.id), name(status.name) {
    }

    StatusGeneral& StatusGeneral::operator=(const StatusGeneral& other) {
        boost::lock_guard<boost::mutex> lock_guard(mutex);
        if (this != &other) {
            id = other.id;
            name = other.name;
        }
   
        return *this;
    }

    bool StatusGeneral::operator==(StatusGeneral const& right) const {
        if (id != right.id) {
            return false;
        }

        return true;
    }

    bool StatusGeneral::operator!=(StatusGeneral const& right) const {
        if (id == right.id) {
            return false;
        }

        return true;
    }

    std::string StatusGeneral::asString() const {
        std::ostringstream stream;
        stream << name << "(" << id << ")";
        return stream.str();
    }

    std::ostream &operator<<(std::ostream &out, const StatusGeneral &status) {
        out << status.name << "(" << status.id << ")";
        return out;
    }

    int StatusGeneral::getId() const {
        return id;
    }


}
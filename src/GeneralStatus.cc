#include "GeneralStatus.h"
#include <ostream>
#include <sstream>


namespace ChimeraTK {

    GeneralStatus::GeneralStatus() : _id(0) {
    }

    GeneralStatus::GeneralStatus(int id) : _id(id) {
    }

    //copy constructor

    GeneralStatus::GeneralStatus(const GeneralStatus &status) : _id(status._id) {
    }

    GeneralStatus::~GeneralStatus(){
    }

    GeneralStatus& GeneralStatus::operator=(const GeneralStatus& other) {
        //boost::lock_guard<boost::mutex> lock_guard(mutex);
        if (this != &other) {
            _id = other._id;
        }
   
        return *this;
    }

    bool GeneralStatus::operator==(GeneralStatus const& right) const {
        if (_id != right._id) {
            return false;
        }

        return true;
    }

    bool GeneralStatus::operator!=(GeneralStatus const& right) const {
        if (_id == right._id) {
            return false;
        }

        return true;
    }

    GeneralStatus::operator int() const {
        return _id;
    }
    
    
    std::string GeneralStatus::asString() const {
        std::ostringstream stream;
        stream << "No name available in GeneralStatus object. Status id: " << "(" << _id << ")";
        return stream.str();
    }

    std::ostream &operator<<(std::ostream &out, const GeneralStatus &status) {
        out << status.asString();
        return out;
    }

    int GeneralStatus::getId() const {
        return _id;
    }


}

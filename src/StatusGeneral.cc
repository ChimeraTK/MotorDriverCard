#include "StatusGeneral.h"
#include <ostream>


namespace mtca4u {
        StatusGeneral::StatusGeneral() : id(0), name("OK") {
        }
        
        StatusGeneral::StatusGeneral(int itemId, std::string itemName) : id(itemId), name(itemName) {
        }

        //copy constructor
        StatusGeneral::StatusGeneral(const StatusGeneral &status) : id(status.id), name(status.name){            
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
        
        std::ostream &operator<<(std::ostream &out, const StatusGeneral &status) {
            out << status.name <<"("<<status.id<<")";
            return out;
        }
}
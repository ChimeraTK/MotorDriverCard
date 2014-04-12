#ifndef MTCA4U_STATUS_GENERAL_H
#define	MTCA4U_STATUS_GENERAL_H

#include <string>


namespace mtca4u {

    class StatusGeneral {
    protected:
        int id;
        std::string name;
    public:
        StatusGeneral();
        StatusGeneral(int itemId, std::string itemName);
        //copy constructor
        StatusGeneral(const StatusGeneral &status);      
    
        bool operator==(StatusGeneral const& right) const;

        bool operator!=(StatusGeneral const& right) const;

        friend std::ostream &operator<<(std::ostream &out, const StatusGeneral &status);
    };

} //namespace mtca4u

#endif //MTCA4U_STATUS_GENERAL_H
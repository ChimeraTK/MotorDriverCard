#ifndef MTCA4U_STATUS_GENERAL_H
#define	MTCA4U_STATUS_GENERAL_H

#include <string>
#include <boost/thread.hpp>


namespace mtca4u {

    class StatusGeneral {
    protected:
        int id;
        std::string name;
        boost::mutex mutex;
    public:
        StatusGeneral();
        StatusGeneral(int itemId, std::string itemName);
        //copy constructor
        StatusGeneral(const StatusGeneral &status);     
        //assigment operator
        StatusGeneral& operator= (const StatusGeneral& other);
    
        bool operator==(StatusGeneral const& right) const;

        bool operator!=(StatusGeneral const& right) const;
        

        
        std::string asString() const;
        
        int getId() const;

        friend std::ostream &operator<<(std::ostream &out, const StatusGeneral &status);
    };

} //namespace mtca4u

#endif //MTCA4U_STATUS_GENERAL_H
#ifndef MTCA4U_STATUS_GENERAL_H
#define	MTCA4U_STATUS_GENERAL_H

#include <string>
#include <boost/thread.hpp>


namespace mtca4u {

    class GeneralStatus {
    protected:
        int id;
        std::string name;
        boost::mutex mutex;
    public:
        GeneralStatus();
        GeneralStatus(int itemId, std::string itemName);
        //copy constructor
        GeneralStatus(const GeneralStatus &status);     
        //assigment operator
        GeneralStatus& operator= (const GeneralStatus& other);
    
        bool operator==(GeneralStatus const& right) const;

        bool operator!=(GeneralStatus const& right) const;
        

        
        std::string asString() const;
        
        int getId() const;

        friend std::ostream &operator<<(std::ostream &out, const GeneralStatus &status);
    };

} //namespace mtca4u

#endif //MTCA4U_STATUS_GENERAL_H
#ifndef MTCA4U_STATUS_GENERAL_H
#define	MTCA4U_STATUS_GENERAL_H

#include <string>
#include <boost/thread.hpp>


namespace mtca4u {

    class GeneralStatus {
    protected:
        int _id;
        //std::string name;
        //boost::mutex mutex;
    public:
        GeneralStatus();
        GeneralStatus(int id);
        //copy constructor
        GeneralStatus(const GeneralStatus &status);     
        //assigment operator
        GeneralStatus& operator= (const GeneralStatus& other);
    
        bool operator==(GeneralStatus const& right) const;

        bool operator!=(GeneralStatus const& right) const;
        

        
        virtual std::string asString() const;
        
        int getId() const;

        
        operator int () const;
        
        friend std::ostream &operator<<(std::ostream &out, const GeneralStatus &status);
    };

} //namespace mtca4u

#endif //MTCA4U_STATUS_GENERAL_H
#ifndef OpBasicInterface_h
#define OpBasicInterface_h

#include <string>

namespace OA {

class OpBasicInterface {
  public:
    OpBasicInterface() {}
    virtual ~OpBasicInterface() {}

    virtual bool operator<  (OpBasicInterface& other) = 0;
    virtual bool operator== (OpBasicInterface& other) = 0;
    virtual bool operator!= (OpBasicInterface& other) = 0;
    virtual std::string toString() = 0;
};

}

#endif 

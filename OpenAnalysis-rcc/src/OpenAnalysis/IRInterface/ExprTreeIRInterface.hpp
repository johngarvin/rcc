#ifndef ExprTreeIRInterface_h
#define ExprTreeIRInterface_h

#include "OpBasicInterface.hpp"
#include "ConstValBasicInterface.hpp"

namespace OA {

class ExprTreeIRInterface {
  public:
    OA_ptr<OpBasicInterface> getOpBasic(OpHandle hOp);

    OA_ptr<ConstValBasicInterface> getConstValBasic(ConstValHandle hConstVal);
};

}

#endif


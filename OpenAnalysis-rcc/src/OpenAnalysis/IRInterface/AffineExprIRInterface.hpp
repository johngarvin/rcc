#ifndef AffineExprIRInterface_H
#define AffineExprIRInterface_H

#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>
#include <OpenAnalysis/Location/Location.hpp>

namespace OA {
namespace AffineExpr {

enum OpType {
    OP_ADD = 0,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
    OP_SHIFT_LEFT,
    OP_SHIFT_RIGHT,
    OP_BIT_AND,
    OP_BIT_OR,
    OP_BIT_XOR,
    OP_OTHER
};

class AffineExprIRInterface :
    public virtual IRHandlesIRInterface
{
  public:
    AffineExprIRInterface() {}
    virtual ~AffineExprIRInterface() {}

    virtual OA_ptr<Location::Location>
        getLocation(ProcHandle p, SymHandle s) = 0;
    virtual int constValIntVal(ConstValHandle h) = 0;
    virtual OpType getOpType(OpHandle h) = 0;
    virtual OA_ptr<ExprTree> getExprTree(ExprHandle h) = 0;
    virtual OA_ptr<MemRefExprIterator>
        getMemRefExprIterator(MemRefHandle h) = 0;
    virtual MemRefHandle getSymMemRefHandle(SymHandle h) = 0;


    // needed for GCD:
    // !!! seperate into LoopDetection package
    // !!! Create interface for results.
    // bool isInLoop(StmtHanlde h, CFG cfg);
};

} } // end namespaces

#endif

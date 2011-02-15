/*! \file
  
  \brief Abstract IR interface for various data dependence analysis algorithms. 
  
  \authors Andy Strout

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef DataDepIRInterface_h
#define DataDepIRInterface_h

#include <OpenAnalysis/ExprTree/ExprTree.hpp>
#include "AffineExprIRInterface.hpp"

namespace OA {
namespace DataDep {

class DataDepIRInterface :
    public virtual AffineExpr::AffineExprIRInterface
{
  public:
    virtual OA_ptr<IdxExprAccessIterator>
        getIdxExprAccessIter(ProcHandle p) = 0;
};

} } // end namespaces

#endif


/*! \file
  
  \brief Abstract interface that all ReachConsts analysis results must satisfy.

  \authors Michelle Strout, Barbara Kreaseck

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef REACHCONSTSINTERFACE_H
#define REACHCONSTSINTERFACE_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/IRInterface/ConstValBasicInterface.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>

namespace OA {
  namespace ReachConsts {

class Interface : public virtual Annotation {

public:
  Interface(){}
  virtual ~Interface(){}

  //! return a ConstValBasicInterface for a given mem ref handle
  //! Returns NULL if no constant value exists for this MemRefHandle
  //virtual OA_ptr<ConstValBasicInterface> getReachConst(MemRefHandle h) = 0;
  //--------------------
  // above is obsolete ??

  //! returns a ConstValBasicInterface for the given MemRefExpr-AliasTagSet that reaches the given statement.
  virtual OA_ptr<ConstValBasicInterface> getReachConst(StmtHandle h, OA::OA_ptr<Alias::AliasTagSet> tset) = 0;
 
  //! returns a ConstValBasicInterface for the given MemRefExpr-AliasTagSet that reaches the given call.
  virtual OA_ptr<ConstValBasicInterface> getReachConst(CallHandle c, OA::OA_ptr<Alias::AliasTagSet> tset) = 0;
 

  virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir) = 0;

  //*****************************************************************
  // Annotation Interface
  //*****************************************************************
  virtual void output(OA::IRHandlesIRInterface& ir,
                      Alias::Interface &alias) const = 0;
  
};
  } // end of ReachConsts namespace
} // end of OA namespace

#endif

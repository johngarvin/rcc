/*! \file
  
  \brief Abstract IR interface for interprocedural SideEffect analysis.  
  
  \authors Michelle Strout
  \version $Id: InterSideEffectIRInterface.hpp,v 1.8 2005/02/18 16:15:51 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef InterSideEffectIRInterface_h
#define InterSideEffectIRInterface_h

#include <OpenAnalysis/IRInterface/SideEffectIRInterface.hpp>
#include <OpenAnalysis/IRInterface/CallGraphDFProblemIRInterface.hpp>
#include <OpenAnalysis/SideEffect/SideEffectStandard.hpp>

namespace OA {

  namespace SideEffect {


//! The InterSideEffectIRInterface abstract base class gives a set of methods
//! for querying the source IR for information relevant to interprocedural
//! SideEffect Analysis.
//! This is the primary interface for interprocedural SideEffect to the 
//! underlying intermediate representation.
class InterSideEffectIRInterface : public virtual SideEffectIRInterface,
                           public virtual DataFlow::CallGraphDFProblemIRInterface 
{
 public:
  InterSideEffectIRInterface() { }
  virtual ~InterSideEffectIRInterface() { }

  //! For the given callee subprocedure symbol return side-effect results
  //! Can only indicate that the procedure has no side effects, has
  //! side effects on unknown locations, or on global locations.
  //! Can't indicate subprocedure has sideeffects on parameters because
  //! don't have a way to get mapping of formal parameters to actuals
  //! in caller.
  //! FIXME?: for now assuming that the intra-procedural results
  //! are SideEffectStandard because the ManagerInterSideEffectStandard
  //! will need to have access to the construction methods
  virtual OA_ptr<SideEffect::SideEffectStandard> 
      getSideEffect(ProcHandle caller, SymHandle calleesym) = 0;

  // need all for full override, just want ProcHandle and can't inherit
  // from IRHandlesIRInterface as well as other two
  virtual std::string toString(const ProcHandle h) const = 0;
  virtual std::string toString(const StmtHandle h) const = 0;
  virtual std::string toString(const ExprHandle h) const = 0;
  virtual std::string toString(const OpHandle h) const = 0;
  virtual std::string toString(const MemRefHandle h) const = 0;
  virtual std::string toString(const SymHandle h) const = 0;
  virtual std::string toString(const ConstSymHandle h) const = 0;
  virtual std::string toString(const ConstValHandle h) const = 0;
};  

  } // end of namespace SideEffect
} // end of namespace OA

#endif 

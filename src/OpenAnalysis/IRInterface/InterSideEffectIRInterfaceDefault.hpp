/*! \file
  
  \brief Default implementation for interprocedural side effect analysis
         IR interface
  
  \authors Michelle Strout
  \version $Id: InterSideEffectIRInterfaceDefault.hpp,v 1.3 2004/12/06 06:53:56 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef InterSideEffectIRInterfaceDefault_h
#define InterSideEffectIRInterfaceDefault_h

#include <OpenAnalysis/IRInterface/InterSideEffectIRInterface.hpp>
#include <OpenAnalysis/SideEffect/SideEffectStandard.hpp>

namespace OA {

  namespace SideEffect {


class InterSideEffectIRInterfaceDefault : public virtual InterSideEffectIRInterface
{
 public:
  InterSideEffectIRInterfaceDefault() { }
  virtual ~InterSideEffectIRInterfaceDefault() { }

  //! Returns the default conservative SideEffectStandard results 
  virtual OA_ptr<SideEffect::SideEffectStandard> 
  getSideEffect(ProcHandle caller, SymHandle calleesym)
    {
      // TODO: need to resolve this so SideEffectStandard is not passed
      //       NULL alias results.
      OA_ptr<Alias::Interface> alias;

      OA_ptr<SideEffect::SideEffectStandard> retval;
      retval = new SideEffect::SideEffectStandard(alias);
      return retval;
    }

};  

  } // end of namespace SideEffect
} // end of namespace OA

#endif 

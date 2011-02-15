/*! \file
  
  \brief Abstract interface that the ICFG AnnotationManagers require.

  \authors Michelle Strout
  \version $Id: ICFGIRInterface.hpp,v 1.2 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>

  An ICFG AnnotationManager generates an ICFG  representation of the
  program.  The analysis engine within the ICFG AnnotationManager requires
  that the ICFGIRInterface be implemented so that queries can be made to
  the Source IR relevant to constructing call graphs.  
*/

#ifndef ICFGIRInterface_h
#define ICFGIRInterface_h

#include <iostream>
#include "IRHandles.hpp"

namespace OA {
  namespace ICFG {


/*! 
 * The ICFGIRInterface abstract base class gives a set of methods for 
 * manipulating a program.  This is the primary interface to the underlying 
 * intermediate representation.
 */
class ICFGIRInterface : public virtual IRHandlesIRInterface {
 public:
  ICFGIRInterface() { }
  virtual ~ICFGIRInterface() {} 
  
  //! Return an iterator over all of the callsites in a given stmt
  virtual OA_ptr<IRCallsiteIterator> getCallsites(StmtHandle h) = 0;

  //! Given a function call return the callee symbol handle
  //virtual SymHandle getSymHandle(ExprHandle expr) = 0;
  
  //! Given the callee symbol returns the callee proc handle
  virtual ProcHandle getProcHandle(SymHandle sym) = 0;
};


  } // end of namespace ICFG
} // end of namespace OA

#endif

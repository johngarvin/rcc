/*! \file
  
  \brief Concrete Location that will use alias results and parameter
         bindings to convert a callee location to a set of caller locations.
  
  \authors Michelle Strout
  \version $Id: CalleeToCallerVisitor.hpp,v 1.2 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef CalleeToCallerVisitorIRInterface_H
#define CalleeToCallerVisitorIRInterface_H

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>


namespace OA {
  
 namespace DataFlow {
//--------------------------------------------------------------------
/*! 
*/
  class CalleeToCallerVisitorIRInterface : public virtual IRHandlesIRInterface {
    public:
        //! get iterator over all caller locs that are associated
        //! with the callee loc that is being visited

       CalleeToCallerVisitorIRInterface() { }	
       virtual ~CalleeToCallerVisitorIRInterface() { }

     //! Given a MemRefHandle return an iterator over
     //! MemRefExprs that describe this memory reference
        virtual OA_ptr<MemRefExprIterator> 
		getMemRefExprIterator(MemRefHandle h) = 0;
  }; //end of calleeToCallerVisitorIRInterface

 } //end of DataFlow Namespace	

} // end of OA namespace

#endif

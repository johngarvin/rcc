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

#ifndef CalleeToCallerVisitor_H
#define CalleeToCallerVisitor_H

#include <OpenAnalysis/Location/LocationVisitor.hpp>
#include <OpenAnalysis/DataFlow/ParamBindings.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/IRInterface/InterSideEffectIRInterface.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExprVisitor.hpp>
#include <OpenAnalysis/IRInterface/ParamBindingsIRInterface.hpp>
#include <stack>
#include <OpenAnalysis/IRInterface/CalleeToCallerVisitorIRInterface.hpp>

namespace OA {

   namespace DataFlow {	 
//--------------------------------------------------------------------
/*! 
*/
class CalleeToCallerVisitor : public virtual LocationVisitor {
    public:
        //! get iterator over all caller locs that are associated
        //! with the callee loc that is being visited
        OA_ptr<LocIterator> getCallerLocIterator();

        CalleeToCallerVisitor(ProcHandle callee,
                              CallHandle call, ProcHandle caller,
                              OA_ptr<Alias::InterAliasTagInterface> interAlias,
                              OA_ptr<DataFlow::ParamBindings> paramBind,
                              OA_ptr<CalleeToCallerVisitorIRInterface> _ir);
	

        ~CalleeToCallerVisitor() {}
        
        void visitNamedLoc(NamedLoc& otherLoc); 

        void visitUnnamedLoc(UnnamedLoc& loc);

        void visitInvisibleLoc(InvisibleLoc& loc);

        void visitUnknownLoc(UnknownLoc& loc);

        void visitLocSubSet(LocSubSet& loc); 

      private:
        CallHandle mCall;
        ProcHandle mCaller;
        ProcHandle mCallee;
        OA_ptr<Alias::InterAliasTagInterface> mInterAlias;
        OA_ptr<DataFlow::ParamBindings> mParamBind;
        OA_ptr<Alias::Interface> mCallerAlias;
        OA_ptr<LocSet> mLocSet;
        OA_ptr<CalleeToCallerVisitorIRInterface> mIR;
    };
   } // end of DataFlow namespace
} // end of OA namespace

#endif

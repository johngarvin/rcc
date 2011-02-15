/*! \file
  
  \brief Definition of ICFGDep.

  \authors Michelle Strout
  \version $Id: ICFGDep.hpp,v 1.2 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ICFGDep_hpp
#define ICFGDep_hpp

#include <set>
#include <map>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Activity/DepDFSet.hpp>

namespace OA {
  namespace Activity {

/* 
   Maps each statement to a set of Dependences use,def where use is a 
   differentiable location used at some point when defining the def location.
   Also keeps track of must def locations for a statement.
   Doesn't have use,def pairs for procedure calls.  In statements with 
   procedure calls, just has other uses and defs.

   For example,
        a = foo(x, &y)
   has the Dep <x,a> but not <y,a> or <x,y>.  
*/
    class ICFGDep : public virtual Annotation {
  public:
    ICFGDep() {}
    ~ICFGDep() {}

    //*****************************************************************
    // Interface Implementation
    //*****************************************************************

    //! Return an iterator over all locations whose definition may 
    //! depend on the given use location in the given stmt.
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> >
        getMayDefIterator(StmtHandle stmt, Alias::AliasTag use) const;


    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> >
        getMayDefIterator(CallHandle call, Alias::AliasTag use) const;


    //! Return an iterator over all locations that are differentiable
    //! locations used in the possible definition of the given location,
    //! in the given stmt
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> >
        getDiffUseIterator(StmtHandle stmt, Alias::AliasTag def) const;
   

    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> >
        getDiffUseIterator(CallHandle call, Alias::AliasTag def) const;


    //! Return an iterator over all locations that are definitely
    //! defined in the given stmt
    OA_ptr<DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > 
        getMustDefIterator(StmtHandle stmt) const;

    //*****************************************************************
    // Construction methods
    //*****************************************************************
    
    //! map stmt to a DepDFSet
    void mapStmtToDeps(StmtHandle stmt, OA_ptr<DepDFSet> depDFSet)
      { mDepDFSet[stmt] = depDFSet; }


    void mapCallToDeps(CallHandle call, OA_ptr<DepDFSet> depDFSet)
      { mCallDepDFSet[call] = depDFSet; }


    //! Insert use,def dependence pair
    void insertDepForStmt(StmtHandle stmt, Alias::AliasTag use, Alias::AliasTag def);


    //! Insert use,def dependence pair
    void insertDepForCall(CallHandle call, Alias::AliasTag use, Alias::AliasTag def);


    //! Insert must def location 
    void insertMustDefForStmt(StmtHandle stmt, Alias::AliasTag def);
   
    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(OA::IRHandlesIRInterface& ir) const { assert(0); }

    void output(OA::IRHandlesIRInterface& ir,
                OA::Alias::Interface& alias) const; 

    //*****************************************************************
    // Output
    //*****************************************************************
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

  private:
    std::map<StmtHandle,OA_ptr<DepDFSet> > mDepDFSet;

    std::map<CallHandle,OA_ptr<DepDFSet> > mCallDepDFSet;

    std::map<StmtHandle,std::set<Alias::AliasTag> > mMustDefMap;

};


  } // end of Activity namespace
} // end of OA namespace

#endif


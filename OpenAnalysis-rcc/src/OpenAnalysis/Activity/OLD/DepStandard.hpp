/*! \file
  
  \brief Definition of DepStandard

  \authors Michelle Strout
  \version $Id: DepStandard.hpp,v 1.4 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef DepStandard_hpp
#define DepStandard_hpp

#include <set>
#include <map>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Location/Locations.hpp>

#include "DepDFSet.hpp"


namespace OA {
  namespace Activity {

/* 
   Maps each statement to a set of Dependences use,def where use is a 
   differentiable location used at some point when defining the def location.
   Also keeps track of must def locations for a statement.
*/
class DepStandard {
  public:
    DepStandard() {}
    ~DepStandard() {}

    //*****************************************************************
    // Interface Implementation
    //*****************************************************************

    //! Returns the DepDFSet for the whole procedure
    OA_ptr<DepDFSet> getFinalDep()
      { return mProcDepDFSet; }
    
    //! Return an iterator over all locations whose definition may 
    //! depend on the given use location in the given stmt.
    OA_ptr<LocIterator> 
        getMayDefIterator(StmtHandle stmt, const OA_ptr<Location> use);

    //! Return an iterator over all locations that are differentiable
    //! locations used in the possible definition of the given location,
    //! in the given stmt
    OA_ptr<LocIterator> 
        getDiffUseIterator(StmtHandle stmt, OA_ptr<Location> def);
    
    //! Return an iterator over all locations that are definitely
    //! defined in the given stmt
    OA_ptr<LocIterator> getMustDefIterator(StmtHandle stmt);

    //*****************************************************************
    // Construction methods
    //*****************************************************************
    
    //! map stmt to a DepDFSet
    void mapStmtToDeps(StmtHandle stmt, OA_ptr<DepDFSet> depDFSet)
      { mDepDFSet[stmt] = depDFSet; }

    //! DepDFSet for whole procedure
    void mapFinalDeps(OA_ptr<DepDFSet> depDFSet)
      { mProcDepDFSet = depDFSet; }

    //! Insert use,def dependence pair
    void insertDepForStmt(StmtHandle stmt, OA_ptr<Location> use,
            OA_ptr<Location> def);

    //! Insert must def location 
    void insertMustDefForStmt(StmtHandle stmt, OA_ptr<Location> def);
   
    //*****************************************************************
    // Output
    //*****************************************************************

    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

  private:
    OA_ptr<DepDFSet> mProcDepDFSet;
    std::map<StmtHandle,OA_ptr<DepDFSet> > mDepDFSet;

    std::map<StmtHandle,OA_ptr<LocSet> > mMustDefMap;

};


  } // end of Activity namespace
} // end of OA namespace

#endif


/*! \file
  
  \brief Maps StmtHandles to set of locations that are InActive.

  \authors Michelle Strout
  \version $Id: ActivePerStmt.hpp,v 1.1 2005/07/01 02:49:56 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ActivePerStmt_hpp
#define ActivePerStmt_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/DataFlow/DFSetBitImpl.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>

namespace OA {
  namespace Activity {



class ActivePerStmt : public virtual Annotation {
  public:
    ActivePerStmt();
    ~ActivePerStmt() {}

    //! =========== For Statement ==============

    //! returns a location data flow set of locations that 
    //! are active coming into the statement 
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > getInActiveSet(StmtHandle s);

    //! returns a location data flow set of locations that 
    //! are active coming out of the statement 
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > getOutActiveSet(StmtHandle s);


    //! ============ For Call ================

    //! returns a location data flow set of locations that
    //! are active coming into the statement
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > getInActiveSet(CallHandle c);

    //! returns a location data flow set of locations that
    //! are active coming out of the statement
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > getOutActiveSet(CallHandle c);


    //! returns the number of iterations over all nodes
    int getNumIter() { return mNumIter; }

    //*****************************************************************
    // Construction methods 
    //*****************************************************************

    //! ========= For the Statement =============

    //! set the InActive set for the statement
    void copyIntoInActive(StmtHandle s, 
                          OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > dfset);

    //! set the OutActive set for the statement
    void copyIntoOutActive(StmtHandle s, 
                           OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > dfset);
   

    //! ========== For the call Call ============

    //! set the InActive set for the statement
    void copyIntoInActive(CallHandle c,
                          OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > dfset);


    //! set the OutActive set for the statement
    void copyIntoOutActive(CallHandle c,
                           OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > dfset);

 
    //! set number of iterations over all nodes
    void setNumIter(int n) { mNumIter = n; }

    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface &ir) const;

    //*****************************************************************
    // Output
    //*****************************************************************

    //! incomplete output of info for debugging
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

  private:
    // data members
    std::map<StmtHandle,OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > > mInActive;
    std::map<StmtHandle,OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > > mOutActive;
    std::map<CallHandle,OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > > mCallInActive;
    std::map<CallHandle,OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > > mCallOutActive;
    int mNumIter;
};

  } // end of Activity namespace
} // end of OA namespace

#endif


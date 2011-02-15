/*! \file
  
  \brief Maps StmtHandles to set of StmtHandle defs that reach
         each StmtHandle 

  \authors Michelle Strout
  \version $Id: ReachDefsStandard.hpp,v 1.14 2004/12/21 21:15:19 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ReachDefsStandard_hpp
#define ReachDefsStandard_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/ReachDefs/Interface.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>

namespace OA {
  namespace ReachDefs {



class ReachDefsStandard : public virtual ReachDefs::Interface,
                          public virtual Annotation 
{
  public:
    ReachDefsStandard(ProcHandle p) 
      {
          mExitReachDefs = new std::set<StmtHandle>; 
      }
    ~ReachDefsStandard() {}

    //! Return an iterator that will iterate over the set of reaching
    //! defs for the stmt
    OA_ptr<Interface::ReachDefsIterator> getReachDefsIterator(StmtHandle s); 

    //! Return an iterator over reaching definitions that reach the exit node
    //! in the procedure
    OA_ptr<Interface::ReachDefsIterator> getExitReachDefsIterator(); 

    //*****************************************************************
    // Construction methods 
    //*****************************************************************

    //! insert a reaching def stmt into the given stmts reachdef set
    void insertReachDef(StmtHandle s, StmtHandle reachDef)
    { if (mReachDefs[s].ptrEqual(NULL)) {
        mReachDefs[s] = new std::set<StmtHandle>;
      }
      mReachDefs[s]->insert(reachDef); 
    }

    //! insert exit reaching def
    void insertExitReachDef(StmtHandle reachDef)
    { 
      mExitReachDefs->insert(reachDef); 
    }


    //*****************************************************************
    // Output
    //*****************************************************************

    //! will use OutputBuilder to generate output 
    void output(IRHandlesIRInterface& pIR) const;

    //! incomplete output of info for debugging, just lists stmts
    //! and associated set of stmts that are reaching defs for the given
    //! stmt
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

  protected:
    // data members
    std::map<StmtHandle,OA_ptr<std::set<StmtHandle> > >  mReachDefs;
    OA_ptr<std::set<StmtHandle> > mExitReachDefs;

  public:

    /*! Inherits interface and postfix from 
        Interface::ReachDefsIterator and inherits rest of implementation
        implementation from IRHandleSetIterator<StmtHandle>
    */
    class ReachDefsStandardReachDefsIterator 
                                : public IRHandleSetIterator<StmtHandle>,
                                  public Interface::ReachDefsIterator
    {
      public:
        ReachDefsStandardReachDefsIterator(OA_ptr<std::set<StmtHandle> > pSet) 
            : IRHandleSetIterator<StmtHandle>(pSet) {}
        ~ReachDefsStandardReachDefsIterator() {}

        void operator++() { IRHandleSetIterator<StmtHandle>::operator++(); }
        bool isValid() const 
          { return IRHandleSetIterator<StmtHandle>::isValid(); }
        StmtHandle current() const 
          { return IRHandleSetIterator<StmtHandle>::current(); }
        void reset() { IRHandleSetIterator<StmtHandle>::current(); }
    };
 
};

  } // end of ReachDefs namespace
} // end of OA namespace

#endif


/*! \file
  
  \brief Declarations and defs for class that maps procedures to CFG's.

  \authors Michelle Strout
  \version $Id: EachCFGStandard.hpp,v 1.3 2005/06/14 03:52:34 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef EachCFGStandard_H
#define EachCFGStandard_H

#include <OpenAnalysis/CFG/ManagerCFG.hpp>
#include <OpenAnalysis/CFG/EachCFGInterface.hpp>
#include <map>

namespace OA {
  namespace CFG {

class EachCFGStandard : public virtual EachCFGInterface {
  public:
    EachCFGStandard(OA_ptr<ManagerCFGStandard> cfgman) 
        : mCFGMan(cfgman) {}
    virtual ~EachCFGStandard() {}

    //! Returns CFG results for the given procedure
    OA_ptr<CFG::CFGInterface> getCFGResults(ProcHandle proc)
      { if (mProcToCFGMap[proc].ptrEqual(0)) {
          OA_ptr<CFG::CFGInterface> cfg = mCFGMan->performAnalysis(proc);
          mProcToCFGMap[proc] = cfg;
        }
        return mProcToCFGMap[proc];
      }

    //! Returns iterator over procedures with CFG results available
    /*
    OA_ptr<ProcHandleIterator> getKnownProcIterator() 
    {
        // create a procSet
        OA_ptr<std::set<ProcHandle> > procSet;
        procSet = new std::set<ProcHandle>;

        // loop through all maps to add to list of ProcHandle's
        std::map<ProcHandle,OA_ptr<CFG::Interface> >::iterator mIter;
        for (mIter=mProcToCFGMap.begin(); 
             mIter != mProcToCFGMap.end(); mIter++) 
        {
            procSet->insert(mIter->first);
        }

        OA_ptr<KnownProcIterator> retval;
        retval = new KnownProcIterator(procSet);
        return retval;
    }
    */

  private:
    /*! commented out by PLM 08/13/06
    OA_ptr<CFG::ManagerStandard> mCFGMan;
    */
    OA_ptr<ManagerCFGStandard> mCFGMan;
    std::map<ProcHandle,OA_ptr<CFG::CFGInterface> > mProcToCFGMap;
    
    /*
    //! An iterator over Procedures
    class KnownProcIterator : public virtual OA::ProcHandleIterator,
                         public OA::IRHandleSetIterator<OA::ProcHandle>
    {
    public:
      KnownProcIterator(OA::OA_ptr<std::set<OA::ProcHandle> > pSet) 
          : OA::IRHandleSetIterator<OA::ProcHandle>(pSet) {}
      ~KnownProcIterator() {}

      void operator++() { OA::IRHandleSetIterator<OA::ProcHandle>::operator++(); }
      bool isValid() const
        { return OA::IRHandleSetIterator<OA::ProcHandle>::isValid(); }
      OA::ProcHandle current() const
        { return OA::IRHandleSetIterator<OA::ProcHandle>::current(); }
      void reset() { OA::IRHandleSetIterator<OA::ProcHandle>::current(); }
    };
    */
};


  } // end of CFG namespace
} // end of OA namespace

#endif


/*! \file
  
  \brief Specially formatted and filtered Chains for XAIF.
  
  
  \authors Michelle Strout
  \version $Id: UDDUChainsXAIF.hpp,v 1.14 2005/01/17 18:46:34 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>

*/

#ifndef ChainsXAIF_hpp
#define ChainsXAIF_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Utils/SetIterator.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/UDDUChains/Interface.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>

namespace OA {

  /**
   * Namespace for analysis results that are specific to XAIF and do 
   * not satisfy general interfaces.
   */
  namespace XAIF {

    typedef std::set<StmtHandle> StmtSet;

    class ChainsXAIF : public virtual Annotation {
    public:

      ChainsXAIF();
 
     ~ChainsXAIF() {}

      static const int CHAIN_ID_NONE = -1; 

      class ChainIterator;

      class ChainsIterator;

      //! get an iterator over all chains 
      OA_ptr<ChainsIterator> getChainsIterator();
    
      //! for a particular id, get an iterator over the statements
      //! involved in that chain
      OA_ptr<ChainIterator> getChainIterator(int id);

      //! create a new chains set and return the id
      int makeEmptyChain();
 
      //! add a statement into the chain
      void addStmt(StmtHandle stmt, int id);

      //! add the given stmt set into the chain
      void addStmtSet(StmtSet& stmtSet, int id);

      //! return id that has identical chain of Stmts
      //! in it, else  return CHAIN_ID_NONE
      int findChain(StmtSet& stmtSet);


      //! ============== Output =================
      void output(OA::IRHandlesIRInterface& ir) const;
      void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);


    protected:

      OA_ptr<std::map<int,OA_ptr<StmtSet> > > mIdToStmtSetMap;
      
    public:

      /**
       * iterator over elements in a particular chain
       */
      class ChainIterator : public SetIterator<StmtHandle>
      {
      public:
        ChainIterator(OA_ptr<std::set<StmtHandle> > pSet) 
            : SetIterator<StmtHandle>(pSet) {}
        ~ChainIterator() {}
      }; 

      /**
       * iterator over all chains 
       */
      class ChainsIterator {
      public:
        ChainsIterator(OA_ptr<std::map<int,OA_ptr<StmtSet> > > pChainMap) 
	  : mChainMap(pChainMap) { reset(); }
        ~ChainsIterator() { }

        OA_ptr<ChainIterator> currentChainIterator()
          { OA_ptr<ChainIterator> retval;
            retval = new ChainIterator((*mChainMap)[mIter->first]);
            return retval;
          }
	
        int currentId() { return mIter->first; }
        bool isValid() { return (mIter != mChainMap->end()); }
        
        void operator++() { if (isValid()) mIter++; }
        void operator++(int) { ++*this; }
	
        void reset() { mIter = mChainMap->begin(); }

      private:
        OA_ptr<std::map<int,OA_ptr<StmtSet> > > mChainMap;
        std::map<int,OA_ptr<StmtSet> >::iterator mIter;
      };

    };

  } 
}


#endif


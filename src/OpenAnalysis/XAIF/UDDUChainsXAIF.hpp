/*! \file
  
  \brief Specially formatted and filtered UDChains for XAIF.
  
  
  \authors Michelle Strout
  \version $Id: UDDUChainsXAIF.hpp,v 1.14 2005/01/17 18:46:34 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  Special things about UDDUChainsXAIF:
  - If a chain element (StmtHandle) crosses basic block boundaries
    then the special StmtHandle(0) is used instead.  This includes crossing
    a BB for a loop.  For example, if a possible def for a use comes after
    the use in the BB, then that def will show up as StmtHandle(0) in
    the ud-chain.
    Also, we assume that definitions to non-local vars have an out-of-scope use.
  - The zeroth UDDUChain is not used, it is assumed to mean there is no
    information about a memory reference.
  - The first UDDUChain is an empty chain.
  - The second UDDUChains includes the StmtHandle(0) by itself.
  - All other chains are unique across procedures.
*/

#ifndef UDDUChainsXAIF_hpp
#define UDDUChainsXAIF_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Utils/SetIterator.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/UDDUChains/Interface.hpp>
#include <OpenAnalysis/XAIF/ChainsXAIF.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>

namespace OA {

  /**
   * Namespace for analysis results that are specific to XAIF and do 
   * not satisfy general interfaces.
   */
  namespace XAIF {


    /**
     * UD chains for XAIF map each MemRefHandle X to the set of 
     * StmtHandle's that may include a define for X as long as the StmtHandle
     * is within the same basic block, otherwise the StmtHandle(0) is specified.
     * DU chains map each MemRefHandle Y to the set of StmtHandle's that
     * may include a use of Y.  Again this is for statements within the same
     * basic block.
     */
    class UDDUChainsXAIF : public ChainsXAIF, public virtual Annotation {

    public:

      UDDUChainsXAIF();

      ~UDDUChainsXAIF() {}

      class MemRefsWithChainIterator; 

      /**
       * Return an iterator over all statements in this procedure
       * that may define a location used by the given memory reference
       * Assumes that the given memory reference is a use
       */
      OA_ptr<ChainIterator> getUDChainIterator(MemRefHandle h);

      /**
       * Return an iterator over all statements in this procedure
       * that may use a location defined by the given memory reference
       * Assumes that the given memory reference is a define
       */
      OA_ptr<ChainIterator> getDUChainIterator(MemRefHandle h);

      /**
       * Return an iterator over all the memory references for which
       * we have UD or DU Chain information
       */
      OA_ptr<MemRefsWithChainIterator> getMemRefsWithChainIterator();
      
      /**
       * get the UDDUChain Id for a particular memory reference
       * the zeroth chain will always be an empty chain
       * this method will always return a valid ChainID if the given
       * ref was analyzed
       */
      int getUDDUChainId(MemRefHandle ref);
      
      /**
       * insert the given mem ref into the chain
       */
      void insertInto(MemRefHandle ref, int Id);



      //! ================ Output Routines =========================

      void output(OA::IRHandlesIRInterface& ir) const;
      void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir);

    private:

      OA_ptr<std::map<MemRefHandle,int> > mMemRefToIdMap;

    public:

      class MemRefsWithChainIterator {
      public:
        MemRefsWithChainIterator(OA_ptr<std::map<MemRefHandle,int> > pMap) 
	  : mMap(pMap), mIter(pMap->begin()) {}
        ~MemRefsWithChainIterator() {}
	
        MemRefHandle current() { return mIter->first; }
        bool isValid() { return (mIter != mMap->end()); }
        void operator++() { if (isValid()) mIter++; }
        void operator++(int) { ++*this; }
        void reset() { mIter = mMap->begin(); }

      private:
        OA_ptr<std::map<MemRefHandle,int> > mMap;
        std::map<MemRefHandle,int>::iterator mIter;
      };

    };

  } 
} 

#endif


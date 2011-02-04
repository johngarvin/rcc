/*! \file
  
  \brief define-overwrite chains for XAIF.
  
  \authors Jean Utke

  See ../../../Copyright.txt for details. <br>

*/

#ifndef ReachDefsOverwriteXAIF_hpp
#define ReachDefsOverwriteXAIF_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "OpenAnalysis/XAIF/ChainsXAIF.hpp"

namespace OA {

  namespace XAIF {


    /**
     * define-overwrite chains for XAIF
     * currently doesn't add anything to 
     * ChainsXAIF but it might do so later
     */
    class ReachDefsOverwriteXAIF : public ChainsXAIF {

    public:

      ReachDefsOverwriteXAIF();

      ~ReachDefsOverwriteXAIF();

      /**
       * get the DoChain Id for a particular memory reference
       * the zeroth chain will always be an empty chain
       * this method will always return a valid ChainID if the given
       * ref was analyzed
       */ 
      int getChainId(StmtHandle stmt);

      /**
       * insert stmt into  mStmtToIdMap
       */ 
      void insert(StmtHandle ref, int setId);

    private: 
      
      /**
       * map statements to chain ids
       */ 
      OA_ptr<std::map<StmtHandle,int> > mStmtToIdMap;


    };

  } 
} 

#endif


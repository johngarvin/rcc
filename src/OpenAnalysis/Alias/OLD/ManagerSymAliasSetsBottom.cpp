/*! \file
  
NOTE: deprecated!!  No longer being compiled.
6/2/06, Removed ManagerInsNoPtrInterAliasMap because it is no longer
valid.  It used the CallGraph data-flow analysis framework
and dependended on the isRefParam() call, which has been deprecated.

  \brief The AnnotationManager that generates SymAliasSets bottom

  BROKEN!!! Not sure it needs fixed.  Can we get rid of it?

  \authors Michelle Strout
  \version $Id: ManagerSymAliasSetsBottom.cpp,v 1.2.6.1 2005/09/22 04:49:16 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerSymAliasSetsBottom.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Alias {

static bool debug = false;

// Visitor to collect SymHandle's from MemRefExpr's
class SymHandleCollector : public virtual MemRefExprVisitor {
    public:
      std::set<SymHandle> mSymSet;
      SymHandleCollector() {}
      ~SymHandleCollector() {}
      void visitNamedRef(NamedRef& ref) 
        { mSymSet.insert(ref.getSymHandle()); }
      void visitUnnamedRef(UnnamedRef& ref) { }
      void visitUnknownRef(UnknownRef& ref) { }
      // recursively visit mem-ref-exprs
      void visitDeref(Deref& ref) 
        { ref.getMemRefExpr()->acceptVisitor(*this); }
      void visitSubSetRef(SubSetRef& ref) 
        { ref.getMemRefExpr()->acceptVisitor(*this); }
};
 
/*!
   Gets all symbols from memory references in the program and for
   those that are not-local or are reference parameters, puts them
   all in the same set.
*/
OA_ptr<Alias::SymAliasSets> 
ManagerSymAliasSetsBottom::performAnalysis(ProcHandle proc) 
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerSymAliasSetsBottom:ALL", debug);
   
  // create a SymHandleCollector
  SymHandleCollector symCollector;

  OA_ptr<SymAliasSets> retMap; retMap = new Alias::SymAliasSets();

  // for each statement stmt
  OA_ptr<OA::IRStmtIterator> sIt = mIR->getStmtIterator(proc);
  for ( ; sIt->isValid(); (*sIt)++) {
    OA::StmtHandle stmt = sIt->current();
        
    if (debug) {
        std::cout << "\tstmt = ";
        mIR->dump(stmt,std::cout);
    }

    //   for each memory reference in the stmt
    OA_ptr<MemRefHandleIterator> mrIterPtr = mIR->getAllMemRefs(stmt);
    for (; mrIterPtr->isValid(); (*mrIterPtr)++ )
    {
      MemRefHandle memref = mrIterPtr->current();
      if (debug) {
        std::cout << "\tmemref = ";
        mIR->dump(memref,std::cout);
      }

      // get the memory reference expressions for this handle
      OA_ptr<MemRefExprIterator> mreIterPtr 
          = mIR->getMemRefExprIterator(memref);
      
      // for each mem-ref-expr associated with this memref
      for (; mreIterPtr->isValid(); (*mreIterPtr)++) {
        OA_ptr<OA::MemRefExpr> mre = mreIterPtr->current();
        if (debug) {
            std::cout << "\tmre = ";
            mre->dump(std::cout,mIR);
        }

        // have Visitor collect SymHandles
        mre->acceptVisitor(symCollector);

      }
    }
  }
 
  // iterate over symbols collected and get their locations to
  // determine if they are local and what other symbols they
  // may and must overlap
  SymHandle lastSym;
  std::set<SymHandle>::iterator symIter;
  for (symIter=symCollector.mSymSet.begin();
       symIter!=symCollector.mSymSet.end(); symIter++ )
  {
      SymHandle sym = *symIter;

      // first get location
      OA_ptr<Location> loc = mIR->getLocation(proc,sym);
      if (debug) {
          std::cout << "loc = ";
          loc->dump(std::cout,mIR);
      }

      // determine if we need to map the symbol to the big group
//      if (mIR->isRefParam(sym) || !loc->isLocal()) {
//    BROKEN, don't want to use isRefParam anymore
      if ( !loc->isLocal()) {
          
          // if the lastSym is something other than SymHandle(0)
          // merge the symbols containing those symbols
          SymHandle symNull = SymHandle(0);
          if (lastSym != symNull ) {
            retMap->mergeSyms(lastSym, sym);
          }

          // map all static overlap symbols to this one as well
          OA_ptr<NamedLoc> nmloc = loc.convert<NamedLoc>();
          OA_ptr<SymHandleIterator> overlapIter = nmloc->getPartOverlapIter();
          for ( ; overlapIter->isValid(); (*overlapIter)++ ) {
              retMap->mergeSyms(sym,overlapIter->current());
          }
          overlapIter = nmloc->getFullOverlapIter();
          for ( ; overlapIter->isValid(); (*overlapIter)++ ) {
              retMap->mergeSyms(sym,overlapIter->current());
          }

          lastSym = sym;
      }
  }

  return retMap;
}

  } // end of namespace Alias
} // end of namespace OA

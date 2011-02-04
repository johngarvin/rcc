/*! \file
  
  \brief The AnnotationManager that generates AliasMaps by giving
         each local variable its own number and virtual location and 
         mapping everyone else to unknown.  

  \authors Michelle Strout
  \version $Id: ManagerAliasMapBasic.cpp,v 1.28.6.1 2006/01/18 23:28:42 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerAliasMapBasic.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Alias {

static bool debug = false;

/*!
   Visitor over memory reference expressions that creates an
   appropriate Location data structure for any MemRefExpr.
   
   Conservatively handles addressOf (only way UnnamedRefs happen) and 
   derefs.  Therefore resulting mLoc will be UnknownLoc for those.
 */
class CreateLocationVisitor : public virtual MemRefExprVisitor {
  public:
    OA_ptr<Location> mLoc;
    CreateLocationVisitor(OA_ptr<AliasIRInterface> ir,
                          ProcHandle proc) : mIR(ir),mProc(proc) {}
    ~CreateLocationVisitor() {}
    void visitNamedRef(NamedRef& ref) 
      {
          mLoc = mIR->getLocation(mProc,ref.getSymHandle());

          /* PLM 1/23/07 deprecated hasFullAccuracy
          // if the memory reference doesn't have full accuracy then
          // create a partial subset of the base memory location
          if (!ref.hasFullAccuracy()) {
              mLoc = new LocSubSet(mLoc,false);
          }
          */

          // mre's with addressOf do not have locations associated with them
          //assert(ref.hasAddressTaken()!=true);
      }
    void visitUnnamedRef(UnnamedRef& ref) { mLoc = new UnknownLoc; }
    void visitUnknownRef(UnknownRef& ref) { mLoc = new UnknownLoc; }
    void visitAddressOf(AddressOf& ref) 
    {
        mLoc = new UnknownLoc();
    }

    void visitDeref(Deref& ref) { mLoc = new UnknownLoc; }
    // default handling of more specific SubSet specificiations
    void visitSubSetRef(SubSetRef& ref) 
      {
          // will set mLoc to our base location
          ref.getMemRefExpr()->acceptVisitor(*this);
          if (mLoc->isaNamed()) {
            mLoc = new LocSubSet(mLoc,false);
          }
      }

  private:
    OA_ptr<AliasIRInterface> mIR;
    ProcHandle mProc;

};


/*!
*/
OA_ptr<Alias::AliasMap> ManagerAliasMapBasic::performAnalysis(ProcHandle proc)
{
  OA_DEBUG_CTRL_MACRO("DEBUG_ManagerAliasMapBasic:ALL", debug);

  OA_ptr<AliasMap> retAliasMap; 
  retAliasMap = new AliasMap(proc);

  mProc = proc;

  // semantics of the AliasMap are that the zeroth set specifies that
  // no alias info is known about a memory reference, it could reference
  // any location
  unsigned int unknownSetId = 0;

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

      // each memref should only be mapped to one set,
      // therefore if memref has already been mapped once
      // we will want to add locations for other mres
      // to the same set
      int setId = AliasMap::SET_ID_NONE;

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

        // if mre is addressOf then do not need to map to a set
        if(mre->isaRefOp()) {
           OA_ptr<RefOp> refop = mre.convert<RefOp>();
           if(refop->isaAddressOf()) { continue; }
        }

        
        // if our memref has already been mapped then our locations
        // must be added to its set
        if (setId == AliasMap::SET_ID_NONE) {
            // see if we have not already mapped an equivalent memrefexpr
            setId = retAliasMap->getMapSetId(mre);
        }
        if (debug) {
            std::cout << "\tsetId = " << setId << std::endl;
        }

        // need to determine what locations mre maps to 
        OA_ptr<Location> loc;
        if (setId == AliasMap::SET_ID_NONE ) {
            
            // default set id
            setId = unknownSetId;

            // use visitor to get Location this MRE maps to
            CreateLocationVisitor locVisitor(mIR,proc);
            mre->acceptVisitor(locVisitor);
            loc = locVisitor.mLoc;
            
            // attempt to find location in current location sets
            setId = retAliasMap->getMapSetId(loc);
            if (debug) {
                std::cout << "\tsetId = " << setId << ", loc = ";
                loc->dump(std::cout, mIR);
            }
        }

        // if we didn't find the location in the aliasmap
        // sets then we need to put the location in a
        // new aliasmap set 
        if (setId==AliasMap::SET_ID_NONE) {
            // get a new set
            setId = retAliasMap->makeEmptySet();

            // put the loc into the new map set
            retAliasMap->addLocation(loc,setId);
        } 

        // map mre to set only if wasn't already mapped
        if (!mre.ptrEqual(0)) { retAliasMap->mapMemRefToMapSet(mre,setId); }

      } // loop over mre's
      
      // map memref to set
      retAliasMap->mapMemRefToMapSet(memref,setId);

    } // loop over memory references
  } // loop over statements

  return retAliasMap;
}

  } // end of namespace Alias
} // end of namespace OA

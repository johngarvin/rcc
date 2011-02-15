/*! \file
  
  \brief Implementation of CalleeToCallerVisitor class.
  
  \authors Michelle Strout, Andy Stone (alias tag update)
  \version $Id: CalleeToCallerVisitor.cpp,v 1.2 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "CalleeToCallerVisitor.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace DataFlow {

static bool debug = false;


        
CalleeToCallerVisitor::CalleeToCallerVisitor(
        ProcHandle callee, CallHandle call, ProcHandle caller,
        OA_ptr<Alias::InterAliasTagInterface> interAlias,
        OA_ptr<DataFlow::ParamBindings> paramBind,
        OA_ptr<CalleeToCallerVisitorIRInterface> _ir)
  :  mIR(_ir),mCall(call), mCaller(caller), 
      mInterAlias(interAlias), mParamBind(paramBind), mCallee(callee)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_CalleeToCallerVisitor:ALL", debug);

    mCallerAlias = mInterAlias->getAliasResults(caller);
}

OA_ptr<LocIterator> CalleeToCallerVisitor::getCallerLocIterator()
{
    OA_ptr<LocSetIterator> retval;
    retval = new LocSetIterator(mLocSet);
    return retval;
}


void CalleeToCallerVisitor::visitNamedLoc(NamedLoc& loc) 
{
    // should start newly empty each time
    mLocSet = new LocSet;

    // if the location is not local then just return it
    if (loc.isLocal()==false) {
        OA_ptr<Location> locCopy; locCopy = new NamedLoc(loc);
        mLocSet->insert(locCopy);
        if (debug) {
            std::cout << "CalleeToCallerVisitor: loc is not local, loc = ";
            loc.dump(std::cout);
            std::cout << std::endl;
        }
    
    }
        /*
    } else {
        // assume it is a parameter and try to get corresponding memref
        SymHandle formal = loc.getSymHandle();
        if (debug) {
            std::cout << "CalleeToCallerVisitor: formal = " << formal.hval();
            std::cout << std::endl;
        }
    
        // get the memory reference in the caller
        MemRefHandle memref = mParamBind->getCallerMemRef(mCall,formal);
    
        // only do the conversion if actual parameter is a memory reference
        if (memref!=MemRefHandle(0)) {
            // get all the may locs for the mem ref in caller
            // and insert them into the caller locs for this reference
            OA_ptr<LocIterator> mayLocIter = mCallerAlias->getMayLocs(memref);
            for ( ; mayLocIter->isValid(); (*mayLocIter)++ ) {
                if (debug) {
                    std::cout << "\tmemref = " << memref.hval()
                            << " has mayLoc ";
                    mayLocIter->current()->dump(std::cout);
                    std::cout << std::endl;
                }
                mLocSet->insert(mayLocIter->current());
            }
        }
    }
    */
}

void CalleeToCallerVisitor::visitUnnamedLoc(UnnamedLoc& loc)
{
    mLocSet = new LocSet;
    OA_ptr<Location> locptr;
    locptr = new UnnamedLoc(loc);
    mLocSet->insert(locptr);
}


/*! 
   This visitor is constructed with a particular caller actual MRE.
   It then visits the MRE for an InvisibleLoc in a callee
   to construct the corresponding MRE in the caller given that the 
   base symbol for the InvisibleLoc mre is a formal parameter and the
   caller actual MRE is one of the MREs passed in as an actual
   for the formal.
*/
class ConstructMREVisitor :  public virtual MemRefExprVisitor{

public:

  OA_ptr<MemRefExpr> getMemRefExpr() {
    return mNewMRE;
  }

  ConstructMREVisitor(OA_ptr<MemRefExpr> actualMRE, 
                      OA_ptr<CalleeToCallerVisitorIRInterface> _ir) 
      : mActualMRE(actualMRE), mIR(_ir)
  {
  }

  void visitNamedRef(NamedRef& ref) {
    // should never get here because will stop at RefOp that 
    // points at the base NamedRef
  }

  void visitUnnamedRef(UnnamedRef& ref) {
    // should never get here because the base ref in an InvisibleLoc 
    // is always a NamedRef
  }

  void visitUnknownRef(UnknownRef& ref) {
    // should never get here because the base ref in an InvisibleLoc is 
    // always a NamedRef
  }

  void visitAddressOf(AddressOf& ref) {


    OA_ptr<AddressOf> refOp;

    refOp = new AddressOf(ref);

    mRefOpStack.push(refOp);

    // see if this is the last RefOp before the NamedRef to the formal parameter
    OA_ptr<MemRefExpr> mre = ref.getMemRefExpr();
    if (mre->isaNamed()) {

      // if it is then call the method that will create the newMRE
      createNewMRE();

      if (debug) {
        std::cout << "\t Immediately after mActualMRE = ";
        mActualMRE->output(*mIR);
      }
      if (debug) {
       std::cout << "\t Immediately after mNewMRE = ";
       mNewMRE->output(*mIR);
      }

    } else {
      mre->acceptVisitor(*this);
    }

    if (debug) {
          std::cout << "\t Inside (end) of visitAddressOf mNewMRE = ";
          mNewMRE->output(*mIR);
    }
    
  }


  void visitDeref(Deref& ref) {
    // push empty version of this kind of RefOp onto stack
    OA_ptr<MemRefExpr> nullMRE;
    // will need to write getMRType()

    /* PLM 1/23/07 deprecated accuracy field    
    OA_ptr<Deref> refOp;

    refOp = 
        new Deref(ref.hasFullAccuracy(), ref.getMRType(), 
                  nullMRE, ref.getNumDerefs()) ;
    */

    
    
    OA_ptr<Deref> refOp;
    
    refOp =
        new Deref(ref.getMRType(), nullMRE, ref.getNumDerefs()) ;
   

    mRefOpStack.push(refOp);

    // see if this is the last RefOp before the NamedRef to the formal parameter
    OA_ptr<MemRefExpr> mre = ref.getMemRefExpr();
    if (mre->isaNamed()) {

      // if it is then call the method that will create the newMRE
      createNewMRE();
      
      if (debug) {
        std::cout << "\t Immediately after mActualMRE = ";
        mActualMRE->output(*mIR);
      }
      if (debug) {
	   std::cout << "\t Immediately after mNewMRE = ";
	   mNewMRE->output(*mIR);
      }
      
    } else {
      mre->acceptVisitor(*this);
    }
    
    if (debug) {
          std::cout << "\t Inside visiDeref mNewMRE = ";
          mNewMRE->output(*mIR);
    }
  }

  void visitSubSetRef(SubSetRef& ref) {
      //assert(0);  // not implemented yet, MMS
      
       OA::OA_ptr<OA::SubSetRef> subset_mre;
        OA::OA_ptr<OA::MemRefExpr> nullMRE;

        subset_mre = new OA::SubSetRef(
                                   OA::MemRefExpr::USE,
                                   nullMRE
                                  );

        mRefOpStack.push(subset_mre);

        // see if this is the last RefOp before the NamedRef to the formal parameter
        OA_ptr<MemRefExpr> mre = ref.getMemRefExpr();
        if (mre->isaNamed()) {
            // if it is then call the method that will create the newMRE
            createNewMRE();
        } else {
            mre->acceptVisitor(*this);
        }
  }

private:

  void createNewMRE()
  {
    mNewMRE = mActualMRE;


    // pop things on the list one at a time, 
    // for each RefOp, create one of the same kind but have 
    // it point to mNewMRE and then assign newly constructed MRE to mNewMRE
    while(!mRefOpStack.empty())
    { 
        OA_ptr<RefOp> refop = mRefOpStack.top(); 
        mRefOpStack.pop();  
        

        mNewMRE = refop->composeWith(mNewMRE);

        
	    if (debug) {
            std::cout << "\t Inside createNewMRE mNewMRE = ";
	        mNewMRE->output(*mIR);
        }
	}
  }

private:
  std::stack<OA_ptr<RefOp> > mRefOpStack;
  OA_ptr<MemRefExpr> mActualMRE;
  OA_ptr<MemRefExpr> mNewMRE;
  OA_ptr<CalleeToCallerVisitorIRInterface> mIR; 


};


void CalleeToCallerVisitor::visitInvisibleLoc(InvisibleLoc& loc)
{
  
  if (debug) {
      std::cout << "In CalleeToCallerVisitor::visitInvisibleLoc" << std::endl;
  }
  mLocSet = new LocSet;
  
  // if base symbols is not local
  // InvisibleLoc
  
  
  SymHandle baseSym = loc.getBaseSym();
 
 
  OA_ptr<Location> baseSymLoc = mIR->getLocation(mCallee, baseSym); 
  if (!baseSymLoc->isLocal()) 	
  {   
   // pull out the MRE
    OA_ptr<MemRefExpr> memref = loc.getMemRefExpr();  
    if (debug) {
      std::cout << "\tBaseSymLoc is NOT local = ";
      memref->output(*mIR);
    }
    
    // ask caller alias analysis to getMayLocs
    // insert each of the may locs into mLocSet
    OA_ptr<LocIterator> mayLocIter = mCallerAlias->getMayLocs(*memref, mCaller);

    for ( ; mayLocIter->isValid(); (*mayLocIter)++ ) {
      mLocSet->insert(mayLocIter->current());
    }

  // base symbol is local
  // the base symbol should be a parameter
  } else {
    OA_ptr<MemRefExpr> mre = loc.getMemRefExpr();
    if (debug) {
      std::cout << "\tBaseSymLoc IS local = ";
      mre->output(*mIR);
    }
    
    // get the base symbol for the mre within the invisible location
    // it should be a formal
    OA_ptr<RefOp> refop;
    if (mre->isaRefOp()) {
        refop = mre.convert<RefOp>();
    } else { 
        assert(0);  // should not have an MRE in InvLoc that isn't a RefOp
    }
    SymHandle formal = refop->getBaseSym();
    
    // get the memory reference in the caller
    MemRefHandle memref = mParamBind->getCallerMemRef(mCall,formal);

    // iterate over all mres for the actual memory reference in caller
    OA_ptr<MemRefExprIterator> mreIterPtr
        = mIR->getMemRefExprIterator(memref);
    for (; mreIterPtr->isValid(); (*mreIterPtr)++) {
        OA_ptr<OA::MemRefExpr> actual_mre = mreIterPtr->current();

        // create a visitor parameterized with mre in caller      
        ConstructMREVisitor visitor(actual_mre, mIR);    

	    if (debug) {
	      std::cout << "\tActual mre = ";
	      actual_mre->output(*mIR);
	    }
	 
        // visit the mre in the callee
        mre->acceptVisitor(visitor);
        OA_ptr<MemRefExpr> composed_mre = visitor.getMemRefExpr();

        if (debug) {
	      std::cout << "\tComposed mre = ";
	      composed_mre->output(*mIR);
	    }
	 
	
        // visit the may locs for resulting mre
        OA_ptr<LocIterator> mayLocIter 
            = mCallerAlias->getMayLocs(*composed_mre, mCaller);
        if (debug) {
          std::cout << "\tConstructing mayLocs from Caller Proc " << mIR->toString(mCaller)
                    << " for Composed mre:\n";
        }
        for ( ; mayLocIter->isValid(); (*mayLocIter)++ ) {	 
          mLocSet->insert(mayLocIter->current());
          if (debug) {
            std::cout << "\t\tinserted==> ";
            (mayLocIter->current())->output(*mIR);
          }
        }         
    }
  }
}

void CalleeToCallerVisitor::visitUnknownLoc(UnknownLoc& loc)
{
    mLocSet = new LocSet;
    OA_ptr<Location> locptr;
    locptr = new UnknownLoc();
    mLocSet->insert(locptr);
}

void CalleeToCallerVisitor::visitLocSubSet(LocSubSet& loc)
{
    // not handling any of the more precise LocSubSet classes
    // just going to make a generic LocSubSet to wrap around
    // all the NamedLoc's the underlying location maps to
    OA_ptr<Location> baseLoc = loc.getLoc();
    if (debug) {
        std::cout << "CalleeToCallerVisitor::visitLocSubSet" << std::endl;
        std::cout << "\tbaseLoc = ";
        baseLoc->dump(std::cout);
    }
    baseLoc->acceptVisitor(*this);
    OA_ptr<LocSet> temp = mLocSet;
    mLocSet = new LocSet;
    OA_ptr<LocIterator> locIter; locIter = new LocSetIterator(temp);
    for ( ; locIter->isValid(); (*locIter)++ ) {
        OA_ptr<Location> mayLoc = locIter->current();
        if (debug) {
            std::cout << "\t\tmayLoc = ";
            mayLoc->dump(std::cout);
        }
        
        // the constructor for LocSubSet handles various mayLoc
        // subclasses
        OA_ptr<Location> subLoc;
        subLoc = new LocSubSet(mayLoc);
        mLocSet->insert(subLoc);
    }

}


  } // end of DataFlow namespace
} // end of OA namespace








/*
class ConstructMREVisitor {

public:

  ConstructMREVisitor(OA_ptr<MemRefExpr> actualMRE) : mActualMRE(actualMRE)
  {
    mMREStack = new std::stack<OA_ptr<MemRefExpr> >;
  }
  
  void visitNamedRef(NamedRef& ref) {
    // should never get here because will stop at RefOp that points at the base NamedRef
      }
  
   void visitUnnamedRef(UnnamedRef& ref) {
    // should never get here because the base ref in an InvisibleLoc is always a NamedRef
      }
  
  void visitUnknownRef(UnknownRef& ref) {
    // should never get here because the base ref in an InvisibleLoc is always a NamedRef
  }

  void visitDeref(Deref& ref)
  {
    // push empty version of this kind of RefOp onto stack
    OA_ptr<MemRefExpr> nullMRE;
    // will need to write getMRType()
    OA_ptr<MemRefExpr> refOp = new RefOp(ref.hasAddressTaken(), ref.hasFullAccuracy(), ref.getMRType(), nullMRE );
    mRefOpStack.push(refOp);

    // see if this is the last RefOp before the NamedRef to the formal parameter
    OA_ptr<MemRefExpr> mre = ref.getMemRefExpr();
    if (mre->isaNamed()) {

         // if it is then call the method that will create the newMRE 
      // createNewMRE();
    }else {
      mre->acceptVisitor(*this);
    }
   }

  void visitSubSetRef(SubSetRef& ref) { 

  }  

private:


      void createNewMRE()
    {
      OA_ptr<MemRefExpr>mNewMRE = mActualMRE;
      // pop things on the list one at a time, for each RefOp, create one of the same kind but have it point to mNewMRE and then assign newly constructed MRE to mNewMRE 

      OA_ptr<MemRefExprIterator> mreIterPtr
            = mIR->getMemRefExprIterator(mRefOpStack);

      OA_ptr<OA::MemRefExpr> mre;
      for (; mreIterPtr->isValid(); (*mreIterPtr)++) {
           mre = mreIterPtr->current();
	   mre.push(mNewMRE);
           mNewMRE = mre;         
	   }



       if (mre->isaDeref()) {

           MRE = new Deref(mre->hasAddressTaken, ..., mNewMRE);

       } else if isaSubsetRef   {

           MRE = new Deref(mre->hasAddressTaken, ..., mNewMRE);
       }

       }
private:

  std::stack<OA_ptr<MemRefExpr> > mRefOpStack;
  OA_ptr<MemRefExpr>mActualMRE;
  OA_ptr<MemRefExpr> mNewMRE;

} */


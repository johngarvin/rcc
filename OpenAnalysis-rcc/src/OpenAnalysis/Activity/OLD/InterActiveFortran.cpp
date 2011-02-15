/*! \file
  
  \brief Implementation of InterActive

  \author Michelle Strout
  \version $Id: InterActive.cpp,v 1.6.6.1 2005/08/23 18:19:14 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "InterActiveFortran.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace Activity {

InterActiveFortran::InterActiveFortran()
{
    mUnknownLocActive = false;
    mNumIterUseful = 0;
    mNumIterVary = 0;
    mSizeInBytes = 0;
}


InterActiveFortran::~InterActiveFortran() 
{
}


//! Return an iterator over all procedures for which activity
//! analysis is available
OA_ptr<ProcHandleIterator> InterActiveFortran::getKnownProcIterator()
{         
    // create a procSet
    OA_ptr<std::set<ProcHandle> > procSet;
    procSet = new std::set<ProcHandle>;
    
    // loop through map to add to list of ProcHandle's
    std::map<ProcHandle,OA_ptr<Activity::ActiveStandard> >::iterator mIter;
    for (mIter=mProcToActiveMap.begin(); mIter != mProcToActiveMap.end(); mIter++)
    {
        procSet->insert(mIter->first);
    }

    OA_ptr<ActivityProcIter> retval;
    retval = new ActivityProcIter(procSet);
    return retval;
}

//! Indicate whether the given symbol is active or not within any procedure
bool InterActiveFortran::isActive(SymHandle sym)
{
  // an unknown location is active, therefore all symbols are active
  if (mUnknownLocActive) {
    return true;
  } else if (mActiveSymSet.find(sym) != mActiveSymSet.end()) {
    return true;
  } else {
    return false;
  }  
}


//! Indicate whether the given stmt is active or not within given proc
bool InterActiveFortran::isActive(ProcHandle proc, StmtHandle stmt)
{
    if (mProcToActiveMap[proc].ptrEqual(0)) {
      assert(0);  // we don't know about this proc
    } else {
      return mProcToActiveMap[proc]->isActive(stmt);
    }
    return false; // default return to make compilers happy
}

//! Indicate whether the given memref is active or not within given proc
bool InterActiveFortran::isActive(ProcHandle proc, MemRefHandle memref)
{
    if (mProcToActiveMap[proc].ptrEqual(0)) {
      assert(0);  // we don't know about this proc
    } else {
      return mProcToActiveMap[proc]->isActive(memref);
    }
    return false; // default return to make compilers happy
}



void InterActiveFortran::setActive(SymHandle sym) 
{
    mActiveSymSet.insert(sym); 
}



//! Return an iterator for set of active symbols
OA_ptr<SymHandleIterator> InterActiveFortran::getActiveSymIterator()
{
  OA_ptr<SymHandleIterator> retval;
  OA_ptr<std::set<SymHandle> > retSet;
  retSet = new std::set<SymHandle>;
  std::set<SymHandle>::iterator symIter;
  for (symIter = mActiveSymSet.begin(); symIter!=mActiveSymSet.end();
       symIter++)
  {
    retSet->insert(*symIter);
  }
  retval = new ActiveSymIterator(retSet);
  return retval;
}



OA_ptr<MemRefHandleIterator>
InterActiveFortran::getActiveMemRefIterator(ProcHandle proc)
{
    OA_ptr<MemRefHandleIterator> retval;

    if (mProcToActiveMap[proc].ptrEqual(0)) {
      assert(0);  // we don't know about this proc
    } else {
      retval = mProcToActiveMap[proc]->getActiveMemRefIterator();
    }

    return retval;
}


OA_ptr<StmtHandleIterator>
InterActiveFortran::getActiveStmtIterator(ProcHandle proc)
{
    OA_ptr<StmtHandleIterator> retval;

    if (mProcToActiveMap[proc].ptrEqual(0)) {
      assert(0);  // we don't know about this proc
    } else {
      retval = mProcToActiveMap[proc]->getActiveStmtIterator();
    }

    return retval;
}


//! Associate the given procedure with the given Activity info
void InterActiveFortran::mapProcToActive(ProcHandle proc, 
    OA_ptr<Activity::ActiveStandard> active)
{
    mProcToActiveMap[proc] = active;

    // get all known active symbols from the procedure so we know all active
    // symbols in program
    OA_ptr<SymHandleIterator> symIter = active->getActiveSymIterator();
    for ( ; symIter->isValid(); (*symIter)++) {
        mActiveSymSet.insert(symIter->current());
    }
 
    // if the procedure has an UnknownLoc that is active then must indicate
    // that for whole program
    if (active->getUnknownLocActive()==true) {
        mUnknownLocActive = true;
    }
}


//! Return the number of iterations over nodes in Useful
int InterActiveFortran::getNumIterUseful() 
{
    return mNumIterUseful; 
}

//! Return the number of iterations over nodes in Vary
int InterActiveFortran::getNumIterVary() 
{
    return mNumIterVary; 
}

//! Return the number of iterations over nodes in Active
int InterActiveFortran::getNumIterActive()
{
  return mNumIterActive;
}

//! Return the size of active symbols in bytes
int InterActiveFortran::getActiveSizeInBytes() 
{
     return mSizeInBytes; 
}

//! Set the number of iterations over nodes in Useful
void InterActiveFortran::setNumIterUseful(int n) 
{
     mNumIterUseful = n;
}

//! Set the number of iterations over nodes in Useful
void InterActiveFortran::setNumIterVary(int n) 
{
     mNumIterVary = n;
}

//! Set the number of iterations over nodes in Active
void InterActiveFortran::setNumIterActive(int n)
{
  mNumIterActive = n;
}

//! Set the size of active symbols in bytes
void InterActiveFortran::setActiveSizeInBytes(int size) 
{ 
     mSizeInBytes = size; 
}


//*****************************************************************
// Annotation Interface
//*****************************************************************
void InterActiveFortran::output(IRHandlesIRInterface &ir){

  sOutBuild->objStart("InterActiveFortran"); {

    // Loop through all known procedures
    OA_ptr<ProcHandleIterator> procIterPtr = getKnownProcIterator();
    for (; procIterPtr->isValid(); (*procIterPtr)++) {
      ProcHandle proc = procIterPtr->current();
      ostringstream oss;
      oss << "Procedure( " << ir.toString(proc) << " )";
      sOutBuild->fieldStart(oss.str()); {

       sOutBuild->objStart("ActiveStmtSet"); {
          sOutBuild->listStart(); {
            ostringstream oss;
            oss << indt;
            OA_ptr<StmtHandleIterator> stmtIterPtr;
            stmtIterPtr= getActiveStmtIterator(proc);
            for ( ; stmtIterPtr->isValid(); (*stmtIterPtr)++ ) {
              sOutBuild->outputString(oss.str());
              sOutBuild->listItemStart(); {
                sOutBuild->outputIRHandle(stmtIterPtr->current(),ir);
              }sOutBuild->listItemEnd();
            }
          }sOutBuild->listEnd();
        }sOutBuild->objEnd("ActiveStmtSet");


        sOutBuild->objStart("ActiveMemRefSet"); {
          sOutBuild->listStart(); {
            ostringstream oss;
            oss << indt;
            OA_ptr<MemRefHandleIterator> memrefIterPtr;
            memrefIterPtr = getActiveMemRefIterator(proc);
            for ( ; memrefIterPtr->isValid(); (*memrefIterPtr)++ ) {
              sOutBuild->outputString(oss.str());
              sOutBuild->listItemStart(); {
                sOutBuild->outputIRHandle(memrefIterPtr->current(),ir);
              }sOutBuild->listItemEnd();
            }
          }sOutBuild->listEnd();
        }sOutBuild->objEnd("ActiveMemRefSet");

      }sOutBuild->fieldEnd("Procedure");
    }

    sOutBuild->fieldStart("ActiveSymSet"); {
      sOutBuild->listStart(); {
        ostringstream oss;
        oss << indt;
        std::set<SymHandle>::iterator symIter;
        for (symIter = mActiveSymSet.begin(); symIter!=mActiveSymSet.end();
             symIter++) 
          {
            sOutBuild->outputString(oss.str());
            sOutBuild->listItemStart(); {
              sOutBuild->outputIRHandle(*symIter,ir);
            }sOutBuild->listItemEnd();
          }
      }sOutBuild->listEnd();
    }sOutBuild->fieldEnd("ActiveSymSet");
    sOutBuild->fieldStart("mNumIterUseful"); {
      ostringstream oss; oss << mNumIterUseful;
      sOutBuild->outputString(oss.str());
    }sOutBuild->fieldEnd("mNumIterUseful");
    sOutBuild->fieldStart("mNumIterVary"); {
      ostringstream oss; oss << mNumIterVary;
      sOutBuild->outputString(oss.str());
    }sOutBuild->fieldEnd("mNumIterVary");
    
  } sOutBuild->objEnd("InterActiveFortran");
  
}

//! incomplete output of info for debugging
void InterActiveFortran::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    std::cout << "====================== InterActiveFortran" << std::endl;

    os << "\tActiveSymList" << std::endl;
    std::set<SymHandle>::iterator symIter;
    for (symIter = mActiveSymSet.begin(); symIter!=mActiveSymSet.end();
         symIter++) 
    {
        os << "\t\t" << ir->toString(*symIter) << " hval = " 
           << (*symIter).hval() 
           << ", active = " << isActive(*symIter) << std::endl;
    }

    os << "\tgetActiveSizeInBytes() = " << getActiveSizeInBytes() << std::endl;

}


  } // end of Activity namespace
} // end of OA namespace


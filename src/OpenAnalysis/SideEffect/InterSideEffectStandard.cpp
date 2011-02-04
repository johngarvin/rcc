/*! \file
  
  \brief Implementation of SideEffect::InterSideEffectStandard

  \author Michelle Strout, Andy Stone (alias tag update)
  \version $Id: InterSideEffectStandard.cpp,v 1.9 2005/08/08 20:03:52 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "InterSideEffectStandard.hpp"
#include "SideEffectStandard.hpp"

namespace OA {
  namespace SideEffect{

using namespace std;
using namespace Alias;

static bool debug = false;

//*****************************************************************
// Interface Implementation
//*****************************************************************

InterSideEffectStandard::InterSideEffectStandard(
    OA_ptr<Alias::InterAliasInterface> alias)
    :
    mAlias(alias)
{ 
  // TODO: Priya, we need to figure out how to handle this case
  OA_ptr<Alias::Interface> nullInter;
  mDefaultSideEffect = new SideEffectStandard(nullInter);
}

//! Return an iterator over all procedures for which interprocedural
//! side effect information is available 
OA_ptr<ProcIterator> InterSideEffectStandard::getKnownProcIterator()
{
  // create a procSet
  OA_ptr<set<ProcHandle> > procSet;
  procSet = new set<ProcHandle>;
  
  // loop through all maps to add to list of ProcHandle's
  ProcToSideEffectMap::iterator mIter;
  for (mIter=mProcToSideEffectMap.begin(); 
  mIter != mProcToSideEffectMap.end(); mIter++) 
  {
    procSet->insert(mIter->first);
  }
  
  OA_ptr<InterSideEffectProcIter> retval;
  retval = new InterSideEffectProcIter(procSet);
  return retval;
}


//*************************************************************************

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getLMODIterator(CallHandle call)
{
  if (mCallToSideEffectMap[call].ptrEqual(0)) {
    return mDefaultSideEffect->getLMODIterator();
  } else {
    return mCallToSideEffectMap[call]->getLMODIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getMODIterator(CallHandle call)
{
  if (mCallToSideEffectMap[call].ptrEqual(0)) {
    return mDefaultSideEffect->getMODIterator();
  } else {
    return mCallToSideEffectMap[call]->getMODIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getLDEFIterator(CallHandle call)
{
  if (mCallToSideEffectMap[call].ptrEqual(0)) {
    return mDefaultSideEffect->getLDEFIterator();
  } else {
    return mCallToSideEffectMap[call]->getLDEFIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getDEFIterator(CallHandle call)
{
  if (mCallToSideEffectMap[call].ptrEqual(0)) {
    return mDefaultSideEffect->getDEFIterator();
  } else {
    return mCallToSideEffectMap[call]->getDEFIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getLUSEIterator(CallHandle call)
{
  if (mCallToSideEffectMap[call].ptrEqual(0)) {
    return mDefaultSideEffect->getLUSEIterator();
  } else {
    return mCallToSideEffectMap[call]->getLUSEIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getUSEIterator(CallHandle call)
{
  if (mCallToSideEffectMap[call].ptrEqual(0)) {
    return mDefaultSideEffect->getUSEIterator();
  } else {
    return mCallToSideEffectMap[call]->getUSEIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getLREFIterator(CallHandle call)
{
  if (mCallToSideEffectMap[call].ptrEqual(0)) {
    return mDefaultSideEffect->getLREFIterator();
  } else {
    return mCallToSideEffectMap[call]->getLREFIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getREFIterator(CallHandle call)
{
  if (mCallToSideEffectMap[call].ptrEqual(0)) {
    return mDefaultSideEffect->getREFIterator();
  } else {
    return mCallToSideEffectMap[call]->getREFIterator(); 
  }
}

//*************************************************************************

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getLMODIterator(ProcHandle p)
{
  if (mProcToSideEffectMap[p].ptrEqual(0)) {
    return mDefaultSideEffect->getLMODIterator();
  } else {
    return mProcToSideEffectMap[p]->getLMODIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getMODIterator(ProcHandle p)
{
  if (mProcToSideEffectMap[p].ptrEqual(0)) {
    return mDefaultSideEffect->getMODIterator();
  } else {
    return mProcToSideEffectMap[p]->getMODIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getLDEFIterator(ProcHandle p)
{
  if (mProcToSideEffectMap[p].ptrEqual(0)) {
    return mDefaultSideEffect->getLDEFIterator();
  } else {
    return mProcToSideEffectMap[p]->getLDEFIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getDEFIterator(ProcHandle p)
{
  if (mProcToSideEffectMap[p].ptrEqual(0)) {
    return mDefaultSideEffect->getDEFIterator();
  } else {
    return mProcToSideEffectMap[p]->getDEFIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getLUSEIterator(ProcHandle p)
{
  if (mProcToSideEffectMap[p].ptrEqual(0)) {
    return mDefaultSideEffect->getLUSEIterator();
  } else {
    return mProcToSideEffectMap[p]->getLUSEIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getUSEIterator(ProcHandle p)
{
  if (mProcToSideEffectMap[p].ptrEqual(0)) {
    return mDefaultSideEffect->getUSEIterator();
  } else {
    return mProcToSideEffectMap[p]->getUSEIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getLREFIterator(ProcHandle p)
{
  if (mProcToSideEffectMap[p].ptrEqual(0)) {
    return mDefaultSideEffect->getLREFIterator();
  } else {
    return mProcToSideEffectMap[p]->getLREFIterator(); 
  }
}

OA_ptr<AliasTagIterator> 
InterSideEffectStandard::getREFIterator(ProcHandle p)
{
  if (mProcToSideEffectMap[p].ptrEqual(0)) {
    return mDefaultSideEffect->getREFIterator();
  } else {
    return mProcToSideEffectMap[p]->getREFIterator(); 
  }
}

//*****************************************************************
// Construction methods
//*****************************************************************
//! Associate the given procedure with the given intraprocedural
//! SideEffect information
void InterSideEffectStandard::mapProcToSideEffect(
    ProcHandle proc, 
    OA_ptr<OA::SideEffect::SideEffectStandard> sideEffect)
{
  mProcToSideEffectMap[proc] = sideEffect;
}

//! Get the SideEffect information associated with the given procedure
OA_ptr<OA::SideEffect::SideEffectStandard> 
InterSideEffectStandard::getSideEffectResults(ProcHandle proc)
{
  return mProcToSideEffectMap[proc];
}

//! Associate the given call with the given intraprocedural
//! SideEffect information
void InterSideEffectStandard::mapCallToSideEffect(
    CallHandle call, 
    OA_ptr<OA::SideEffect::SideEffectStandard> sideEffect)
{
    if (sideEffect.ptrEqual(0) ) { cout << "sideEffect.ptrEqual(0)" << endl; }
    if (debug) {
        cout << "mapCallToSideEffect(call = " << call.hval() << endl;
    }
    mCallToSideEffectMap[call] = sideEffect;
}

int InterSideEffectStandard::getTagCount(ProcHandle proc)
{
  if (mProcToSideEffectMap[proc].ptrEqual(0)) {
    return 0;
  }
  
  int count = 0;
  OA_ptr<AliasTagIterator> iter;
  
  // LMOD
  iter = mProcToSideEffectMap[proc]->getLMODIterator();
  for (; iter->isValid(); ++(*iter) ) { count++; }
  // MOD
  iter = mProcToSideEffectMap[proc]->getMODIterator();
  for (; iter->isValid(); ++(*iter) ) { count++; }
  // LDEF
  iter = mProcToSideEffectMap[proc]->getLDEFIterator();
  for (; iter->isValid(); ++(*iter) ) { count++; }
  // DEF
  iter = mProcToSideEffectMap[proc]->getDEFIterator();
  for (; iter->isValid(); ++(*iter) ) { count++; }
  // LUSE
  iter = mProcToSideEffectMap[proc]->getLUSEIterator();
  for (; iter->isValid(); ++(*iter) ) { count++; }
  // USE
  iter = mProcToSideEffectMap[proc]->getUSEIterator();
  for (; iter->isValid(); ++(*iter) ) { count++; }
  // LREF
  iter = mProcToSideEffectMap[proc]->getLREFIterator();
  for (; iter->isValid(); ++(*iter) ) { count++; }
  // REF
  iter = mProcToSideEffectMap[proc]->getREFIterator();
  for (; iter->isValid(); ++(*iter) ) { count++; }
  
  return count;
}
    
void InterSideEffectStandard::initCallSideEffect(CallHandle call)
{
  // TODO: Figure out what interface to pass into constructor
  OA_ptr<Alias::Interface> alias;

  mCallToSideEffectMap[call] = new SideEffectStandard(alias);
  // empty out all the sets
  mCallToSideEffectMap[call]->emptyLMOD();
  mCallToSideEffectMap[call]->emptyMOD();
  mCallToSideEffectMap[call]->emptyLDEF();
  mCallToSideEffectMap[call]->emptyDEF();
  mCallToSideEffectMap[call]->emptyLUSE();
  mCallToSideEffectMap[call]->emptyUSE();
  mCallToSideEffectMap[call]->emptyLREF();
  mCallToSideEffectMap[call]->emptyREF();
}

//! Insert a tag into the LMOD set for the given call
void InterSideEffectStandard::insertLMOD(CallHandle p, AliasTag tag)
{
    mCallToSideEffectMap[p]->insertLMOD(tag);
}
    
//! Insert a tag into the MOD set for the given call
void InterSideEffectStandard::insertMOD(CallHandle p, AliasTag tag)
{
    mCallToSideEffectMap[p]->insertMOD(tag);
}
    
//! Insert a tag into the LDEF set for the given call
void InterSideEffectStandard::insertLDEF(CallHandle p, AliasTag tag)
{
    mCallToSideEffectMap[p]->insertLDEF(tag);
}
    
//! Insert a tag into the DEF set for the given call
void InterSideEffectStandard::insertDEF(CallHandle p, AliasTag tag)
{
    mCallToSideEffectMap[p]->insertDEF(tag);
}
    
//! Insert a tag into the LUSE set for the given call
void InterSideEffectStandard::insertLUSE(CallHandle p, AliasTag tag)
{
    mCallToSideEffectMap[p]->insertLUSE(tag);
}
    
//! Insert a tag into the USE set for the given call
void InterSideEffectStandard::insertUSE(CallHandle p, AliasTag tag)
{
    mCallToSideEffectMap[p]->insertUSE(tag);
}
    
//! Insert a tag into the LREF set for the given call
void InterSideEffectStandard::insertLREF(CallHandle p, AliasTag tag)
{
    mCallToSideEffectMap[p]->insertLREF(tag);
}
    
//! Insert a tag into the REF set for the given call
void InterSideEffectStandard::insertREF(CallHandle p, AliasTag tag)
{
    mCallToSideEffectMap[p]->insertREF(tag);
}
    
//*****************************************************************
// Output
//*****************************************************************
void InterSideEffectStandard::dump(
    ostream& os, 
    OA_ptr<IRHandlesIRInterface> ir)
{
  cout << "====================== InterSideEffect" << endl;
  
  // Loop through all known procedures
  OA_ptr<ProcIterator> procIterPtr = getKnownProcIterator();
  for (; procIterPtr->isValid(); (*procIterPtr)++) {
    ProcHandle proc = procIterPtr->current();
    
    // print all sets for each procedure
    cout << "Procedure( " << ir->toString(proc) << " )" << endl;
    
    OA_ptr<AliasTagIterator> tagIterPtr;
    
    cout << "\tLMOD = ";
    tagIterPtr = getLMODIterator(proc);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;

    cout << "\tMOD = ";
    tagIterPtr = getMODIterator(proc);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;
    
    cout << "\tLDEF = ";
    tagIterPtr = getLDEFIterator(proc);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;

    cout << "\tDEF = ";
    tagIterPtr = getDEFIterator(proc);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;
    
    cout << "\tLUSE = ";
    tagIterPtr = getLUSEIterator(proc);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;

    cout << "\tUSE = ";
    tagIterPtr = getUSEIterator(proc);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;
    
    cout << "\tLREF = ";
    tagIterPtr = getLREFIterator(proc);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;

    cout << "\tREF = ";
    tagIterPtr = getREFIterator(proc);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;
  }
  
  // Loop through all known call sites
  CallToSideEffectMap::iterator mapIter;
  for(mapIter =  mCallToSideEffectMap.begin();
      mapIter != mCallToSideEffectMap.end();
      mapIter++)
  {
    CallHandle call = mapIter->first;
    
    // print all sets for each procedure
    cout << "Call( " << ir->toString(call) << " )" << endl;
    
    OA_ptr<AliasTagIterator> tagIterPtr;
    
    cout << "\tLMOD = ";
    tagIterPtr = getLMODIterator(call);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }

    cout << "\tMOD = ";
    tagIterPtr = getMODIterator(call);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;
    
    cout << "\tLDEF = ";
    tagIterPtr = getLDEFIterator(call);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }

    cout << "\tDEF = ";
    tagIterPtr = getDEFIterator(call);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;
    
    cout << "\tLUSE = ";
    tagIterPtr = getLUSEIterator(call);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;

    cout << "\tUSE = ";
    tagIterPtr = getUSEIterator(call);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;
    
    cout << "\tLREF = ";
    tagIterPtr = getLREFIterator(call);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;

    cout << "\tREF = ";
    tagIterPtr = getREFIterator(call);
    for ( ; tagIterPtr->isValid(); ++(*tagIterPtr) ) {
      cout << tagIterPtr->current() << " ";
    }
    cout << endl;
  }
}

  } // end of UDDUChains namespace
} // end of OA namespace

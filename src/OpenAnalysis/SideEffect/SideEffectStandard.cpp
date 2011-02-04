/*! \file
  
  \brief Implementation of SideEffect::SideEffectStandard

  \author Michelle Strout, Andy Stone (convert to alias tags)
  \version $Id: SideEffectStandard.cpp,v 1.9 2005/03/17 21:47:46 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "SideEffectStandard.hpp"
#include <OpenAnalysis/Alias/AliasTagSet.hpp>

namespace OA {
  namespace SideEffect{

using namespace Alias;
using namespace std;

static bool debug = false;

//*****************************************************************
// Interface Implementation
//*****************************************************************


SideEffectStandard::SideEffectStandard(OA_ptr<Alias::Interface> alias)
    : mAlias(alias)
{
  OA_DEBUG_CTRL_MACRO("DEBUG_SideEffectStandard:ALL", debug);

  // instantiate all sets to initially be empty
  mLMODSet = new AliasTagSet(false);
  mMODSet  = new AliasTagSet(false);
  mLDEFSet = new AliasTagSet(false);
  mDEFSet  = new AliasTagSet(false);
  mLUSESet = new AliasTagSet(false);
  mUSESet  = new AliasTagSet(false);
  mLREFSet = new AliasTagSet(false);
  mREFSet  = new AliasTagSet(false);

}

OA_ptr<AliasTagIterator> SideEffectStandard::getLMODIterator() {
  return mLMODSet->getIterator();
}

OA_ptr<AliasTagIterator> SideEffectStandard::getMODIterator() {
  return mMODSet->getIterator();
}

OA_ptr<AliasTagIterator> SideEffectStandard::getLDEFIterator() { 
  return mLDEFSet->getIterator();
}

OA_ptr<AliasTagIterator> SideEffectStandard::getDEFIterator() {
  return mDEFSet->getIterator();
}

OA_ptr<AliasTagIterator> SideEffectStandard::getLUSEIterator() {
  return mLUSESet->getIterator();
}

OA_ptr<AliasTagIterator> SideEffectStandard::getUSEIterator() {
  return mUSESet->getIterator();
}

OA_ptr<AliasTagIterator> SideEffectStandard::getLREFIterator() {
  return mLREFSet->getIterator();
}

OA_ptr<AliasTagIterator> SideEffectStandard::getREFIterator() {
  return mREFSet->getIterator();
}



//*****************************************************************
// Interfaces that returns Memory Reference Expressions for the
// SideEffect Results.
//*****************************************************************

OA_ptr<MemRefExprIterator> SideEffectStandard::getLMODMREIterator() {

  OA_ptr<std::set<OA_ptr<MemRefExpr> > > rset;
  rset = new std::set<OA_ptr<MemRefExpr> >;

  OA_ptr<AliasTagIterator> aIter;
  aIter = mLMODSet->getIterator();
  for( ; aIter->isValid(); ++(*aIter) ) {

      OA_ptr<MemRefExprIterator> mreIter;
      mreIter = mAlias->getMemRefExprIterator(aIter->current());
      for( ; mreIter->isValid(); ++(*mreIter) ) {
          std::cout << "===== inserting into LMODMREIterator =====" << std::endl;
          rset->insert(mreIter->current());
      }

  }

  OA_ptr<MemRefExprIterator> retval;
  retval = new MemRefExprIterator(rset);
  return retval;

  //return mLMODSet->getIterator();
}
    

OA_ptr<MemRefExprIterator> SideEffectStandard::getMODMREIterator() {

  OA_ptr<std::set<OA_ptr<MemRefExpr> > > rset;
  rset = new std::set<OA_ptr<MemRefExpr> >;

  OA_ptr<AliasTagIterator> aIter;
  aIter = mMODSet->getIterator();
  for( ; aIter->isValid(); ++(*aIter) ) {

      OA_ptr<MemRefExprIterator> mreIter;
      mreIter = mAlias->getMemRefExprIterator(aIter->current());
      for( ; mreIter->isValid(); ++(*mreIter) ) {

          std::cout << "===== inserting into MODMREIterator =====" << std::endl;
          rset->insert(mreIter->current());
      }

  }

  OA_ptr<MemRefExprIterator> retval;
  retval = new MemRefExprIterator(rset);
  return retval;

  //return mMODSet->getIterator();
}

OA_ptr<MemRefExprIterator> SideEffectStandard::getLDEFMREIterator() {

  OA_ptr<std::set<OA_ptr<MemRefExpr> > > rset;
  rset = new std::set<OA_ptr<MemRefExpr> >;

  OA_ptr<AliasTagIterator> aIter;
  aIter = mLDEFSet->getIterator();
  for( ; aIter->isValid(); ++(*aIter) ) {

      OA_ptr<MemRefExprIterator> mreIter;
      mreIter = mAlias->getMemRefExprIterator(aIter->current());
      for( ; mreIter->isValid(); ++(*mreIter) ) {
         
          std::cout << "===== inserting into LDEFMREIterator =====" << std::endl;
          rset->insert(mreIter->current());
      }

  }

  OA_ptr<MemRefExprIterator> retval;
  retval = new MemRefExprIterator(rset);
  return retval;

  //return mLDEFSet->getIterator();
}


OA_ptr<MemRefExprIterator> SideEffectStandard::getDEFMREIterator() {

  OA_ptr<std::set<OA_ptr<MemRefExpr> > > rset;
  rset = new std::set<OA_ptr<MemRefExpr> >;

  OA_ptr<AliasTagIterator> aIter;
  aIter = mDEFSet->getIterator();
  for( ; aIter->isValid(); ++(*aIter) ) {

      OA_ptr<MemRefExprIterator> mreIter;
      mreIter = mAlias->getMemRefExprIterator(aIter->current());
      for( ; mreIter->isValid(); ++(*mreIter) ) {

          std::cout << "inserting into DEFMREIterator" << std::endl;
          rset->insert(mreIter->current());
      }

  }

  OA_ptr<MemRefExprIterator> retval;
  retval = new MemRefExprIterator(rset);
  return retval;

  //return mDEFSet->getIterator();
}

OA_ptr<MemRefExprIterator> SideEffectStandard::getLUSEMREIterator() {

  OA_ptr<std::set<OA_ptr<MemRefExpr> > > rset;
  rset = new std::set<OA_ptr<MemRefExpr> >;

  OA_ptr<AliasTagIterator> aIter;
  aIter = mLUSESet->getIterator();
  for( ; aIter->isValid(); ++(*aIter) ) {

      OA_ptr<MemRefExprIterator> mreIter;
      mreIter = mAlias->getMemRefExprIterator(aIter->current());
      for( ; mreIter->isValid(); ++(*mreIter) ) {

          std::cout << "inserting into LUSEMREIterator" << std::endl;
          rset->insert(mreIter->current());
      }

  }

  OA_ptr<MemRefExprIterator> retval;
  retval = new MemRefExprIterator(rset);
  return retval;

  //return mLUSESet->getIterator();
}



OA_ptr<MemRefExprIterator> SideEffectStandard::getUSEMREIterator() {

  OA_ptr<std::set<OA_ptr<MemRefExpr> > > rset;
  rset = new std::set<OA_ptr<MemRefExpr> >;

  OA_ptr<AliasTagIterator> aIter;
  aIter = mUSESet->getIterator();
  for( ; aIter->isValid(); ++(*aIter) ) {

      OA_ptr<MemRefExprIterator> mreIter;
      mreIter = mAlias->getMemRefExprIterator(aIter->current());
      for( ; mreIter->isValid(); ++(*mreIter) ) {

          std::cout << "inserting into USEMREIterator" << std::endl;
          rset->insert(mreIter->current());
      }

  }

  OA_ptr<MemRefExprIterator> retval;
  retval = new MemRefExprIterator(rset);
  return retval;

  //return mUSESet->getIterator();
}

OA_ptr<MemRefExprIterator> SideEffectStandard::getLREFMREIterator() {

  OA_ptr<std::set<OA_ptr<MemRefExpr> > > rset;
  rset = new std::set<OA_ptr<MemRefExpr> >;

  OA_ptr<AliasTagIterator> aIter;
  aIter = mLREFSet->getIterator();
  for( ; aIter->isValid(); ++(*aIter) ) {

      OA_ptr<MemRefExprIterator> mreIter;
      mreIter = mAlias->getMemRefExprIterator(aIter->current());
      for( ; mreIter->isValid(); ++(*mreIter) ) {

          std::cout << "inserting into LREFMREIterator" << std::endl;
          rset->insert(mreIter->current());
      }

  }

  OA_ptr<MemRefExprIterator> retval;
  retval = new MemRefExprIterator(rset);
  return retval;

  //return mLREFSet->getIterator();
}



OA_ptr<MemRefExprIterator> SideEffectStandard::getREFMREIterator() {


  OA_ptr<std::set<OA_ptr<MemRefExpr> > > rset;
  rset = new std::set<OA_ptr<MemRefExpr> >;

  OA_ptr<AliasTagIterator> aIter;
  aIter = mREFSet->getIterator();
  for( ; aIter->isValid(); ++(*aIter) ) {

      OA_ptr<MemRefExprIterator> mreIter;
      mreIter = mAlias->getMemRefExprIterator(aIter->current());
      for( ; mreIter->isValid(); ++(*mreIter) ) {

          std::cout << "inserting into MREIterator" << std::endl;
          rset->insert(mreIter->current());
      }

  }

  OA_ptr<MemRefExprIterator> retval;
  retval = new MemRefExprIterator(rset);
  return retval;

  //return mREFSet->getIterator();
}



//*****************************************************************
// Other informational methods
//*****************************************************************
bool SideEffectStandard::inLMOD(Alias::AliasTag tag) {
  return mLMODSet->contains(tag);
}

bool SideEffectStandard::inMOD(Alias::AliasTag tag) {
  return mMODSet->contains(tag);
}

bool SideEffectStandard::inLDEF(Alias::AliasTag tag) {
  return mLDEFSet->contains(tag);
}

bool SideEffectStandard::inDEF(Alias::AliasTag tag) {
  return mLDEFSet->contains(tag);
}

bool SideEffectStandard::inLUSE(Alias::AliasTag tag) {
  return mLDEFSet->contains(tag);
}

bool SideEffectStandard::inUSE(Alias::AliasTag tag) {
  return mLDEFSet->contains(tag);
}

bool SideEffectStandard::inLREF(Alias::AliasTag tag) {
  return mLDEFSet->contains(tag);
}

bool SideEffectStandard::inREF(Alias::AliasTag tag) {
  return mLDEFSet->contains(tag);
}

int SideEffectStandard::countLMOD() {
  return mLMODSet->size();
}

int SideEffectStandard::countMOD() {
  return mMODSet->size();
}

int SideEffectStandard::countLDEF() {
  return mLDEFSet->size();
}

int SideEffectStandard::countDEF() {
  return mDEFSet->size();
}

int SideEffectStandard::countLUSE() {
  return mLUSESet->size();
}

int SideEffectStandard::countUSE() {
  return mUSESet->size();
}

int SideEffectStandard::countLREF() {
  return mLREFSet->size();
}

int SideEffectStandard::countREF() {
  return mREFSet->size();
}


//*****************************************************************
// Construction methods
//*****************************************************************
//! Insert a tag into the LMOD set 
void SideEffectStandard::insertLMOD(Alias::AliasTag tag)
{
  mLMODSet->insert(tag);
}
    
//! Insert a tag into the MOD set 
void SideEffectStandard::insertMOD(Alias::AliasTag tag)
{
  if (debug) {
    cout << "SideEffectStandard::insertMOD: tag = ";
    cout << tag << endl;
  }
  mMODSet->insert(tag);
}
    
//! Insert a tag into the LDEF set 
void SideEffectStandard::insertLDEF(Alias::AliasTag tag)
{
  mLDEFSet->insert(tag);
}
    
//! Insert a tag into the DEF set 
void SideEffectStandard::insertDEF(Alias::AliasTag tag)
{
  mDEFSet->insert(tag);
}
    
//! Insert a tag into the LUSE set 
void SideEffectStandard::insertLUSE(Alias::AliasTag tag)
{
  if (debug) {
      cout << "LUseLocation insert = " << tag << endl;
  }
  
  mLUSESet->insert(tag);
}
    
//! Insert a tag into the USE set 
void SideEffectStandard::insertUSE(Alias::AliasTag tag)
{
  if (debug) {
    cout << "UseLocation insert = " << tag << endl;
  }

  mUSESet->insert(tag);
}
    
//! Insert a tag into the LREF set 
void SideEffectStandard::insertLREF(Alias::AliasTag tag)
{
  mLREFSet->insert(tag);
}
    
//! Insert a tag into the MOD set 
void SideEffectStandard::insertREF(Alias::AliasTag tag)
{
  mREFSet->insert(tag);
}
    
//*************************************************************
//! Make the LMOD set empty
void SideEffectStandard::emptyLMOD()
{
  mLMODSet->clear();
}

//! Make the MOD set empty
void SideEffectStandard::emptyMOD()
{
  mMODSet->clear();
}

//! Make the LDEF set empty
void SideEffectStandard::emptyLDEF()
{
  mLDEFSet->clear();
}
    
//! Make the DEF set empty
void SideEffectStandard::emptyDEF()
{
  mDEFSet->clear();
}
    
//! Make the LUSE set empty
void SideEffectStandard::emptyLUSE()
{
  mLUSESet->clear();
}

//! Make the USE set empty
void SideEffectStandard::emptyUSE()
{
  mUSESet->clear();
}

//! Make the LREF set empty
void SideEffectStandard::emptyLREF()
{
  mLREFSet->clear();
}

//! Make the REF set empty
void SideEffectStandard::emptyREF()
{
  mREFSet->clear();
}

//*****************************************************************
// Output
//*****************************************************************


void SideEffectStandard::dump(ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
  OA_ptr<AliasTagIterator> iter;

  cout << "====================== SideEffect" << endl;
  
  cout << "\tLMOD = ";
  iter = getLMODIterator();
  for ( ; iter->isValid(); ++(*iter) ) {
    cout << iter->current() << " ";
  }

  cout << "\tMOD = ";
  iter = getMODIterator();
  for ( ; iter->isValid(); ++(*iter) ) {
    cout << iter->current() << " ";
  }
  cout << endl;
  
  cout << "\tLDEF = ";
  iter = getLDEFIterator();
  for ( ; iter->isValid(); ++(*iter) ) {
    cout << iter->current() << " ";
  }
  cout << endl;

  cout << "\tDEF = ";
  iter = getDEFIterator();
  for ( ; iter->isValid(); ++(*iter) ) {
  }
  cout << endl;
  
  cout << "\tLUSE = ";
  iter = getLUSEIterator();
  for ( ; iter->isValid(); ++(*iter) ) {
    cout << iter->current() << " ";
  }

  cout << "\tUSE = ";
  iter = getUSEIterator();
  for ( ; iter->isValid(); ++(*iter) ) {
    cout << iter->current() << " ";
  }
  cout << endl;
  
  cout << "\tLREF = ";
  iter = getLREFIterator();
  for ( ; iter->isValid(); ++(*iter) ) {
    cout << iter->current() << " ";
  }
  cout << endl;

  cout << "\tREF = ";
  iter = getREFIterator();
  for ( ; iter->isValid(); ++(*iter) ) {
    cout << iter->current() << " ";
  }
  cout << endl;
}

  } // end of UDDUChains namespace
} // end of OA namespace

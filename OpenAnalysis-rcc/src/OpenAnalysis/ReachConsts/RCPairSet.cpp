/*! \file
  
  \brief Implementation of ReachConsts::RCPairSet

  \author Michelle Strout, Barbara Kreaseck
  \version $Id: RCPairSet.cpp,  Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/


#include "RCPairSet.hpp"
#include <OpenAnalysis/Utils/Util.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace ReachConsts {

static bool debug = false;


//! ===============================================================
//! RCPairSet methods
//! ===============================================================

RCPairSet::RCPairSet()
{
  OA_DEBUG_CTRL_MACRO("DEBUG_RCPairSet:ALL", debug);
  mSet = new OA::DataFlow::DataFlowSetImpl<RCPair>;
  mTagSet = new OA::DataFlow::DataFlowSetImpl<Alias::AliasTag>;
  mUniversal = false;
}


//! does not clone, makes a copy
RCPairSet::RCPairSet(const RCPairSet& other)
  :  mSet(other.mSet), mTagSet(other.mTagSet), mUniversal(other.mUniversal)
{
}


//! Partial-universal set constructor
RCPairSet::RCPairSet(const OA_ptr<Alias::AliasTagSet> tset)
{
  mUniversal = false;
  mSet =  new OA::DataFlow::DataFlowSetImpl<RCPair>;

  if (tset.ptrEqual(0)) {
    mTagSet = new OA::DataFlow::DataFlowSetImpl<Alias::AliasTag>;
  } else {
    OA_ptr<OA::DataFlow::DataFlowSet> dfset;
    dfset = tset->getDataFlowSet(); // this is a copy
    mTagSet = dfset.convert<OA::DataFlow::DataFlowSetImpl<Alias::AliasTag> >();
  }
}

//! Partial-universal set constructor
RCPairSet::RCPairSet(const OA_ptr<OA::DataFlow::DataFlowSetImpl<Alias::AliasTag> > iset)
{
  mUniversal = false;
  mSet =  new OA::DataFlow::DataFlowSetImpl<RCPair>; // sets universal false
  mTagSet = new OA::DataFlow::DataFlowSetImpl<Alias::AliasTag>;
  
  if (!iset.ptrEqual(0)) {
    OA_ptr<OA::DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > tagIter;
    tagIter = new OA::DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(iset);
    for (; tagIter->isValid(); (*tagIter)++) {
      mTagSet->insert(tagIter->current());
    }
  }
}

RCPairSet::~RCPairSet() { }



OA_ptr<DataFlow::DataFlowSet> RCPairSet::clone() const
{
   OA_ptr<RCPairSet> retval;
   retval = new RCPairSet();

   if (isUniversalSet()) {
     retval->setUniversal();
     return retval;
   }

  retval->mSet = ((mSet)->clone()).convert<OA::DataFlow::DataFlowSetImpl<RCPair> >();
  retval->mTagSet = ((mTagSet)->clone()).convert<OA::DataFlow::DataFlowSetImpl<Alias::AliasTag> >();

  return retval;
}

// =============================================
// ===== access for partial-universal sets =====
// =============================================

bool RCPairSet::isPartialSet() const
{
  return (mTagSet->size() > 0);
}


int RCPairSet::tagsetSize() const
{
  return (mTagSet->size());
}


OA_ptr<OA::DataFlow::DataFlowSetImplIterator<Alias::AliasTag> >
RCPairSet::getTagSetIterator() const
{
  OA_ptr<OA::DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > iter;
  iter = new OA::DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(mTagSet);
  return iter;
}


bool RCPairSet::containsPartialTag(Alias::AliasTag tag) const
{
  return (mTagSet->contains(tag));
}




bool RCPairSet::containsPairTag(Alias::AliasTag tag) const
{
  OA_ptr<OA::DataFlow::DataFlowSetImplIterator<RCPair> > defIter;
  defIter = new OA::DataFlow::DataFlowSetImplIterator<RCPair>(mSet);
  for (; defIter->isValid(); (*defIter)++) {
    if ((defIter->current()).getTag() == tag) {
      return true;
    }
  }
  return false;
}




void RCPairSet::insert(RCPair rc) { 
  // don't insert RCPairs with a NULL constValBasicInterface
  if ((rc.getConstPtr()).ptrEqual(0)) {
    return;
  }
  
  // this is not a UNION, it is an insert.  We assume that the caller
  // knows what they are doing.  Hmmm... maybe not =)
  
  if (mUniversal) {
    return;
  }
  
  if (mTagSet->contains(rc.getTag())) {
    return;
  }
  
  // insert rc into pairset
  mSet->insert(rc);
  mUniversal = false;
  
}


void RCPairSet::remove(RCPair rc) { 
  
  // erase rc from set
  mSet->remove(rc);
  
}






//! operator == for a RCPairSet CAN NOW rely upon the == operator for the
// underlying sets, because the == operator of an element of a RCPairSet, 
// namely a RCPair, considers both the AliasTags and all of the other
// fields.
bool RCPairSet::operator==(DataFlow::DataFlowSet &other) const
{
  
  // first dynamic cast to a RCPairSet, if that doesn't work then 
  // other is a different kind of DataFlowSet and *this is not equal
  RCPairSet& recastOther = dynamic_cast<RCPairSet&>(other);

  return (*this == recastOther);

  /*
  return (  (   mUniversal == recastOther.mUniversal)
            && (mSet       == recastOther.mSet)
            && (mTagSet    == recastOther.mTagSet)
            );
  */

}



bool RCPairSet::operator !=(DataFlow::DataFlowSet &other) const
{
  // first dynamic cast to a RCPairSet, if that doesn't work then 
  // other is a different kind of DataFlowSet and *this is not equal
  RCPairSet& recastOther = dynamic_cast<RCPairSet&>(other);

  return (!(*this==recastOther)); 
}



bool RCPairSet::operator==(const RCPairSet& other) const
{ 
  return (  (   mUniversal == other.mUniversal)
            && (mSet       == other.mSet)
            && (mTagSet    == other.mTagSet)
            );
}


bool RCPairSet::operator!=(const RCPairSet& other) const
{ 
  //return !(RCPairSet::operator==(const_cast<RCPairSet&>(other))); 

  return (!(*this == other));

}


void RCPairSet::setUniversal() 
{ 
  mUniversal = true;
  // with two underlying DataFlowSetImpls with their own mUniversals, 
  // we are maintaining an mUniversal within RCPairSet.
  // should there be a setNonUniversal()?  Or is it sufficient that 
  // an insert(RCPair) will change a Universal set to be a non-Universal set?

  // accordingly, we should make sure that mTagSet and mSet are empty
  if (mTagSet->size() != 0) {
    mTagSet->clear();
  }
  if (mSet->size() != 0) {
    mSet->clear();
  }
  mSet->setUniversal();

}

void RCPairSet::clear() { 
  mSet->clear();
  mTagSet->clear();
  mUniversal = false;
}


int  RCPairSet::size() const { 
  return mSet->size(); // arbitrary 
}


bool RCPairSet::isUniversalSet() const { 
  return (mUniversal);
}


bool RCPairSet::isEmpty() const { 
  return (mSet->isEmpty() && mTagSet->isEmpty() && !mUniversal); 
}

RCPairSet& RCPairSet::operator= (const RCPairSet& other)
{
  mSet = ((other.mSet)->clone()).convert<OA::DataFlow::DataFlowSetImpl<RCPair> >();
  mTagSet = ((other.mTagSet)->clone()).convert<OA::DataFlow::DataFlowSetImpl<Alias::AliasTag> >();
  mUniversal = other.mUniversal;
  return *this;
}

RCPairSet& RCPairSet::intersectEqu(RCPairSet& other)
{
  if (other.isUniversalSet()) {
    return *this;
  }
  else if (isUniversalSet()) {
    *this = other;
    return *this;
  }

  // X intersectEqu Y (where X is *this)
  //=============================================
  // X and Y are either Partial or Explicit sets
  OA_ptr<OA::DataFlow::DataFlowSetImpl<RCPair> > pairsetXonlyY;
  OA_ptr<OA::DataFlow::DataFlowSetImpl<RCPair> > pairsetYonlyX;
  OA_ptr<OA::DataFlow::DataFlowSetImplIterator<RCPair> > pIter;

  
  if (this->isPartialSet()) {
    if (other.isPartialSet()) {
      //! Partial X    intersectEqu   Partial Y
      //   ==> tagset:= X.tagset intersect Y.tagset
      //   ==> pairset:= (X.pairset intersect Y.pairset)
      //           union {<t,c>|<t,c> in X.pairset and t in Y.tagset}
      //           union {<t,c>|<t,c> in Y.pairset and t in X.tagset}
      
      // make {<t,c>|<t,c> in X.pairset and t in Y.tagset}
      pairsetXonlyY = mSet->clone().convert<OA::DataFlow::DataFlowSetImpl<RCPair> >();
      pIter = new OA::DataFlow::DataFlowSetImplIterator<RCPair>(pairsetXonlyY);
      for (; pIter->isValid(); (*pIter)++) {
        Alias::AliasTag tag = (pIter->current()).getTag();
        if (!((other.mTagSet)->contains(tag))) {
          pairsetXonlyY->remove(pIter->current());
        }
      }
      
      // make {<t,c>|<t,c> in Y.pairset and t in X.tagset}
      pairsetYonlyX = other.mSet->clone().convert<OA::DataFlow::DataFlowSetImpl<RCPair> >();
      pIter = new OA::DataFlow::DataFlowSetImplIterator<RCPair>(pairsetYonlyX);
      for (; pIter->isValid(); (*pIter)++) {
        Alias::AliasTag tag = (pIter->current()).getTag();
        if (!(mTagSet->contains(tag))) {
          pairsetYonlyX->remove(pIter->current());
        }
      }

      // make resulting pairset in *this
      mSet->intersectEqu(*(other.mSet));
      mSet->unionEqu(*pairsetXonlyY);
      mSet->unionEqu(*pairsetYonlyX);

      // make resulting tagset in *this (mTagSet could become empty)
      mTagSet->intersectEqu(*(other.mTagSet));
      
    } else {
      //! Partial X    intersectEqu   Explicit Y
      //   ==> tagset becomes empty
      //   ==> pairset:= (X.pairset intersect Y.pairset)
      //           union {<t,c>|<t,c> in Y.pairset and t in X.tagset}

      // make {<t,c>|<t,c> in Y.pairset and t in X.tagset}
      pairsetYonlyX = other.mSet->clone().convert<OA::DataFlow::DataFlowSetImpl<RCPair> >();
      pIter = new OA::DataFlow::DataFlowSetImplIterator<RCPair>(pairsetYonlyX);
      for (; pIter->isValid(); (*pIter)++) {
        Alias::AliasTag tag = (pIter->current()).getTag();
        if (!(mTagSet->contains(tag))) {
          pairsetYonlyX->remove(pIter->current());
        }
      }

      // make resulting pairset in *this
      mSet->intersectEqu(*(other.mSet));
      mSet->unionEqu(*pairsetYonlyX);

      // make resulting tagset in *this
      mTagSet->clear();

    }
  } else {
    if (other.isPartialSet()) {
      //! Explicit X   intersectEqu   Partial Y
      //   ==> tagset becomes empty
      //   ==> pairset:= (X.pairset intersect Y.pairset)
      //           union {<t,c>|<t,c> in X.pairset and t in Y.tagset}

      // make {<t,c>|<t,c> in X.pairset and t in Y.tagset}
      pairsetXonlyY = mSet->clone().convert<OA::DataFlow::DataFlowSetImpl<RCPair> >();
      pIter = new OA::DataFlow::DataFlowSetImplIterator<RCPair>(pairsetXonlyY);
      for (; pIter->isValid(); ++(*pIter)) {
        Alias::AliasTag tag = (pIter->current()).getTag();
        if (!((other.mTagSet)->contains(tag))) {
          pairsetXonlyY->remove(pIter->current());
        }
      }
      
      // make resulting pairset in *this
      mSet->intersectEqu(*(other.mSet));
      mSet->unionEqu(*pairsetXonlyY);

      // make resulting tagset in *this
      mTagSet->clear();

    } else {
      //! Explicit X   intersectEqu   Explicit Y
      //    ==> intersection of the underlying pairsets

      mSet->intersectEqu(*(other.mSet));

    }
  }
  
  return *this;

}


RCPairSet& RCPairSet::unionEqu(RCPairSet& other)
{
  if (isUniversalSet()) {
    return *this;
  } else if (other.isUniversalSet()) {
    *this = other;
    return *this;
  }

  // X unionEqu Y (where X is *this)
  //============================================
  // X and Y are either Partial or Explicit sets

  // union the underlying sets (for Explicit, tagsets will be empty, OK)
  mTagSet->unionEqu(*(other.mTagSet));
  mSet->unionEqu(*(other.mSet));

  // remove any pairset in mSet whose tag is in mTagSet
  if (mTagSet->size()!=0 && mSet->size()!=0) {
    OA_ptr<OA::DataFlow::DataFlowSetImplIterator<RCPair> > pIter;
    pIter = new OA::DataFlow::DataFlowSetImplIterator<RCPair>(mSet);
    for (; pIter->isValid(); (*pIter)++) {
      Alias::AliasTag tag = (pIter->current()).getTag();
      if ((mTagSet->contains(tag))) {
        mSet->remove(pIter->current());
      }
    }
  }
  
  return *this;

}


RCPairSet& RCPairSet::minusEqu(RCPairSet& other)
{

  // X minusEqu Universal
  // ==============================
  if (other.isUniversalSet()) {
    clear(); // should make *this empty
    return *this;
  }

  // Universal minusEqu X
  // ==============================
  // --> we shouldn't need this right now.
  // --> also, we have no way of indicating const values that are NOT
  //     in the set
  assert(  !(isUniversalSet())  );


  // X minusEqu Y   (where Y is *this)
  //=========================
  // X and Y are either Partial or Explicit

  // if X is Partial and Y has a non-empty pairset, assert for now
  assert ( !((mTagSet->size() > 0) && ((other.mSet)->size() > 0)) );
  
  // set difference of underlying pairsets
  if ((other.mSet)->size() > 0) {
    mSet->minusEqu(*(other.mSet));
  }

  if ((other.mTagSet)->size() > 0) {

    // set difference of underlying tagsets
    if (mTagSet->size() > 0) {
      mTagSet->minusEqu(*(other.mTagSet));
    }

    // remove <t,*> from mSet when t in other.mTagSet
    if (mSet->size() > 0) {
      OA_ptr<OA::DataFlow::DataFlowSetImplIterator<RCPair> > pIter;
      pIter = new OA::DataFlow::DataFlowSetImplIterator<RCPair>(mSet);
      for (; pIter->isValid(); (*pIter)++) {
        Alias::AliasTag tag = (pIter->current()).getTag();
        if ( (other.mTagSet)->contains(tag) ) {
          mSet->remove(pIter->current());
        }
      }
    }

  }

  return *this;

}


//! ================ Output =========================
void RCPairSet::output(IRHandlesIRInterface& ir) const {
     std::cout << "Please call output with alias::Interface"
               << std::endl;
     assert(0);
}



//! Return a string representing the contents of a RCPairSet

std::string RCPairSet::toString(OA_ptr<IRHandlesIRInterface> pIR){

  std::ostringstream oss;
  oss << "{ ";

  if (mUniversal) {
    oss << "isUniversal ";
  } else {
    if (mTagSet->size() > 0) {
      oss << "isPartialUniversal Tags: [";
      oss << mTagSet->toString(*pIR);
      oss << "] ";
    }
    if (mSet->size() > 0) {
      oss << "Pairs: ";
      OA_ptr<OA::DataFlow::DataFlowSetImplIterator<RCPair> > defIter;
      defIter = new OA::DataFlow::DataFlowSetImplIterator<RCPair>(mSet);
      if (defIter->isValid()) { // handle first RCPair
        oss << (defIter->current()).toString(*pIR);
        (*defIter)++;
      }
      for (; defIter->isValid(); (*defIter)++) { // handle rest
        oss << ", " << (defIter->current()).toString(*pIR);
      }
    }
  }

  oss << " }";
  return oss.str();
}


std::string RCPairSet::toString() {
  std::ostringstream oss;
  oss << "{ ";

  if (mUniversal) {
    oss << "isUniversal ";
  } else {
    if (mTagSet->size() > 0) {
      oss << "isPartialUniversal Tags: [";
      OA_ptr<OA::DataFlow::DataFlowSetImplIterator<Alias::AliasTag> > tagIter;
      tagIter = new OA::DataFlow::DataFlowSetImplIterator<Alias::AliasTag>(mTagSet);
      for (; tagIter->isValid(); (*tagIter)++) {
        //Alias::AliasTag tag = tagIter->current();
        oss << tagIter->current() << ", ";
      }
      oss << "] ";
    }
    if (mSet->size() > 0) {
      oss << "Pairs: ";
      OA_ptr<OA::DataFlow::DataFlowSetImplIterator<RCPair> > defIter;
      defIter = new OA::DataFlow::DataFlowSetImplIterator<RCPair>(mSet);
      if (defIter->isValid()) { // handle first RCPair
        oss << (defIter->current());
        (*defIter)++;
      }
      for (; defIter->isValid(); (*defIter)++) { // handle rest
        oss << ", " << (defIter->current());
      }
    }
  }
  oss << " }";

  return oss.str();
}
  




void RCPairSet::output(OA::IRHandlesIRInterface &ir,
                       Alias::Interface& alias) const
{
  sOutBuild->objStart("RCPairSet");
  
  if (mUniversal) {
    sOutBuild->field("mUniversal","TRUE");
  } else {
    if (mTagSet->size() > 0) {
      //sOutBuild->field("mPartial","TRUE");
      //std::ostringstream oss;
      //mTagSet->dump(oss,ir);
      //sOutBuild->field("mTagSet",oss.str());
      sOutBuild->field("mTagSet",mTagSet->toString(ir));
    }
    sOutBuild->listStart();
    OA_ptr<OA::DataFlow::DataFlowSetImplIterator<RCPair> > defIter;
    defIter = new OA::DataFlow::DataFlowSetImplIterator<RCPair>(mSet);
    for (; defIter->isValid(); (*defIter)++) {
        sOutBuild->listItemStart();
        sOutBuild->fieldStart("RCPair");
        (defIter->current()).output(ir,alias);
        sOutBuild->fieldEnd("RCPair");
        sOutBuild->listItemEnd();
    }
    sOutBuild->listEnd();
  }

  sOutBuild->objEnd("RCPairSet");
}



void RCPairSet::dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> pIR)
{ os << toString(pIR) << std::endl; }




//! ===============================
//! RCPairSet Iterator 
//! ===============================

RCPairSetIterator::RCPairSetIterator (RCPairSet& RCSet) 
  : mSet(RCSet.mSet)
{
  assert(!mSet.ptrEqual(NULL));
  mRCIter = new OA::DataFlow::DataFlowSetImplIterator<RCPair>(mSet);

  mUniversal = RCSet.mUniversal;
  mPartial = RCSet.isPartialSet();
}




RCPairSetIterator::~RCPairSetIterator () {}




void RCPairSetIterator::operator ++ () { 
  if (isValid()) (*mRCIter)++; 
}




//! is the iterator at the end
bool RCPairSetIterator::isValid() 
{ 
  return (mRCIter->isValid()); 
}




//! return copy of current node in iterator
RCPair RCPairSetIterator::current() 
{ 
  assert(isValid());
  return (mRCIter->current()); 
}




//! reset iterator to beginning of set
void RCPairSetIterator::reset() 
{
  mRCIter->reset();
}


//! empty sets and universal sets look the same to the iterator, so check
bool RCPairSetIterator::isUniversalSetIter()
{
  return (mUniversal);
}

//! is RCPairSet base a partial universal set?
bool RCPairSetIterator::isPartialSetIter()
{
  return (mPartial);
}



  } // end of ReachConst namespace
} // end of OA namespace


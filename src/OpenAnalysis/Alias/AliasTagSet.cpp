/*! \file
  
  \brief Contains implementation of alias-tag set class and iterator class.

  \authors Andy Stone
  \version $Id$

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <OpenAnalysis/Alias/AliasTagSet.hpp>

#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace Alias {

using namespace DataFlow;

AliasTagSet::AliasTagSet(OA_ptr<std::set<AliasTag> > tags, bool isMust) {
    mTags = new DataFlow::DataFlowSetImpl<AliasTag>(tags);
    mIsMust = isMust;
}


AliasTagSet::AliasTagSet(bool isMust)
{
    mTags = new DataFlow::DataFlowSetImpl<AliasTag>();
    mIsMust = isMust;
}

//! copy constructor
AliasTagSet::AliasTagSet(AliasTagSet& other)
{
    OA_ptr<DataFlow::DataFlowSet> genericSet = other.mTags->clone();
    mTags = genericSet.convert<DataFlow::DataFlowSetImpl<AliasTag> >();
    mIsMust = other.mIsMust;
}

//! Return a ptr to a clone
OA_ptr<AliasTagSet> AliasTagSet::clone()
{
    OA_ptr<AliasTagSet> retval;
    retval = new AliasTagSet(*this);
    return retval;
}

bool AliasTagSet::isMust() {
    return mIsMust;
}

OA_ptr<AliasTagIterator> AliasTagSet::getIterator() const {
    OA_ptr<AliasTagIterator> iter;
    iter = new AliasTagIterator(mTags);
    return iter;
}

bool AliasTagSet::contains(AliasTag tag) {
    return (*mTags).contains(tag);
}

int AliasTagSet::size() {
    return mTags->size();
}

OA_ptr<DataFlow::DataFlowSet> AliasTagSet::getDataFlowSet() {
    return mTags->clone();
}


void AliasTagSet::insert(AliasTag tag) {
    mTags->insert(tag);
}

void AliasTagSet::clear() {
    mTags->clear();
}

void AliasTagSet::output(OA::IRHandlesIRInterface& ir) const {
    sOutBuild->field("mIsMust", mIsMust ? "true" : "false");
    sOutBuild->objStart("mTags");
    mTags->output(ir);
    sOutBuild->objEnd("mTags");
}

void AliasTagSet::output(
    OA::IRHandlesIRInterface& ir,
    Alias::Interface &aliasResults) const
{

    sOutBuild->field("mIsMust", mIsMust ? "true" : "false");
    sOutBuild->objStart("mTags");
    sOutBuild->listStart();

    // iterate through each tag
    OA_ptr<AliasTagIterator> tagIter;
    tagIter = getIterator();
    for(; tagIter->isValid(); ++(*tagIter)) {

        sOutBuild->mapEntryStart();
          sOutBuild->mapKeyStart();
            tagIter->current().output(ir, aliasResults);
          sOutBuild->mapKeyEnd();

         sOutBuild->mapValueStart();
          sOutBuild->listStart();
   
          // iterate through each MRE
          OA_ptr<MemRefExprIterator> mreIter;
          mreIter = aliasResults.getMemRefExprIterator(tagIter->current());
          for(; mreIter->isValid(); ++(*mreIter)) {
              sOutBuild->listItemStart();
              mreIter->current()->output(ir);
              sOutBuild->listItemEnd();
          }

          sOutBuild->listEnd();
         sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();

    }

    sOutBuild->listEnd();
    sOutBuild->objEnd("mTags");
}

void AliasTagSet::dump(
    std::ostream& os,
    OA::IRHandlesIRInterface& ir,
    Alias::Interface &aliasResults)
{
    OA_ptr<AliasTagIterator> aliasIter;

    os << "[";
    aliasIter = getIterator();
    for(; aliasIter->isValid(); ++(*aliasIter)) {
        aliasIter->current().dump(os, ir, aliasResults);
    }
    os << "]";
}

} } // end namespaces



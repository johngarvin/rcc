/*! \file
  
  \brief Interface for alias tag sets.

  \authors Andy Stone
  \version $Id$

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef AliasTagSet_H
#define AliasTagSet_H

#include <OpenAnalysis/DataFlow/DataFlowSetImpl.hpp>
#include <OpenAnalysis/Utils/Iterator.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>
#include <OpenAnalysis/Alias/AliasTag.hpp>


namespace OA {
  namespace Alias {

class Interface;

//! Iterator over tags in an alias-tag set.
class AliasTagIterator : public Iterator {
  public:

    /*! Construct an iterator given a dataflow set of ints representing
        alias tags. */
    AliasTagIterator(OA_ptr<DataFlow::DataFlowSetImpl<AliasTag> > tags) {
        mIter = new DataFlow::DataFlowSetImplIterator<AliasTag>(tags);
    }

    /*! Construct an iterator over an empty set. (isValid() will always return
        false). */
    AliasTagIterator() {
        mIter = NULL;
    }

    //************************************************************
    // From interface
    //************************************************************
    virtual AliasTag current() {
        return mIter->current();
    }

    virtual bool isValid() const {
        if(mIter.ptrEqual(NULL)) { return false; }

        return mIter->isValid();
    }

    virtual void operator++ () {
        ++(*mIter);
    }

    void reset() { mIter->reset(); }

  private:
    OA_ptr<DataFlow::DataFlowSetImplIterator<AliasTag> > mIter;
};



//! Set of alias tags.
class AliasTagSet : public virtual Annotation {
  public:
    //************************************************************
    // Constructors / destructors
    //************************************************************
  
    /*! Construct a new alias-tag set given a set of tags and a
        may/must value */
    AliasTagSet(OA_ptr<std::set<AliasTag> > tags, bool isMust);

    /*! Construct an empty set of alias tags. */
    AliasTagSet(bool isMust);

    //! copy constructor
    AliasTagSet(AliasTagSet& other);

    //! Return a ptr to a clone
    OA_ptr<AliasTagSet> clone();

    //************************************************************
    // Queries
    //************************************************************

    //! returns true if all tags in the set are must aliases
    bool isMust();

    //! return an iterator over all tags in the set
    OA_ptr<AliasTagIterator> getIterator() const;

    //! return true if the passed tag is included in the set
    bool contains(AliasTag tag);

    //! Return the number of elements contained in this set
    int size();

    /*! Return a data-flow set containing the alias tags this set
        contains. Note that this set is a copy */
    OA_ptr<DataFlow::DataFlowSet> getDataFlowSet();

    //************************************************************
    // Modification
    //************************************************************

    //! Insert a new tag into the set
    void insert(AliasTag tag);

    //! Remove all tags from the set
    void clear();

    //************************************************************
    // Output
    //************************************************************
    virtual void output(OA::IRHandlesIRInterface& ir) const;
    virtual void output(
        OA::IRHandlesIRInterface& ir,
        Alias::Interface &aliasResults) const;
    virtual void dump(
        std::ostream& os,
        OA::IRHandlesIRInterface& ir,
        Alias::Interface &aliasResults);

  private:
    OA_ptr<DataFlow::DataFlowSetImpl<AliasTag> > mTags;
    bool mIsMust;
};




} } // end namespaces 

#endif

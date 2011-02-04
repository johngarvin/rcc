/*! \file
  
  \brief Decleration for location sub set abstraction.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef LocSubSet_H
#define LocSubSet_H

#include "Location.hpp"

namespace OA {

/*!
   The LocSubSet implements a decorator pattern for locations.
   If we decorate a location with a LocSubSet that represents some
   subset of the location depending on what subset abstraction is being used.
   Many subset abstractions are possible.
*/
class LocSubSet : public Location {
  public:
    LocSubSet(OA_ptr<Location> baseLoc);
    LocSubSet(OA_ptr<Location> baseLoc, bool full);

    //! copy constructor
    LocSubSet(LocSubSet &other) : mLoc(other.mLoc), mFull(other.mFull) {}

    virtual ~LocSubSet() { }

    virtual void acceptVisitor(LocationVisitor& pVisitor);

    //*****************************************************************
    // Location subclass type methods 
    //*****************************************************************
    bool isaSubSet() { return true; }

    //*****************************************************************
    // LocSubSet subclass type methods 
    //*****************************************************************
    //! This needs to be logical OR of all isa methods below it
    //! If a direct instance of this class then will return false
    bool isSubClassOfLocSubSet() { return isaIdxSubSet() 
                                          || isaFieldSubSet(); }

    virtual bool isaIdxSubSet() { return false; }
    virtual bool isaFieldSubSet() { return false; }

    //*****************************************************************
    // Info methods 
    //*****************************************************************
    //! LocSubSets will return the location for which they indicate subsetting.
    OA_ptr<Location> getBaseLoc() { return mLoc; }

    OA_ptr<Location> getLoc() { return mLoc; }

    //! something useful to have for all LocSubSet's
    virtual bool isFull() { return mFull; }

    bool isLocal() { assert(!mLoc.ptrEqual(NULL));  return mLoc->isLocal(); }

    bool isUnique() { assert(!mLoc.ptrEqual(NULL));  return mLoc->isUnique(); }
    // FIXME: does this make sense for a subset?

    //*****************************************************************
    // Relationship methods
    //*****************************************************************
    virtual bool operator<(Location & other);
    virtual bool operator==(Location& other);
    virtual bool mayOverlap(Location& other);

    virtual bool mustOverlap(Location& other);

    virtual bool subSetOf(Location& other);

    // *****************************************************************
    // Debugging
    // *****************************************************************
    void dump(std::ostream& os);
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR);

    std::string toString(OA_ptr<IRHandlesIRInterface> pIR);

    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir);

    virtual int getOrder() { return sOrder; }

  protected:

    OA_ptr<Location> mLoc;

  private:    
    bool mFull;

    //! helper function for constructor
    void composeWithBaseLoc(OA_ptr<Location> baseLoc);

    static const int sOrder = 600;
};

} // end namespace

#endif

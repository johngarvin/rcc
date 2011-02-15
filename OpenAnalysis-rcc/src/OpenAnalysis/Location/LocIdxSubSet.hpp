/*! \file
  
  \brief Decleration for index sub set location abstraction.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef LocIdxSubSet_H
#define LocIdxSubSet_H

#include "Location.hpp"
#include "LocSubSet.hpp"

namespace OA {

/*! The LocIdxSubSet describes a subset for an array that includes one
    constant index.  Way of representing an individual element of an array. */
class LocIdxSubSet : public LocSubSet {
  public:
    LocIdxSubSet(OA_ptr<Location> loc, int idx);

    //! copy constructor
    LocIdxSubSet(LocIdxSubSet &other) : LocSubSet(other), mIdx(other.mIdx) {}

    ~LocIdxSubSet() { }

    void acceptVisitor(LocationVisitor& pVisitor);

    //*****************************************************************
    // LocSubSet subclass type methods 
    //*****************************************************************
    bool isaIdxSubSet() { return true; }
    
    //*****************************************************************
    // Info methods 
    //*****************************************************************
    bool isFull() { return false; }

    
    /* PLM 1/23/07 deprecated hasFullAccuracy
    bool hasFullAccuracy() { return true; }
    */

    int getIdx() { return mIdx;}

    //*****************************************************************
    // Relationship methods, will be defined in subclasses 
    //*****************************************************************
    bool operator<(Location & other);
    bool operator==(Location& other);
    
    bool mayOverlap(Location& other);

    bool mustOverlap(Location& other);

    bool subSetOf(Location& other);

    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir);

    //*****************************************************************
    // Debugging
    //*****************************************************************
    void dump(std::ostream& os);
    void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR);

    std::string toString(OA_ptr<IRHandlesIRInterface> pIR);

    virtual int getOrder() { return sOrder; }

private:
    int mIdx;
    static const int sOrder = 400;
};

} // end namespace

#endif

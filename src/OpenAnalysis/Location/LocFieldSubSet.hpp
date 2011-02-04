/*! \file
  
  \brief Decleration for field sub set location abstraction.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef LocFieldSubSet_H
#define LocFieldSubSet_H

#include "LocSubSet.hpp"

namespace OA {

/*!
   The LocFieldSubSet describes a subset for a location with a field
   access. For example, the memory reference a.b could be mapped
   to LocFieldSubSet( b, NamedLoc(a) ) and the memory reference a->b
   could be mapped to LocFieldSubSet( b, UnnamedLoc(a = new A;) ).
*/
class LocFieldSubSet : public LocSubSet {
  public:
    LocFieldSubSet(OA_ptr<Location> loc, std::string fieldName)
        : LocSubSet(loc), mFieldName(fieldName) { }

    //! copy constructor
    LocFieldSubSet(LocFieldSubSet &other) 
        : LocSubSet(other), mFieldName(other.mFieldName) {}

    ~LocFieldSubSet() { }

    void acceptVisitor(LocationVisitor& pVisitor);

    //*****************************************************************
    // LocSubSet subclass type methods 
    //*****************************************************************
    bool isaFieldSubSet() { return true; }
    
    //*****************************************************************
    // Info methods 
    //*****************************************************************
    bool isFull() { return false; }
    string getFieldName() { return mFieldName;}

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
    string mFieldName;
    static const int sOrder = 500;
};

} // end namespace
#endif

/*! \file
  
  \brief Definition for unknown location, location abstraction.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef UnknownLoc_H
#define UnknownLoc_H

#include "Location.hpp"

namespace OA {

/*!
   An unknown location may overlap with any location in the whole program.
*/
class UnknownLoc: public Location {
  public:
    UnknownLoc() { }

    void acceptVisitor(LocationVisitor& pVisitor);

    //! copy constructor
    UnknownLoc(UnknownLoc &other) {}

    ~UnknownLoc() { }

    //*****************************************************************
    // Subclass type methods 
    //*****************************************************************
    bool isaUnknown() { return true; }

    //*****************************************************************
    // Info methods 
    //*****************************************************************
    //! An UnknownLoc returns itself as a base location.
    OA_ptr<Location> getBaseLoc(); 

    bool isLocal() { return false; } // actually unknown, conservative
    bool isUnique() { return false; } // actually unknown, conservative

    /* PLM 1/23/07 deprecated hasFullAccuracy
    bool hasFullAccuracy() { return false; }
    */

    //*****************************************************************
    // Relationship methods 
    //*****************************************************************
    bool operator<(Location & other);
    bool operator==(Location& other);

    bool mayOverlap(Location& other) { return true; }

    bool mustOverlap(Location& other) { return false; }

    bool subSetOf(Location& other) { return true; }

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
    static const int sOrder = 100000000;
};

}

#endif

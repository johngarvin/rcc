/*! \file
  
  \brief Definition for named location, location abstraction.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef NamedLoc_H
#define NamedLoc_H

#include "Location.hpp"

namespace OA {

/*!
   A named location has a SymHandle associated with it. 
   Examples include local and global variables.
*/
class NamedLoc: public Location {
  public:
    NamedLoc(SymHandle sh, bool isLocal) :
        mSymHandle(sh), mLocal(isLocal) { }

    //! copy constructor
    NamedLoc(NamedLoc &other); 

    ~NamedLoc() { }

    void addFullOverlap(SymHandle s) { mFullOverlap.insert(s); }
    void addPartOverlap(SymHandle s) { mPartOverlap.insert(s); }

    void acceptVisitor(LocationVisitor& pVisitor);

    //*****************************************************************
    // Subclass type methods 
    //*****************************************************************
    bool isaNamed() { return true; }

    //*****************************************************************
    // Info methods 
    //*****************************************************************
    //! A NamedLoc returns itself as a base location.
    OA_ptr<Location> getBaseLoc(); 

    bool isLocal() { return mLocal; }
    bool isUnique() { return true; }
    SymHandle getSymHandle() { return mSymHandle; }
    bool staticFullOverlap(SymHandle);
    bool staticPartOverlap(SymHandle);
    OA_ptr<SymHandleIterator> getPartOverlapIter();
    OA_ptr<SymHandleIterator> getFullOverlapIter();

    //*****************************************************************
    // Relationship methods 
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
    SymHandle mSymHandle;

    bool mLocal;
    
    // need sets for these so can use comparison of sets
    std::set<SymHandle>  mFullOverlap;
    std::set<SymHandle>  mPartOverlap;

    static const int sOrder = 100;
};

}

#endif

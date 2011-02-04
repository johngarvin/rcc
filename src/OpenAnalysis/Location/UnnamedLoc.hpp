/*! \file
  
  \brief Definition for unnamed location, location abstraction.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef UnnamedLoc_H
#define UnnamedLoc_H

#include "Location.hpp"

namespace OA {

/*!
   An unnamed location has a StmtHandle associated with it. 
   Examples include statements that call malloc or new to allocate
   blocks of memory.  
*/
class UnnamedLoc: public Location {
  public:
    UnnamedLoc(ExprHandle h, bool isLocal) : mExprHandle(h), mLocal(isLocal)
	    { }

    //! copy constructor
    UnnamedLoc(UnnamedLoc &other);

    ~UnnamedLoc() { }

    void acceptVisitor(LocationVisitor& pVisitor);

    //*****************************************************************
    // Subclass type methods 
    //*****************************************************************
    bool isaUnnamed() { return true; }

    //*****************************************************************
    // Info methods 
    //*****************************************************************
    //! An UnnamedLoc returns itself as a base location.
    OA_ptr<Location> getBaseLoc(); 

    bool isLocal() { return mLocal; }
    bool isUnique() { return false; }
    
    /* PLM 1/23/07 deprecated hasFullAccuracy
    bool hasFullAccuracy() { return false; }
    */

    ExprHandle getExprHandle() { return mExprHandle; }

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
    ExprHandle mExprHandle;
    bool mLocal;

    static const int sOrder = 200;
};

} // end namespace

#endif

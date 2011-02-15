/*! \file

  \brief Declarations for the invisible Location, location abstraction.

  \authors Michelle Strout, Andy Stone
  \version $Id: InvisibleLoc.hpp,v 1.26.6.1 2005/11/04 16:24:12 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef InvisibleLoc_H
#define InvisibleLoc_H

#include "Location.hpp"
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExprVisitor.hpp>

namespace OA {

/*! An invisible location involves a memory reference whose base is
    a param or non-local symbol.  Invisible locations are used to summarize
    information within a procedure so that callers of that procedure can
    use the summarized information. */
class InvisibleLoc : public Location {
  public:
    InvisibleLoc(OA_ptr<MemRefExpr> mre) : mMRE(mre) { }

    //! copy constructor
    InvisibleLoc(InvisibleLoc &other);

    ~InvisibleLoc() { }

    void acceptVisitor(LocationVisitor& pVisitor);

    //*****************************************************************
    // Subclass type methods 
    //*****************************************************************
    bool isaInvisible() { return true; }

    //*****************************************************************
    // Info methods 
    //*****************************************************************
    //! An InvisibleLoc returns itself as a base location.
    //! It contains an MRE, not a sub location.
    OA_ptr<Location> getBaseLoc(); 

    bool isLocal() { return false; }
    bool isUnique() { return false; } // actually unknown

    /* PLM 1/23/07 deprecated hasFullAccuracy
    bool hasFullAccuracy() { return mMRE->hasFullAccuracy(); }
    */

    OA_ptr<MemRefExpr> getMemRefExpr() { return mMRE; }
    SymHandle getBaseSym();
    
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
    OA_ptr<MemRefExpr> mMRE;

    static const int sOrder = 300;
};

} // end namespace
#endif

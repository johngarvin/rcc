/*! \file
  
  \brief Definition for unnamed location, location abstraction.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "UnnamedLoc.hpp"
#include <OpenAnalysis/Location/LocationVisitor.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {

static bool debug = false;

UnnamedLoc::UnnamedLoc(UnnamedLoc &other) : mExprHandle(other.mExprHandle),
                                            mLocal(other.mLocal)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_UnnamedLoc:ALL", debug);
}

void UnnamedLoc::acceptVisitor(LocationVisitor& pVisitor)
{
    pVisitor.visitUnnamedLoc(*this);
}

OA_ptr<Location> UnnamedLoc::getBaseLoc() 
{ 
    OA_ptr<Location> retval; 
    retval = new UnnamedLoc(*this);
    return retval;
}

/*!
   Locations are ordered first between Location subclasses 
   based on their sOrder value.
   Then there is an ordering within each subclass.

   For UnnamedLoc the ordering is based on the ExprHandle.
*/
bool UnnamedLoc::operator<(Location& other)
{
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class
    UnnamedLoc& loc = static_cast<UnnamedLoc&>(other);

    if (getExprHandle() < loc.getExprHandle()) 
    { return true; } else { return false; }
}  

bool UnnamedLoc::operator==(Location& other)
{
    if(getOrder() != other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class
    UnnamedLoc& loc = static_cast<UnnamedLoc&>(other);

    if (getExprHandle() == loc.getExprHandle()
        && loc.isLocal() == isLocal() ) 
    { return true; } else { return false; }
}   

class UnnamedLocMayOverlapVisitor : public virtual LocationVisitor {
  private:
    UnnamedLoc& mThisLoc;

  public:
    bool mMayOverlap;

    UnnamedLocMayOverlapVisitor(UnnamedLoc& thisLoc) 
        : mThisLoc(thisLoc), mMayOverlap(true) {}
    ~UnnamedLocMayOverlapVisitor() {}
    
    // we don't overlap other named locations
    void visitNamedLoc(NamedLoc& loc) { mMayOverlap = false; }

    // we don't overlap other unnamed locations allocated at different stmts
    void visitUnnamedLoc(UnnamedLoc& loc)
      { if (mThisLoc.getExprHandle() != loc.getExprHandle())
        { mMayOverlap = false; } else { mMayOverlap = true; }
      }

    // don't overlap with Invisibles because anything that does should
    // be mapped to same Invisible by alias analysis
    void visitInvisibleLoc(InvisibleLoc& loc) { mMayOverlap = false; }

    // all locs overlap with the UnknownLoc
    void visitUnknownLoc(UnknownLoc& loc) { mMayOverlap = true; }

    // if the other location is a subset and we are not then
    // make the other location do the work
    void visitLocSubSet(LocSubSet& loc) 
      { mMayOverlap = loc.mayOverlap(mThisLoc); }

};

bool UnnamedLoc::mayOverlap(Location& other)
{
    // apply the visitor to the other location and return result
    UnnamedLocMayOverlapVisitor mayOverlapVisitor(*this);
    other.acceptVisitor(mayOverlapVisitor);
    return mayOverlapVisitor.mMayOverlap;
}

bool UnnamedLoc::mustOverlap(Location& other)
{
    return false;
}

//! conservatively answers this question, if we may overlap with
//! the other location but don't must overlap then we may or may
//! not be a subset so we just return false
bool UnnamedLoc::subSetOf(Location & other)
{
    return false;
}

void UnnamedLoc::output(IRHandlesIRInterface& pIR)
{
    sOutBuild->objStart("UnnamedLoc");

    sOutBuild->fieldStart("mExprHandle");
    sOutBuild->outputIRHandle(mExprHandle,pIR);
    sOutBuild->fieldEnd("mExprHandle");

    sOutBuild->field("mLocal", bool2string(mLocal));
   
    sOutBuild->objEnd("UnnamedLoc");
}


void UnnamedLoc::dump(std::ostream& os)
{
    os << "UnnamedLoc(this=" << this << ", mExprHandle.hval()="
       << mExprHandle.hval() << ")";
    os << std::endl;
}

void UnnamedLoc::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    os << "UnnamedLoc(this=" << this << ", mExprHandle.hval()="
       << mExprHandle.hval() << "): "
       << pIR->toString(mExprHandle);
    os << std::endl;
}

std::string UnnamedLoc::toString(OA_ptr<IRHandlesIRInterface> pIR)
{
    std::ostringstream oss;
    oss << "UnnamedLoc(" << mExprHandle << "):" << pIR->toString(mExprHandle);
    return oss.str();
}

} // end namespace

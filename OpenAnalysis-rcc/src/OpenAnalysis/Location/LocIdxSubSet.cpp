/*! \file
  
  \brief Definition for index sub set location abstraction.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <OpenAnalysis/Location/LocationVisitor.hpp>
#include "LocIdxSubSet.hpp"

#include <OpenAnalysis/Utils/OutputBuilder.hpp>
namespace OA {

static bool debug = false;

LocIdxSubSet::LocIdxSubSet(OA_ptr<Location> loc, int idx) :
    LocSubSet(loc),
    mIdx(idx)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_LocIdxSubSet:ALL", debug);
}

void LocIdxSubSet::acceptVisitor(LocationVisitor& pVisitor)
{
    pVisitor.visitLocIdxSubSet(*this);
}

/*!
   Locations are ordered first between Location subclasses 
   based on their sOrder value.
   Then there is an ordering within each subclass.

   When comparing between LocIdxSubSet's will first compare base
   locations and will then will compare indices.

*/
bool LocIdxSubSet::operator<(Location& other)
{
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class
    LocIdxSubSet& loc = static_cast<LocIdxSubSet&>(other);

    if (getLoc() < loc.getLoc()
      || ( (getLoc() == loc.getLoc()) &&
           (getIdx() <  loc.getIdx()) ) ) 
    { return true; } else { return false; }
}   

bool LocIdxSubSet::operator==(Location& other)
{
    if(getOrder() != other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class
    LocIdxSubSet& loc = static_cast<LocIdxSubSet&>(other);


    if ( getLoc() == loc.getLoc() &&
         getIdx() == loc.getIdx() )  
    { return true; } else { return false; }
}

bool LocIdxSubSet::mayOverlap(Location& other)
{
    // may overlap if underlying locations overlap
    // don't know how the overlap occurs so can't assume
    // indices would have to line up
    assert(!getLoc().ptrEqual(NULL));
    return getLoc()->mayOverlap(other);
}


class LocIdxSubSetMustOverlapVisitor : public virtual LocationVisitor {
  private:
    LocIdxSubSet& mThisLoc;
  public:
    bool mMustOverlap;

    LocIdxSubSetMustOverlapVisitor(LocIdxSubSet& thisLoc) 
        : mThisLoc(thisLoc), mMustOverlap(false) {}
    ~LocIdxSubSetMustOverlapVisitor() {}

    void visitNamedLoc(NamedLoc& loc) { mMustOverlap = false; }
    void visitUnnamedLoc(UnnamedLoc& loc) { mMustOverlap = false; } 
    void visitInvisibleLoc(InvisibleLoc& loc) { mMustOverlap = false; } 
    void visitUnknownLoc(UnknownLoc& loc) { mMustOverlap = false; }
    void visitLocSubSet(LocSubSet& loc) { mMustOverlap = false; }
    void visitLocIdxSubSet(LocIdxSubSet& loc) 
      // if other is a LocIdxSubSet then the underlying
      // locations must overlap and they must have same index
      { if ( loc.getLoc()->mustOverlap(*(mThisLoc.getLoc())) 
             && mThisLoc.getIdx() == loc.getIdx() )  
        { mMustOverlap = true; } else { mMustOverlap = false; }
      }
};

bool LocIdxSubSet::mustOverlap(Location& other)
{
    // apply the visitor to the other location and return result
    LocIdxSubSetMustOverlapVisitor mustOverlapVisitor(*this);
    other.acceptVisitor(mustOverlapVisitor);
    return mustOverlapVisitor.mMustOverlap;
}   

// FIXME
bool LocIdxSubSet::subSetOf(Location & other)
{
    // if our location must overlap the other location then
    // we are a subset of the other location
    if (getLoc()->mustOverlap(other)) {
        return true;
    } else {
        return false;
    }
}

void LocIdxSubSet::output(IRHandlesIRInterface& pIR)
{
    sOutBuild->objStart("LocIdxSubSet");

    LocSubSet::output(pIR);
    sOutBuild->field("mIdx",int2string(mIdx));

    sOutBuild->objEnd("LocIdxSubSet");
}


void LocIdxSubSet::dump(std::ostream& os)
{
    os << "LocIdxSubSet(this=" << this << ", mLoc=";
    getLoc()->dump(os);
    os << "\tmIdx=" << getIdx() << " )"; 
    os << std::endl;
}

void LocIdxSubSet::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    os << "LocIdxSubSet(this=" << this << ", mLoc=";
    getLoc()->dump(os,pIR);
    os << "\tmIdx=" << getIdx() << " )"; 
    os << std::endl;
}

std::string LocIdxSubSet::toString(OA_ptr<IRHandlesIRInterface> pIR)
{
  std::ostringstream oss;
    oss << "LocIdxSubSet( mLoc = ";
    assert(!getLoc().ptrEqual(NULL));
    oss << getLoc()->toString(pIR);
    oss << "\tmIdx = " << getIdx() << " )"; 
    return oss.str();
}

} // end namespace

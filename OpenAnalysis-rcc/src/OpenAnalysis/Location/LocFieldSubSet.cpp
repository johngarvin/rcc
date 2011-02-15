/*! \file
  
  \brief Definition for field sub set location abstraction.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "LocFieldSubSet.hpp"
#include <OpenAnalysis/Location/LocationVisitor.hpp>

#include <OpenAnalysis/Utils/OutputBuilder.hpp>
namespace OA {

static bool debug = false;

void LocFieldSubSet::acceptVisitor(LocationVisitor& pVisitor)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_LocFieldSubSet:ALL", debug);
    pVisitor.visitLocFieldSubSet(*this);
}

/*!
   Locations are ordered first between Location subclasses 
   based on their sOrder value.
   Then there is an ordering within each subclass.

   When comparing between LocFieldSubSet's will first compare base
   locations and will then will compare field names.

*/
bool LocFieldSubSet::operator<(Location& other)
{
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class
    LocFieldSubSet& loc = static_cast<LocFieldSubSet&>(other);

    if (getLoc() < loc.getLoc()
          || ( (getLoc() == loc.getLoc()) &&
               (getFieldName() < loc.getFieldName()) ) ) 
      { return true; } else { return false; }
}   

bool LocFieldSubSet::operator==(Location& other)
{
    if(getOrder() != other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class, same underlying loc, and same field name
    LocFieldSubSet& loc = static_cast<LocFieldSubSet&>(other);

	if (getLoc() == loc.getLoc() &&
        getFieldName() == loc.getFieldName() )  
    { return true; } else { return false; }
}

bool LocFieldSubSet::mayOverlap(Location& other)
{
  // this code is incomplete/wrong.
  // FIXME: have to handle unions
  assert(0);
    // may overlap if underlying locations overlap
    // however, if other is also a LocFieldSubSet, 
    // the fields must also match.
    assert(!getLoc().ptrEqual(NULL));
    bool underlyingLocsOverlap = getLoc()->mayOverlap(other);
    return getLoc()->mayOverlap(other);
}


class LocFieldSubSetMustOverlapVisitor : public virtual LocationVisitor {
  private:
    LocFieldSubSet& mThisLoc;
  public:
    bool mMustOverlap;

    LocFieldSubSetMustOverlapVisitor(LocFieldSubSet& thisLoc) 
        : mThisLoc(thisLoc), mMustOverlap(false) {}
    ~LocFieldSubSetMustOverlapVisitor() {}

    void visitNamedLoc(NamedLoc& loc) { mMustOverlap = false; }
    void visitUnnamedLoc(UnnamedLoc& loc) { mMustOverlap = false; } 
    void visitInvisibleLoc(InvisibleLoc& loc) { mMustOverlap = false; } 
    void visitUnknownLoc(UnknownLoc& loc) { mMustOverlap = false; }
    void visitLocSubSet(LocSubSet& loc) { mMustOverlap = false; }
    void visitLocIdxSubSet(LocIdxSubSet& loc) { mMustOverlap = false; }
    void visitLocFieldSubSet(LocFieldSubSet& loc) 
      // if other is a LocFieldSubSet then the underlying
      // locations must overlap and they must have same field
      { if ( loc.getLoc()->mustOverlap(*(mThisLoc.getLoc())) 
             && mThisLoc.getFieldName() == loc.getFieldName() )  
        { mMustOverlap = true; } else { mMustOverlap = false; }
      }
};

bool LocFieldSubSet::mustOverlap(Location& other)
{
    // apply the visitor to the other location and return result
    LocFieldSubSetMustOverlapVisitor mustOverlapVisitor(*this);
    other.acceptVisitor(mustOverlapVisitor);
    return mustOverlapVisitor.mMustOverlap;
}   

// FIXME
bool LocFieldSubSet::subSetOf(Location & other)
{
    // if our location must overlap the other location then
    // we are a subset of the other location
    if (getLoc()->mustOverlap(other)) {
        return true;
    } else {
        return false;
    }
}

void LocFieldSubSet::output(IRHandlesIRInterface& pIR)
{
    sOutBuild->objStart("LocFieldSubSet");

    //LocSubSet::output(pIR);

    sOutBuild->fieldStart("mLoc");
    mLoc->output(pIR);
    sOutBuild->fieldEnd("mLoc");

    sOutBuild->field("mFieldName", mFieldName);

    sOutBuild->objEnd("LocFieldSubSet");
}


void LocFieldSubSet::dump(std::ostream& os)
{
    os << "LocFieldSubSet(this=" << this << ", mLoc=";
    getLoc()->dump(os);
    os << "\tmField=" << mFieldName << " )"; 
    os << std::endl;
}

void LocFieldSubSet::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    os << "LocFieldSubSet(this=" << this << ", mLoc=";
    getLoc()->dump(os,pIR);
    os << "\tmFieldName()=" << mFieldName << " )";
    os << std::endl;
}

std::string LocFieldSubSet::toString(OA_ptr<IRHandlesIRInterface> pIR)
{
    std::ostringstream oss;
    oss << "LocFieldSubSet( mLoc = ";
    assert(!getLoc().ptrEqual(NULL));
    oss << getLoc()->toString(pIR);
    oss << "\tmFieldName()=" << mFieldName << " )";
    return oss.str();
}

} // end namespace

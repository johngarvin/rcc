/*! \file
  
  \brief Definition for location sub set abstraction.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "LocSubSet.hpp"
#include <OpenAnalysis/Location/LocationVisitor.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {

static bool debug = false;

LocSubSet::LocSubSet(OA_ptr<Location> baseLoc) : mFull(false) 
{ 
    OA_DEBUG_CTRL_MACRO("DEBUG_LocSubSet:ALL", debug);

    // will compose this loc sub set with baseLoc and set mLoc
    // accordingly
    composeWithBaseLoc(baseLoc);
}

LocSubSet::LocSubSet(OA_ptr<Location> baseLoc, bool full) : mFull(full) 
{
    // will compose this loc sub set with baseLoc and set mLoc
    // accordingly
    composeWithBaseLoc(baseLoc);
}

/*!
    If the given location is a subset then we will be subset of 
    the baseLoc's base location.  This method also has logic
    for determining what our accuracy should be based on our initially
    specified accuracy and the accuracy of baseLoc
 */
void LocSubSet::composeWithBaseLoc(OA_ptr<Location> baseLoc)
{    
    if (baseLoc->isaSubSet()) {
        /* PLM 1/23/07 deprecated hasFullAccuracy
        mFull = mFull && baseLoc->hasFullAccuracy();
        */
        mLoc = baseLoc->getBaseLoc();

    } else {
        mLoc = baseLoc;
    }
}

void LocSubSet::acceptVisitor(LocationVisitor& pVisitor)
{
    pVisitor.visitLocSubSet(*this);
}

void LocSubSet::output(IRHandlesIRInterface& pIR)
{
    sOutBuild->objStart("LocSubSet");

    sOutBuild->fieldStart("mLoc");
    mLoc->output(pIR);
    sOutBuild->fieldEnd("mLoc");
    sOutBuild->field("mFull", bool2string(mFull));

    sOutBuild->objEnd("LocSubSet");
}

/*!
   Locations are ordered first between Location subclasses 
   based on their sOrder value.
   Then there is an ordering within each subclass.

   When comparing between direct instances of LocSubSet's will first 
   compare base locations and will then let FULL < PARTIAL.
*/
bool LocSubSet::operator<(Location& other)
{
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class
    LocSubSet& loc = static_cast<LocSubSet&>(other);

    // is the location we wrap less than the other guy
    if (getLoc() < loc.getLoc())  {
      return true;
    } else if (getLoc() == loc.getLoc() &&
               isFull() && !loc.isFull()) {
      return true;
    } else { return false; }
}          

bool LocSubSet::operator==(Location& other)
{
    if(getOrder() != other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class
    LocSubSet& loc = static_cast<LocSubSet&>(other);


    if ( getLoc() == loc.getLoc() &&
         isFull() == loc.isFull() )  
    { return true; } else { return false; }
}  

bool LocSubSet::mayOverlap(Location& other)
{
    // may overlap if underlying locations overlap
    assert(!getLoc().ptrEqual(NULL));
    return getLoc()->mayOverlap(other);
}

class LocSubSetMustOverlapVisitor : public virtual LocationVisitor {
  private:
    LocSubSet& mThisLoc;
  public:
    bool mMustOverlap;

    LocSubSetMustOverlapVisitor(LocSubSet& thisLoc) 
        : mThisLoc(thisLoc), mMustOverlap(false) {}
    ~LocSubSetMustOverlapVisitor() {}

    // helper method that determines if mThisLoc is a full
    // subset and if the underlying location for it must
    // overlap with the given location
    void fullAndLocMustOverlap(Location &loc)
      { if ( loc.mustOverlap(*(mThisLoc.getLoc()))
             && mThisLoc.isFull() )
        { mMustOverlap = true; } else { mMustOverlap = false; }
      }
    
    // we overlap with named locations if we are a full subset
    // of that NamedLoc
    void visitNamedLoc(NamedLoc& loc) { fullAndLocMustOverlap(loc); }

    // we overlap with unnamed locations if we are a full subset
    // of that NamedLoc
    void visitUnnamedLoc(UnnamedLoc& loc) { fullAndLocMustOverlap(loc); }

    // we overlap with invisible locations if we are a full subset
    // of that NamedLoc
    void visitInvisibleLoc(InvisibleLoc& loc) { fullAndLocMustOverlap(loc);}
    // no locs must overlap with the UnknownLoc
    void visitUnknownLoc(UnknownLoc& loc) { mMustOverlap = false; }

    // if the other location is a subset then we must
    // overlap with them if we are a full subset
    // and they must overlap with our underlying location
    void visitLocSubSet(LocSubSet& loc) { fullAndLocMustOverlap(loc); }

};

bool LocSubSet::mustOverlap(Location& other)
{
    // apply the visitor to the other location and return result
    LocSubSetMustOverlapVisitor mustOverlapVisitor(*this);
    other.acceptVisitor(mustOverlapVisitor);
    return mustOverlapVisitor.mMustOverlap;
}

// FIXME
bool LocSubSet::subSetOf(Location & other)
{
    // if our location must overlap the other location then
    // we are a subset of the other location
    assert(!getLoc().ptrEqual(NULL));
    if (getLoc()->mustOverlap(other)) {
        return true;
    } else {
        return false;
    }
}


void LocSubSet::dump(std::ostream& os)
{
    os << "LocSubSet(this=" << this << " ,mLoc=";
    getLoc()->dump(os);
    os << "\tisFull=" << isFull() << " )"; 
    os << std::endl;
}

void LocSubSet::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    os << "LocSubSet(this=" << this << " ,mLoc=";
    getLoc()->dump(os,pIR);
    os << "\tisFull=" << isFull() << " )"; 
    os << std::endl;
}

std::string LocSubSet::toString(OA_ptr<IRHandlesIRInterface> pIR)
{
  std::ostringstream oss;
    oss << "LocSubSet( mLoc = ";
    assert(!getLoc().ptrEqual(NULL));
    oss << getLoc()->toString(pIR);
    oss << "\tisFull = " << isFull() << " )"; 
    return oss.str();
}

} // end namespace

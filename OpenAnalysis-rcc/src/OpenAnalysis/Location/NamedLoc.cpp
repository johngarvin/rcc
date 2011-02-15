/*! \file
  
  \brief Definition for named location, location abstraction.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "NamedLoc.hpp"
#include <OpenAnalysis/Location/LocationVisitor.hpp>
#include <OpenAnalysis/Location/Iterators/OverlapSymIterator.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>
namespace OA {

static bool debug = false;

NamedLoc::NamedLoc(NamedLoc & other) : mSymHandle(other.mSymHandle),
                                       mLocal(other.mLocal)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_NamedLoc:ALL", debug);

    //std::copy(other.mFullOverlap->begin(), other.mFullOverlap->end(),
    //          mFullOverlap->begin());
    //std::copy(other.mPartOverlap.begin(), other.mPartOverlap.end(),
    //          mPartOverlap.begin());
    mFullOverlap = other.mFullOverlap;
    mPartOverlap = other.mPartOverlap;
}

void NamedLoc::acceptVisitor(LocationVisitor& pVisitor)
{
    pVisitor.visitNamedLoc(*this);
}

OA_ptr<Location> NamedLoc::getBaseLoc() 
{ 
    OA_ptr<Location> retval; 
    retval = new NamedLoc(*this);
    return retval;
}

OA_ptr<SymHandleIterator> NamedLoc::getPartOverlapIter()
{
    OA_ptr<SymHandleIterator> retval;
    OA_ptr<std::set<SymHandle> > setCopy;
    setCopy = new std::set<SymHandle>(mPartOverlap);
    retval = new OverlapSymIterator(setCopy);
    return retval;
}

OA_ptr<SymHandleIterator> NamedLoc::getFullOverlapIter()
{
    OA_ptr<SymHandleIterator> retval;
    OA_ptr<std::set<SymHandle> > setCopy;
    setCopy = new std::set<SymHandle>(mFullOverlap);
    retval = new OverlapSymIterator(setCopy);
    return retval;
}

/*!
   Locations are ordered first between Location subclasses 
   based on their sOrder value.
   Then there is an ordering within each subclass.

   For NamedLoc the ordering is based on the SymHandle.

   Note: this could cause a problem if consider two NamedLoc's equiv
   if they must overlap.
*/
bool NamedLoc::operator<(Location& other)
{
    if (debug) { 
        std::cout << "NamedLoc::operator<, *this = ";
        this->dump(std::cout);
        std::cout << ", other = ";
        other.dump(std::cout);
    }

    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class
    NamedLoc& loc = static_cast<NamedLoc&>(other);

    if (getSymHandle() < loc.getSymHandle())
    { return true; } else { return false; }
}  

//! only semantically equivalent to another NamedLoc
bool NamedLoc::operator==(Location& other)
{
    // FIXME: what about may and must overlap lists
    if (debug) { 
        std::cout << "NamedLoc::operator==, *this = ";
        this->dump(std::cout);
        std::cout << ", other = ";
        other.dump(std::cout);
    }

    if(getOrder() != other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class
    NamedLoc& loc = static_cast<NamedLoc&>(other);

    if (getSymHandle() == loc.getSymHandle()
        && loc.isLocal() == isLocal() )
    { return true; } else { return false; }
}   

bool NamedLoc::staticFullOverlap(SymHandle sym)
{
  //return std::find(mFullOverlap->begin(),mFullOverlap->end(),sym) 
  //       != mFullOverlap->end();
  //return std::find(mFullOverlap.begin(),mFullOverlap.end(),sym) 
  //       != mFullOverlap.end();
  return mFullOverlap.find(sym) != mFullOverlap.end();
}

bool NamedLoc::staticPartOverlap(SymHandle sym)
{
  return mPartOverlap.find(sym) != mPartOverlap.end();
}

class NamedLocMayOverlapVisitor : public virtual LocationVisitor {
  private:
    NamedLoc& mThisLoc;
  public:
    bool mMayOverlap;

    NamedLocMayOverlapVisitor(NamedLoc& thisLoc) 
        : mThisLoc(thisLoc), mMayOverlap(true) {}
    ~NamedLocMayOverlapVisitor() {}
    
    // we don't overlap other named locations if we have a different
    // symbol and have not been declared to statically overlap
    void visitNamedLoc(NamedLoc& otherLoc) 
      { 
        if (debug) {
          if (mThisLoc.getSymHandle() != otherLoc.getSymHandle()) {
            std::cout << "NOT MAY OVERLAP due to SYMHANDLE\n";
          } else if ( !mThisLoc.staticFullOverlap(otherLoc.getSymHandle())) {
            std::cout << "NOT MAY OVERLAP due to staticFullOverlap\n";
          } else if ( !mThisLoc.staticPartOverlap(otherLoc.getSymHandle())) {
            std::cout << "NOT MAY OVERLAP due to staticPartOverlap\n";
          } else {
            std::cout << "MAY OVERLAP\n";
          }
        }
        if (mThisLoc.getSymHandle() != otherLoc.getSymHandle()
            && !mThisLoc.staticFullOverlap(otherLoc.getSymHandle())
            && !mThisLoc.staticPartOverlap(otherLoc.getSymHandle()) )
        { mMayOverlap = false; } else { mMayOverlap = true; }
      }

    void visitUnnamedLoc(UnnamedLoc& loc) { mMayOverlap = false; }

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

bool NamedLoc::mayOverlap(Location& other)
{
    // apply the visitor to the other location and return result
    NamedLocMayOverlapVisitor mayOverlapVisitor(*this);
    other.acceptVisitor(mayOverlapVisitor);
    return mayOverlapVisitor.mMayOverlap;
}

class NamedLocMustOverlapVisitor : public virtual LocationVisitor {
  private:
    NamedLoc& mThisLoc;

  public:
    bool mMustOverlap;

    NamedLocMustOverlapVisitor(NamedLoc& thisLoc) 
        : mThisLoc(thisLoc), mMustOverlap(false) {}
    ~NamedLocMustOverlapVisitor() {}
    
    // we don't overlap other named locations if we have a different
    // symbol and have not been declared to statically overlap
    void visitNamedLoc(NamedLoc& loc) 
      { if (mThisLoc.getSymHandle() == loc.getSymHandle()
            || mThisLoc.staticFullOverlap(loc.getSymHandle()) )
        { mMustOverlap = true; } else { mMustOverlap = false; }
      }

    void visitUnnamedLoc(UnnamedLoc& loc) { mMustOverlap = false; }

    // don't overlap with Invisibles because anything that does should
    // be mapped to same Invisible by alias analysis
    void visitInvisibleLoc(InvisibleLoc& loc) { mMustOverlap = false; }

    // no locs must overlap with the UnknownLoc
    void visitUnknownLoc(UnknownLoc& loc) { mMustOverlap = false; }

    // if the other location is a subset and we are not then
    // make the other location do the work
    void visitLocSubSet(LocSubSet& loc) 
      { mMustOverlap = loc.mustOverlap(mThisLoc); }
};


bool NamedLoc::mustOverlap(Location& other)
{
    // apply the visitor to the other location and return result
    NamedLocMustOverlapVisitor mustOverlapVisitor(*this);
    other.acceptVisitor(mustOverlapVisitor);
    return mustOverlapVisitor.mMustOverlap;
}


//! conservatively answers this question, if we may overlap with
//! the other location but don't must overlap then we may or may
//! not be a subset so we just return false
//! FIXME: some analyses may want to use this conservatively the
//! other way, in other words, checking if it is not a subset?
bool NamedLoc::subSetOf(Location & other)
{
    return mustOverlap(other);
}

void NamedLoc::output(IRHandlesIRInterface& pIR)
{
    sOutBuild->objStart("NamedLoc");

    sOutBuild->fieldStart("mSymHandle");
    sOutBuild->outputIRHandle(mSymHandle,pIR);
    sOutBuild->fieldEnd("mSymHandle");

    sOutBuild->field("mLocal", bool2string(mLocal));
   
    std::set<SymHandle>::iterator listIter;
    sOutBuild->fieldStart("mFullOverlap");
    sOutBuild->listStart();
    for (listIter=mFullOverlap.begin(); listIter!=mFullOverlap.end(); listIter++)   {
      sOutBuild->listItemStart();
      sOutBuild->outputIRHandle(*listIter,pIR);
      sOutBuild->listItemEnd();
    }
    sOutBuild->listEnd();
    sOutBuild->fieldEnd("mFullOverlap");

    sOutBuild->fieldStart("mPartOverlap");
    sOutBuild->listStart();
    for (listIter=mPartOverlap.begin(); listIter!=mPartOverlap.end(); listIter++)   {
      sOutBuild->outputIRHandle(*listIter,pIR);
      sOutBuild->listItemStart();
      sOutBuild->listItemEnd();
    }
    sOutBuild->listEnd();
    sOutBuild->fieldEnd("mPartOverlap");

    sOutBuild->objEnd("NamedLoc");
}

void NamedLoc::dump(std::ostream& os)
{
    os << "NamedLoc(this=" << this << ", mSymHandle.hval()="
       << mSymHandle.hval() << ", mLocal=" << mLocal << ")";
    os << std::endl;
}

void NamedLoc::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    // string for mSymHandle
    os << "NamedLoc(this=" << this << ", mSymHandle.hval()=" 
       << mSymHandle.hval() << ", mLocal=" << mLocal << "):";
    os << pIR->toString(mSymHandle);
    os << std::endl;
}

std::string NamedLoc::toString(OA_ptr<IRHandlesIRInterface> pIR)
{
    std::ostringstream oss;
    // string for mSymHandle
    oss << "NamedLoc(" << mSymHandle.hval() << "):" 
        << pIR->toString(mSymHandle);
    return oss.str();
}

} // end namespace

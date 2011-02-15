/*! \file
  
  \brief Definition for invisible location, location abstraction.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "InvisibleLoc.hpp"
#include <OpenAnalysis/Location/LocationVisitor.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {

static bool debug = false;

InvisibleLoc::InvisibleLoc(InvisibleLoc &other)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_InvisibleLoc:ALL", debug);
    mMRE = other.mMRE;
}

SymHandle InvisibleLoc::getBaseSym()
{
    OA_ptr<MemRefExpr> m = mMRE; // Stupid Sun CC 5.4

    //  if our mre is another RefOp then call recursively
    if (mMRE->isaRefOp()) {
           OA_ptr<RefOp> refOp = m.convert<RefOp>();
           return refOp->getBaseSym();
    }
    // otherwise should be a named ref
    assert(mMRE->isaNamed());
    OA_ptr<NamedRef> namedRef = m.convert<NamedRef>();
    return namedRef->getSymHandle();
}
		                                         
void InvisibleLoc::acceptVisitor(LocationVisitor& pVisitor)
{
    pVisitor.visitInvisibleLoc(*this);
}

/*!
 Implementation note: don't know whether base symbol of MemRefExpr is
 local or not so indicating it is not local (IOW visible in other procedures
 to be conservative.
 */
/* 
    The Invisible Location will just return same InvisibleLoc
 */
OA_ptr<Location> InvisibleLoc::getBaseLoc() 
{ 
    OA_ptr<Location> retval; 
    retval = new InvisibleLoc(*this);
    return retval;
}

/* some of this code can be used for InvisibleLoc::getBaseSym
    OA_ptr<Location> retval; 
    if (mMRE->isaNamed()) {
        OA_ptr<MemRefExpr> m = mMRE; // Stupid Sun CC 5.4
        OA_ptr<NamedRef> namedRef = m.convert<NamedRef>();
        retval = new NamedLoc(namedRef->getSymHandle(),false);
    } else if (mMRE->isaRefOp()) {
        OA_ptr<MemRefExpr> m = mMRE; // Stupid Sun CC 5.4
        OA_ptr<RefOp> refOp = m.convert<RefOp>();
        retval = new NamedLoc(refOp->getBaseSym(),false);
    } else {
        assert(0);  // shouldn't have an invisible loc without a named base loc
    }
    return retval;
}
*/


/*!
   Locations are ordered first between Location subclasses 
   based on their sOrder value.
   Then there is an ordering within each subclass.

   For InvisibleLoc the ordering is based on the MRE.

*/
bool InvisibleLoc::operator<(Location& other)
{
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class
    InvisibleLoc& loc = static_cast<InvisibleLoc &>(other);

    if (getMemRefExpr() < loc.getMemRefExpr() )
    { return true; } else { return false; }
}

bool InvisibleLoc::operator==(Location& other)
{
    if(getOrder() != other.getOrder()) { return false; }

    // if execution gets here then we're comparing two instances of the same
    // class
    InvisibleLoc& loc = static_cast<InvisibleLoc &>(other);

    if ( getMemRefExpr() == loc.getMemRefExpr() ) 
    { return true; } 
    else { return false; }
}   

class equivalentMREsVisitor : public virtual MemRefExprVisitor {
    private:
        OA_ptr<MemRefExpr> innermost_mre;
        int numDerefs;
    public:
        equivalentMREsVisitor()
        {
            numDerefs=0;
        }
        ~equivalentMREsVisitor() {}
        void visitNamedRef(NamedRef& ref) {
             numDerefs=0; 
             innermost_mre = ref.clone();
        }
        void visitUnnamedRef(UnnamedRef& ref) {
             numDerefs=0;
             innermost_mre = ref.clone();
        }
        void visitUnknownRef(UnknownRef& ref) {
             assert(0);
        }
        void visitAddressOf(AddressOf& ref) {
             OA_ptr<MemRefExpr> mre = ref.getMemRefExpr();
             if (!mre.ptrEqual(0)) { mre->acceptVisitor(*this); }
        }
        void visitDeref(Deref& ref) {
             numDerefs++;
             OA_ptr<MemRefExpr> mre = ref.getMemRefExpr();
             if (!mre.ptrEqual(0)) { mre->acceptVisitor(*this); }
        }
        virtual void visitSubSetRef(SubSetRef& ref) {
             OA_ptr<MemRefExpr> mre = ref.getMemRefExpr();
             if (!mre.ptrEqual(0)) { mre->acceptVisitor(*this); }
        }
        int getnumDerefs() { return numDerefs; }
        OA_ptr<MemRefExpr> getInnermostMRE() { return innermost_mre; }
};


class InvisibleLocMayOverlapVisitor : public virtual LocationVisitor {
  private:
    InvisibleLoc& mThisLoc;
  public:
    bool mMayOverlap;

    InvisibleLocMayOverlapVisitor(InvisibleLoc& thisLoc) 
        : mThisLoc(thisLoc), mMayOverlap(true) {}
    ~InvisibleLocMayOverlapVisitor() {}
    
    // we don't overlap named locations because if we did then those
    // named memory references should have been mapped to same invisible
    // instead of being mapped to a named location
    void visitNamedLoc(NamedLoc& loc) { mMayOverlap = false; }

    // we don't overlap other unnamed locations for a similar reason
    void visitUnnamedLoc(UnnamedLoc& loc) { mMayOverlap = false; }

    // only overlap with other Invisibles that have an equivalent MemRefExpr
    // InvisibleLocs overlap if they have the same number of dereferences
    // and the same base symbol
    void visitInvisibleLoc(InvisibleLoc& loc) 
      { 

         mMayOverlap = false;

         equivalentMREsVisitor eVisitor1;
         OA_ptr<MemRefExpr> mre1 = mThisLoc.getMemRefExpr();
         mre1->acceptVisitor(eVisitor1);
         OA_ptr<MemRefExpr> inner_mre1 = eVisitor1.getInnermostMRE();
         int numderefs1 = eVisitor1.getnumDerefs();

         OA_ptr<MemRefExpr> mre2 = loc.getMemRefExpr();
         equivalentMREsVisitor eVisitor2;
         mre2->acceptVisitor(eVisitor2);
         int numderefs2 = eVisitor2.getnumDerefs();
         OA_ptr<MemRefExpr> inner_mre2 = eVisitor2.getInnermostMRE();

         if(numderefs1 == numderefs2) {
             if(inner_mre1->isaNamed() && inner_mre2->isaNamed()) {
                OA_ptr<NamedRef> named1 = inner_mre1.convert<NamedRef>();
                OA_ptr<NamedRef> named2 = inner_mre2.convert<NamedRef>();
                if(named1->getSymHandle() == named2->getSymHandle()) {
                    mMayOverlap = true;
                }
             }
         }
      }

    // all locs overlap with the UnknownLoc
    void visitUnknownLoc(UnknownLoc& loc) { mMayOverlap = true; }

    // if the other location is a subset and we are not then
    // make the other location do the work
    void visitLocSubSet(LocSubSet& loc) 
      { mMayOverlap = loc.mayOverlap(mThisLoc); }
};

bool InvisibleLoc::mayOverlap(Location& other)
{
    // apply the visitor to the other location and return result
    InvisibleLocMayOverlapVisitor mayOverlapVisitor(*this);
    other.acceptVisitor(mayOverlapVisitor);
    return mayOverlapVisitor.mMayOverlap;
}

class InvisibleLocMustOverlapVisitor : public virtual LocationVisitor {
  private:
    InvisibleLoc& mThisLoc;
  public:
    bool mMustOverlap;

    InvisibleLocMustOverlapVisitor(InvisibleLoc& thisLoc) 
        : mThisLoc(thisLoc), mMustOverlap(false) {}
    ~InvisibleLocMustOverlapVisitor() {}
    
    // we don't overlap other named locations 
    void visitNamedLoc(NamedLoc& loc) { mMustOverlap = false; }

    void visitUnnamedLoc(UnnamedLoc& loc) { mMustOverlap = false; }

    // if the memory reference expression is equivalent
    // then we must overlap with another invisible location
    void visitInvisibleLoc(InvisibleLoc& loc) 
      { if ( mThisLoc.getMemRefExpr() == loc.getMemRefExpr() ) 
        { mMustOverlap = true; } else { mMustOverlap = false; }
      }

    // no locs must overlap with the UnknownLoc
    void visitUnknownLoc(UnknownLoc& loc) { mMustOverlap = false; }

    // if the other location is a subset and we are not then
    // make the other location do the work
    void visitLocSubSet(LocSubSet& loc) 
      { mMustOverlap = loc.mustOverlap(mThisLoc); }
};

bool InvisibleLoc::mustOverlap(Location& other)
{
    // apply the visitor to the other location and return result
    InvisibleLocMustOverlapVisitor mustOverlapVisitor(*this);
    other.acceptVisitor(mustOverlapVisitor);
    return mustOverlapVisitor.mMustOverlap;
}

//! conservatively answers this question, if we may overlap with
//! the other location but don't must overlap then we may or may
//! not be a subset so we just return false
//! FIXME
bool InvisibleLoc::subSetOf(Location & other)
{
    return mustOverlap(other);
}

void InvisibleLoc::output(IRHandlesIRInterface& pIR)
{
    sOutBuild->objStart("InvisibleLoc");

    sOutBuild->fieldStart("mMRE");
    mMRE->output(pIR);
    sOutBuild->fieldEnd("mMRE");

    sOutBuild->objEnd("InvisibleLoc");
}

void InvisibleLoc::dump(std::ostream& os)
{
    // string for mSymHandle
    os << "InvisibleLoc(this=" << this << ", mMRE=";
    mMRE->dump(os);
    os << " )" << std::endl;
}


void InvisibleLoc::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    // string for mSymHandle
    os << "InvisibleLoc(this=" << this << ", mMRE=";
    mMRE->dump(os,pIR);
    os << " )" << std::endl;
}

std::string InvisibleLoc::toString(OA_ptr<IRHandlesIRInterface> pIR)
{
    std::ostringstream oss;
    // string for mSymHandle
    oss << "InvisibleLoc: ";
    mMRE->dump(oss,pIR);
    return oss.str();
}

} // end namespace

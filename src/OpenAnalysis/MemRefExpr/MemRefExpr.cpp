/*! \file
  
  \brief Definition for MemRefExpr class hierarchy.  

  \authors Michelle Strout, Andy Stone
  \version $Id: MemRefExpr.cpp,v 1.14.6.6 2005/11/04 16:24:12 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "MemRefExpr.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

// included here instead of in hpp file to remove circular reference
#include <OpenAnalysis/MemRefExpr/MemRefExprVisitor.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {

using namespace std;

//*****************************************************************
// MemRefExpr methods
//*****************************************************************

/*!
   Need consistent ordering.  
*/
bool MemRefExpr::operator<(MemRefExpr& other) 
{
    bool retval = false;
    if(getOrder() < other.getOrder()) { retval = true; }
    return retval;
}

bool MemRefExpr::operator==(MemRefExpr& other) 
{
    bool retval = false;
    if(getOrder() == other.getOrder()) { retval = true; }
    return retval;
}

void MemRefExpr::output(IRHandlesIRInterface& pIR) const
{
    sOutBuild->field("mMemRefType", toString(mMemRefType) );
}

void MemRefExpr::dump(std::ostream& os, IRHandlesIRInterface& pIR)
{
    MemRefExpr::dump(os);
}


void MemRefExpr::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    MemRefExpr::dump(os);
}

void MemRefExpr::dump(std::ostream& os)
{
    os << "MemRefExpr(this=" << this 
       << ", type = " << toString(mMemRefType) << ")\t";
}

string MemRefExpr::comment(const IRHandlesIRInterface& pIR) const {
    return "MemRefExpr()";
}

std::string MemRefExpr::toString(MemRefType type)  const
{
    std::ostringstream oss;
    switch (mMemRefType) {
      case USE: oss << "USE"; break;
      case DEF: oss << "DEF"; break;
      case USEDEF: oss << "USEDEF"; break;
      case DEFUSE: oss << "DEFUSE"; break;
    }
    return oss.str();
}

//*****************************************************************
// NamedRef methods
//*****************************************************************

void NamedRef::acceptVisitor(MemRefExprVisitor& pVisitor)
{
    pVisitor.visitNamedRef(*this);
}

OA_ptr<MemRefExpr>  NamedRef::clone()
{
    OA_ptr<MemRefExpr> retval;
    retval = new NamedRef(*this);
    return retval;
}

/*! 
   References are ordered first between MemRefExpr subclasses 
   based on their sOrder value.
   Then there is a consistent ordering within each subclass.

   For NamedRef's ordering is done by SymHandle.
*/
bool NamedRef::operator<(MemRefExpr& other)
{
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // execution will reach here of two NamedRef objects are being compared
    NamedRef& ref = static_cast<NamedRef&>(other);

    if( getSymHandle() < ref.getSymHandle() )
    {
        return true;
    } else {
        return false;
    }
} 

bool NamedRef::operator==(MemRefExpr& other)
{
    if(getOrder() != other.getOrder()) { return false; }

    // execution reaches here if two NamedRef objects are being compared
    NamedRef& ref = static_cast<NamedRef&>(other);

    if( getSymHandle() == ref.getSymHandle() ) 
    {
        return true;
    } else {
        return false;
    }
} 

void NamedRef::output(IRHandlesIRInterface& pIR) const
{
    sOutBuild->objStart("NamedRef");
    MemRefExpr::output(pIR);
    sOutBuild->fieldStart("mSymHandle");
    sOutBuild->outputIRHandle(mSymHandle,pIR);
    sOutBuild->fieldEnd("mSymHandle");
    sOutBuild->field("mStrictlyLocal", 
                     OA::int2string(mStrictlyLocal));
    sOutBuild->objEnd("NamedRef");
}

void NamedRef::dump(std::ostream& os)
{
    MemRefExpr::dump(os);
    os << "NamedRef(this=" << this << ", mSymHandle.hval()=" 
       << mSymHandle.hval() << ", mStrictlyLocal="
       << mStrictlyLocal << ")";
    os << std::endl;
}

void NamedRef::dump(std::ostream& os, IRHandlesIRInterface& pIR)
{
    // string for mSymHandle
    MemRefExpr::dump(os,pIR);
    os << "NamedRef(this=" << this << ", mSymHandle.hval()=" 
       << mSymHandle.hval() << ", mStrictlyLocal="
       << mStrictlyLocal << "): "
       << pIR.toString(mSymHandle);
    os << std::endl;
}


void NamedRef::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    dump(os,*pIR);
}

string NamedRef::comment(const IRHandlesIRInterface& pIR) const {
    return "NamedRef(" + pIR.toString(mSymHandle) + ")";
}

//*****************************************************************
// UnnamedRef methods
//*****************************************************************

void UnnamedRef::acceptVisitor(MemRefExprVisitor& pVisitor)
{
    pVisitor.visitUnnamedRef(*this);
}

OA_ptr<MemRefExpr>  UnnamedRef::clone()
{
    OA_ptr<MemRefExpr> retval;
    retval = new UnnamedRef(*this);
    return retval;
}

/*! 
   References are ordered first between MemRefExpr subclasses 
   based on their sOrder value.
   Then there is a consistent ordering within each subclass.

   For UnnamedRef's ordering is done by ExprHandle.
*/
bool UnnamedRef::operator<(MemRefExpr& other)
{
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // execution will reach here of two UnnamedRef objects are being compared
    UnnamedRef& ref = static_cast<UnnamedRef&>(other);

    if( getExprHandle() < ref.getExprHandle() )
    {
        return true;
    } else {
        return false;
    }
}  

bool UnnamedRef::operator==(MemRefExpr& other)
{
    if(getOrder() != other.getOrder()) { return false; }

    // execution reaches here if two UnnamedRef objects are being compared
    UnnamedRef& ref = static_cast<UnnamedRef&>(other);

    if( getExprHandle() == ref.getExprHandle() ) 
    {
        return true;
    } else {
        return false;
    }
}   

void UnnamedRef::output(IRHandlesIRInterface& pIR) const
{
    sOutBuild->objStart("UnnamedRef");
    MemRefExpr::output(pIR);
    sOutBuild->fieldStart("mExprHandle");
    sOutBuild->outputIRHandle(mExprHandle,pIR);
    sOutBuild->fieldEnd("mExprHandle");
    sOutBuild->field("mLocal", bool2string(mLocal));
    if(isLocal() == true) {
       sOutBuild->fieldStart("mProcHandle");
       sOutBuild->outputIRHandle(mProcHandle,pIR);
       sOutBuild->fieldEnd("mProcHandle");
    }
    sOutBuild->objEnd("UnnamedRef");
}


void UnnamedRef::dump(std::ostream& os)
{
    MemRefExpr::dump(os);
    os << "UnnamedRef(this=" << this << ", mExprHandle.hval()="
       << mExprHandle.hval() << ", mLocal=" << mLocal;
    if( isLocal() == true) {
        os << ", mProcHandle.hval()=" << mProcHandle.hval();
    }
    os << ")";
    os << std::endl;
}

void UnnamedRef::dump(std::ostream& os, IRHandlesIRInterface& pIR)
{
    MemRefExpr::dump(os,pIR);
    os << "UnnamedRef(this=" << this << ", mExprHandle.hval()="
       << mExprHandle.hval() << "): "
       << pIR.toString(mExprHandle);
    os << std::endl;
}

void UnnamedRef::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    dump(os,*pIR);
}

string UnnamedRef::comment(const IRHandlesIRInterface& pIR) const {
    return "UnnamedRef()";
}

//*****************************************************************
// UnknownRef methods
//*****************************************************************

void UnknownRef::acceptVisitor(MemRefExprVisitor& pVisitor)
{
    pVisitor.visitUnknownRef(*this);
}

OA_ptr<MemRefExpr>  UnknownRef::clone()
{
    OA_ptr<MemRefExpr> retval;
    retval = new UnknownRef(*this);
    return retval;
}

bool UnknownRef::operator<(MemRefExpr& other) 
{
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // execution will reach here of two UnknownRef objects are being compared
    return false;
}
    
bool UnknownRef::operator==(MemRefExpr& other)
{
    if(getOrder() != other.getOrder()) { return false; }
    else { return true; }
}

void UnknownRef::output(IRHandlesIRInterface& pIR) const
{
    sOutBuild->objStart("UnknownRef");
    MemRefExpr::output(pIR);
    sOutBuild->objEnd("UnknownRef");
}


void UnknownRef::dump(std::ostream& os)
{
    MemRefExpr::dump(os);
    os << "UnknownRef";
    os << std::endl;
}

void UnknownRef::dump(std::ostream& os, IRHandlesIRInterface& pIR)
{
    MemRefExpr::dump(os,pIR);
    os << "UnknownRef";
    os << std::endl;
}
void UnknownRef::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    dump(os,*pIR);
}

string UnknownRef::comment(const IRHandlesIRInterface& pIR) const {
    return "UnknownRef()";
}

//*****************************************************************
// RefOp methods
//*****************************************************************
SymHandle RefOp::getBaseSym()
{
    OA_ptr<MemRefExpr> m = mMRE; // Stupid Sun CC 5.4

    // if our mre is another RefOp then call recursively
    if (mMRE->isaRefOp()) {
        OA_ptr<RefOp> refOp = m.convert<RefOp>();
        return refOp->getBaseSym();
    }

    // otherwise should be a named ref
    assert(mMRE->isaNamed());
    OA_ptr<NamedRef> namedRef = m.convert<NamedRef>();
    return namedRef->getSymHandle();
}

OA_ptr<MemRefExpr> RefOp::getBase() {
    OA_ptr<MemRefExpr> m = mMRE; // Stupid Sun CC 5.4

    // if our mre is another RefOp then call recursively
    if (mMRE->isaRefOp()) {
        OA_ptr<RefOp> refOp = m.convert<RefOp>();
        return refOp->getBase();
    }

    // otherwise should be a named ref
    assert(mMRE->isaNamed());
    OA_ptr<NamedRef> namedRef = m.convert<NamedRef>();
    return namedRef;
}

void RefOp::output(IRHandlesIRInterface& ir) const
{
    MemRefExpr::output(ir);

    if (! mMRE.ptrEqual(0) ) {
        sOutBuild->fieldStart("mMRE");
        mMRE->output(ir);
        sOutBuild->fieldEnd("mMRE");
    } else {
        sOutBuild->field("mMRE", "<null MRE>");
    }
        
}


//*****************************************************************
// AddressOf methods
//*****************************************************************

void AddressOf::acceptVisitor(MemRefExprVisitor& pVisitor)
{
    pVisitor.visitAddressOf(*this);
}

OA_ptr<MemRefExpr> AddressOf::clone()
{
    OA_ptr<MemRefExpr> retval;
    retval = new AddressOf(*this);
    return retval;
}

bool AddressOf::operator<(MemRefExpr& other)
{
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // execution will reach here of two AddressOf objects are being compared
    AddressOf& ref = static_cast<AddressOf&>(other);

    if( getMemRefExpr() < ref.getMemRefExpr() )
    {
        return true;
    } else {
        return false;
    }
}

bool AddressOf::operator==(MemRefExpr& other)
{
    if(getOrder() != other.getOrder()) { return false; }

    // execution reaches here if two AddressOf objects are being compared
    AddressOf& ref = static_cast<AddressOf&>(other);

    if( getMemRefExpr() == ref.getMemRefExpr() )
    {
        return true;
    } else {
        return false;
    }
}


OA_ptr<MemRefExpr> AddressOf::composeWith(OA_ptr<MemRefExpr> mre)
{
    OA_ptr<MemRefExpr> retval;
    OA_ptr<MemRefExpr> clone_mre ;
    clone_mre = mre->clone();

   if(clone_mre->isaRefOp()) {

       OA_ptr<RefOp> ref = clone_mre.convert<RefOp>();

       if (ref->isaDeref()) {
           OA_ptr<MemRefExpr> child_mre = ref->getMemRefExpr();
           return child_mre;
       }
       if (ref->isaAddressOf()) {
           return clone_mre;
       }
   }

   retval = new AddressOf(this->getMRType(), clone_mre);
   return retval;
}



void AddressOf::output(IRHandlesIRInterface& pIR) const
{
    sOutBuild->objStart("AddressOf");
    RefOp::output(pIR);
    sOutBuild->objEnd("AddressOf");
}


void AddressOf::dump(std::ostream& os)
{
    MemRefExpr::dump(os);
    os << "AddressOf(this=" << this << ", mMemRefExpr=";
    getMemRefExpr()->dump(os);
    os << std::endl;
}

void AddressOf::dump(std::ostream& os, IRHandlesIRInterface& pIR)
{
    MemRefExpr::dump(os,pIR);
    os << "AddressOf(this=" << this << ", mMemRefExpr=";
    getMemRefExpr()->dump(os,pIR);
    os << std::endl;
}

void AddressOf::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    dump(os,*pIR);
}

string AddressOf::comment(const IRHandlesIRInterface& pIR) const {
    return "AddressOf()";
}

//*****************************************************************
// Deref methods
//*****************************************************************

void Deref::acceptVisitor(MemRefExprVisitor& pVisitor)
{
    pVisitor.visitDeref(*this);
}

OA_ptr<MemRefExpr>  Deref::clone()
{
    OA_ptr<MemRefExpr> retval;
    retval = new Deref(*this);
    return retval;
}

/*! 
   References are ordered first between MemRefExpr subclasses in an
   specific order: NamedRef < UnnamedRef < Deref
   < SubSetRefs( IdxAccess < ... ) < UnknownRef.
   Then there is a consistent ordering within each subclass.

   For Deref's ordering is done by underlying reference and then
   number of derefs.
*/
bool Deref::operator<(MemRefExpr& other)
{
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // execution will reach here of two Deref objects are being compared
    Deref& ref = static_cast<Deref&>(other);


    if( (getMemRefExpr() < ref.getMemRefExpr() ||
          (getMemRefExpr() == ref.getMemRefExpr()
            && getNumDerefs() < ref.getNumDerefs()) ) )
    {
        return true;
    } else { 
        return false;
    }
}   

bool Deref::operator==(MemRefExpr& other)
{
    if(getOrder() != other.getOrder()) { return false; }

    // execution reaches here if two Deref objects are being compared
    Deref& ref = static_cast<Deref&>(other);


    // check if deref count is equal and underlying memref is equal
    // have to check that our MemRefExpr fields like fullAccuracy are
    // equal because accuracy can change at any level
    if( getNumDerefs() == ref.getNumDerefs()
        && getMemRefExpr() == ref.getMemRefExpr() )
    {
        return true;
    } else {
        return false;
    }
}



OA_ptr<MemRefExpr> Deref::composeWith(OA_ptr<MemRefExpr> mre)
{
    OA_ptr<MemRefExpr> retval;
    OA_ptr<MemRefExpr> clone_mre = mre->clone();      

    if( clone_mre->isaRefOp() ) {
      OA_ptr<RefOp> ref = clone_mre.convert<RefOp>();

      if (ref->isaAddressOf()) {
          OA_ptr<MemRefExpr> child_mre = ref->getMemRefExpr();
          retval = child_mre;
          return retval;
      }
    }   
          
    retval = new Deref(this->getMRType(), clone_mre, 1);

    return retval;
}

void Deref::output(IRHandlesIRInterface& pIR) const
{
    sOutBuild->objStart("Deref");
    RefOp::output(pIR);
    sOutBuild->field("mNumDeref", int2string(mNumDeref));
    sOutBuild->objEnd("Deref");
}


void Deref::dump(std::ostream& os)
{
    MemRefExpr::dump(os);
    os << "Deref(this=" << this << ", mMemRefExpr=";
    getMemRefExpr()->dump(os);
    os << "\tmNumDerefs = " << getNumDerefs() << " )"; 
    os << std::endl;
}

void Deref::dump(std::ostream& os, IRHandlesIRInterface& pIR)
{
    MemRefExpr::dump(os,pIR);
    os << "Deref(this=" << this << ", mMemRefExpr=";
    getMemRefExpr()->dump(os,pIR);
    os << "\tmNumDerefs = " << getNumDerefs() << " )"; 
    os << std::endl;
}

void Deref::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    dump(os,*pIR);
}

string Deref::comment(const IRHandlesIRInterface& pIR) const {
    return "Deref(" + mMRE->comment(pIR) + ")";
}

//*****************************************************************
// SubSetRef methods
//*****************************************************************
OA_ptr<MemRefExpr>  SubSetRef::clone()
{
    OA_ptr<MemRefExpr> retval;
    retval = new SubSetRef(*this);
    return retval;
}

void SubSetRef::acceptVisitor(MemRefExprVisitor& pVisitor)
{
    pVisitor.visitSubSetRef(*this);
}



OA_ptr<MemRefExpr> SubSetRef::composeWith(OA_ptr<MemRefExpr> mre)
{
    OA_ptr<MemRefExpr> clone_mre = mre->clone();
    OA_ptr<MemRefExpr> retval;

    retval = new SubSetRef(clone_mre->getMRType(), clone_mre);
    return retval;
}



void SubSetRef::output(IRHandlesIRInterface& pIR) const
{
    sOutBuild->objStart("SubSetRef");
    RefOp::output(pIR);
    sOutBuild->objEnd("SubSetRef");
}



bool SubSetRef::operator<(MemRefExpr& other)
{
   if(getOrder() < other.getOrder()) { return true; }
   else if(getOrder() > other.getOrder()) { return false; }
   SubSetRef& ref = static_cast<SubSetRef&>(other);
   if( getMemRefExpr() < ref.getMemRefExpr() ) 
   {
       return true;
   } else {
       return false;
   }
}


bool SubSetRef::operator==(MemRefExpr& other)
{
    if(getOrder() != other.getOrder()) { return false; }
    // execution reaches here if two Deref objects are being compared
    SubSetRef& ref = static_cast<SubSetRef&>(other);
    if( getMemRefExpr() == ref.getMemRefExpr() )
    {
        return true;
    } else {
        return false;
    }
}


void SubSetRef::dump(std::ostream& os)
{
    MemRefExpr::dump(os);
    os << "SubSetRef(this=" << this << ", mMemRefExpr=";
    getMemRefExpr()->dump(os);
    os << " )";
    os << std::endl;
}

void SubSetRef::dump(std::ostream& os, IRHandlesIRInterface& pIR)
{
    MemRefExpr::dump(os,pIR);
    os << "SubSetRef(this=" << this << ", mMemRefExpr=";
    getMemRefExpr()->dump(os,pIR);
    os << " )";
    os << std::endl;
}

void SubSetRef::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    dump(os,*pIR);
}

string SubSetRef::comment(const IRHandlesIRInterface& pIR) const {
    if(mMRE.ptrEqual(NULL)) {
        return "SubSetRef(?)";
    }
    return "SubSetRef(" + mMRE->comment(pIR) + ")";
}

//*****************************************************************
// IdxAccess methods
//*****************************************************************

void IdxAccess::acceptVisitor(MemRefExprVisitor& pVisitor)
{
    pVisitor.visitIdxAccess(*this);
}

OA_ptr<MemRefExpr>  IdxAccess::clone()
{
    OA_ptr<MemRefExpr> retval;
    retval = new IdxAccess(*this);
    return retval;
}

/*! 
   References are ordered first between MemRefExpr subclasses in an
   specific order: NamedRef < UnnamedRef < Deref
   < SubSetRefs( IdxAccess < ... ) < UnknownRef.
   Then there is a consistent ordering within each subclass.

   For IdxAccess ordering is done by underlying reference and then
   then idx.
   
   Any subclasses to SubSetRef that are added later must handle 
   IdxAccess specifically,
   otherwise there will be inconsistent ordering.
*/
bool IdxAccess::operator<(MemRefExpr& other)
{
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // execution will reach here of two IdxAccess objects are being compared
    IdxAccess& ref = static_cast<IdxAccess&>(other);

    if( getMemRefExpr() < ref.getMemRefExpr() ||
          (getMemRefExpr() == ref.getMemRefExpr()
            && getIdx() < ref.getIdx()) )
    {
        return true;
    } else {
        return false;
    }
}  

bool IdxAccess::operator==(MemRefExpr& other)
{
    if(getOrder() != other.getOrder()) { return false; }

    // execution reaches here if two IdxAccess objects are being compared
    IdxAccess& ref = static_cast<IdxAccess&>(other);


    if( getMemRefExpr() == ref.getMemRefExpr()
         && getIdx() == ref.getIdx() )
    {
        return true;
    } else {
        return false;
    }
}

void IdxAccess::output(IRHandlesIRInterface& pIR) const
{
    sOutBuild->objStart("IdxAccess");
    RefOp::output(pIR);
    sOutBuild->field("mIdx", int2string(mIdx));
    sOutBuild->objEnd("IdxAccess");
}


void IdxAccess::dump(std::ostream& os)
{
    MemRefExpr::dump(os);
    os << "IdxAccess(this=" << this << ", mMemRefExpr=";
    getMemRefExpr()->dump(os);
    os << "\tmIdx=" << getIdx() << " )"; 
    os << std::endl;
}

void IdxAccess::dump(std::ostream& os, IRHandlesIRInterface& pIR)
{
    MemRefExpr::dump(os,pIR);
    os << "IdxAccess(this=" << this << ", mMemRefExpr=";
    getMemRefExpr()->dump(os,pIR);
    os << "\tmIdx=" << getIdx() << " )"; 
    os << std::endl;
}

void IdxAccess::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    dump(os,*pIR);
}

string IdxAccess::comment(const IRHandlesIRInterface& pIR) const {
    if(mMRE.ptrEqual(NULL)) {
        return "IdxAccess(?)";
    }
    return "IdxAccess(" + mMRE->comment(pIR) + ")";
}

//*****************************************************************
// IdxExprAccess methods
//*****************************************************************
void IdxExprAccess::acceptVisitor(MemRefExprVisitor& pVisitor) {
    pVisitor.visitIdxExprAccess(*this);
}

OA_ptr<MemRefExpr> IdxExprAccess::clone() {
    OA_ptr<MemRefExpr> retval;
    retval = new IdxExprAccess(*this);
    return retval;
}

/*! 
*/
bool IdxExprAccess::operator<(MemRefExpr & other) {
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // execution will reach here of two FieldAccess objects are being compared
    IdxExprAccess& ref = static_cast<IdxExprAccess&>(other);

    if( (getMemRefExpr() < ref.getMemRefExpr()) ||
            (getMemRefExpr() == ref.getMemRefExpr()
             && getExpr() < ref.getExpr()) )
    {
        return true;
    } else {
        return false;
    }
}

bool IdxExprAccess::operator==(MemRefExpr& other) {
    if(getOrder() != other.getOrder()) { return false; }

    // execution reaches here if two IdxExprAccess objects are being compared
    IdxExprAccess& ref = static_cast<IdxExprAccess&>(other);


    if( getMemRefExpr() == ref.getMemRefExpr() 
         && getExpr() == ref.getExpr() )
    {
        return true;
    } else {
        return false;
    }
}

void IdxExprAccess::output(IRHandlesIRInterface& ir) const {
    sOutBuild->objStart("IdxExprAccess");
    RefOp::output(ir);
    sOutBuild->outputIRHandle(mhExpr, ir);
    sOutBuild->objEnd("IdxExprAccess");
}

void IdxExprAccess::dump(std::ostream& os)
{
    MemRefExpr::dump(os);
    os << "IdxExprAccess(this=" << this << ", mMemRefExpr=";
    getMemRefExpr()->dump(os);
    os << "\tmhExpr=" << mhExpr.hval () << " )";
    os << std::endl;
}

void IdxExprAccess::dump(std::ostream& os, IRHandlesIRInterface& pIR)
{
    MemRefExpr::dump(os,pIR);
    os << "IdxExprAccess(this=" << this << ", mMemRefExpr="; 
    getMemRefExpr()->dump(os,pIR);
    os << "\tmhExpr=" << mhExpr.hval() << " ) ";
    os << std::endl;
}

void IdxExprAccess::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR) 
{
    dump(os,*pIR);
}

string IdxExprAccess::comment(const IRHandlesIRInterface& pIR) const {
    if(mMRE.ptrEqual(NULL)) {
        return "IdxExprAccess(?)";
    }
    return "IdxExprAccess(" + mMRE->comment(pIR) + ")";
}


//*****************************************************************
// FieldAccess methods
//*****************************************************************

void FieldAccess::acceptVisitor(MemRefExprVisitor& pVisitor)
{
    pVisitor.visitFieldAccess(*this);
}

OA_ptr<MemRefExpr>  FieldAccess::clone()
{
    OA_ptr<MemRefExpr> retval;
    retval = new FieldAccess(*this);
    return retval;
}

/*! 
   For FieldAccess ordering is done by underlying reference and then
   then by FieldName.
   
   Any subclasses to SubSetRef that are added later must handle 
   FieldName (and IdxAccess) specifically,
   otherwise there will be inconsistent ordering.
*/
bool FieldAccess::operator<(MemRefExpr& other)
{
    if(getOrder() < other.getOrder()) { return true; }
    else if(getOrder() > other.getOrder()) { return false; }

    // execution will reach here of two FieldAccess objects are being compared
    FieldAccess& ref = static_cast<FieldAccess&>(other);

    if( getMemRefExpr() < ref.getMemRefExpr() ||
         (getMemRefExpr() == ref.getMemRefExpr()
            && getFieldName() < ref.getFieldName()) ) 
    {
        return true;
    } else {
        return false;
    }
}  

bool FieldAccess::operator==(MemRefExpr& other)
{
    if(getOrder() != other.getOrder()) { return false; }

    // execution reaches here if two FieldAccess objects are being compared
    FieldAccess& ref = static_cast<FieldAccess&>(other);

    if( getMemRefExpr() == ref.getMemRefExpr()
         && getFieldName() == ref.getFieldName() )
    {
        return true;
    } else {
        return false;
    }
}
 
void FieldAccess::output(IRHandlesIRInterface& pIR) const
{
    sOutBuild->objStart("FieldAccess");

    RefOp::output(pIR);

    sOutBuild->field("mFieldName",mFieldName);

    sOutBuild->objEnd("FieldAccess");
}

void FieldAccess::dump(std::ostream& os)
{
    MemRefExpr::dump(os);
    os << "FieldAccess(this=" << this << ", mMemRefExpr=";
    getMemRefExpr()->dump(os);
    //    os << "\tmFieldName.hval()=" << mFieldName.hval() << " )"; 
    os << "\tmFieldName=" << mFieldName << " )"; 
    os << std::endl;
}

void FieldAccess::dump(std::ostream& os, IRHandlesIRInterface& pIR)
{
    MemRefExpr::dump(os,pIR);
    os << "FieldAccess(";
    getMemRefExpr()->dump(os,pIR);
    //    os << "\tmFieldName()=" << mFieldName.hval() << " ): " << pIR.toString(mFieldName); 
    os << "\tmFieldName()=" << mFieldName << " ) ";
    os << std::endl;
}

void FieldAccess::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR)
{
    dump(os,*pIR);
}

string FieldAccess::comment(const IRHandlesIRInterface& pIR) const {
    if(mMRE.ptrEqual(NULL)) {
        return "FieldAccess(?, " + mFieldName + ")";
    }
    return "FieldAccess(" + mMRE->comment(pIR) + ", " + mFieldName + ")";
}


} // end of OA namespace


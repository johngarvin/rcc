/*! \file
  
  \brief Declarations for the MemRefExpr abstraction.

  \authors Michelle Strout, Andy Stone
  \version $Id: MemRefExpr.hpp,v 1.17.6.7 2005/11/04 16:24:12 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef MemRefExpr_H
#define MemRefExpr_H

#include <iostream>
#include <cassert>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Utils/SetIterator.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>

namespace OA {

class MemRefExpr;
class IdxExprAccess;
class MemRefExpr;
class NamedRef;
class UnnamedRef;
class UnknownRef;
class RefOp;
class Deref;
class SubSetRef;
class IdxAccess;
class IdxExprAccess;
class FieldAccess;

class MemRefExprVisitor;

// ----- Iterator classes -----
template<class T>
class MREIteratorClass {
  public:
    MREIteratorClass() { }
    virtual ~MREIteratorClass() { }

    // Returns the current item.
    virtual OA_ptr<T> current() const = 0;

    // False when all items are exhausted.;
    virtual bool isValid() const = 0;

    virtual void operator++() = 0;
    void operator++(int) { ++*this; } ;

    virtual void reset() = 0;
};

typedef SetIterator<OA_ptr<MemRefExpr> >     MemRefExprIterator;
typedef SetIterator<OA_ptr<NamedRef> >       NamedRefIterator;
typedef SetIterator<OA_ptr<UnnamedRef> >     UnnamedRefIterator;
typedef SetIterator<OA_ptr<UnknownRef> >     UnknownRefIterator;
typedef SetIterator<OA_ptr<RefOp> >          RefOpIterator;
typedef SetIterator<OA_ptr<Deref> >          DerefIterator;
typedef SetIterator<OA_ptr<SubSetRef> >      SubSetRefIterator;
typedef SetIterator<OA_ptr<IdxAccess> >      IdxAccessIterator;
typedef SetIterator<OA_ptr<IdxExprAccess> >  IdxExprAccessIterator;
typedef SetIterator<OA_ptr<FieldAccess> >    FieldAccessIterator;

// ------ MemRefExpr classes -----

/*! abstract base class that has default implementations for the
    methods that all MemRefExpr's must provide
*/
class MemRefExpr : public Annotation {
  public:
    typedef enum {
      USE,     // mem ref specifies where to find source location for use
      DEF,     // mem ref specifies where to put result of assign
      USEDEF,  // mem ref that specifies a location that will be used first
               // and then defined (eg. i++)
      DEFUSE   // mem ref that specifies a location that will be defined first
               // and then used (eg. i++)
    } MemRefType;

    MemRefExpr(MemRefType mrType) : mMemRefType(mrType) {}

    //! copy constructor
    MemRefExpr(MemRefExpr &mre) : mMemRefType(mre.mMemRefType) {}

    virtual ~MemRefExpr() {}

    virtual void acceptVisitor(MemRefExprVisitor& pVisitor) = 0;

    // return a ptr to a copy of self
    virtual OA_ptr<MemRefExpr> clone() = 0;

    //*****************************************************************
    // Subclass type methods 
    //*****************************************************************
    virtual bool isaNamed() { return false; }
    virtual bool isaUnnamed() { return false; }
    virtual bool isaUnknown() { return false; }
    virtual bool isaRefOp() { return false; }
    
    virtual bool isaDeref() { return false; }
    virtual bool isaAddressOf() { return false; }

    virtual bool isaSubSetRef() { return false; }
    virtual bool isaIdxAccess() { return false; }
    virtual bool isaIdxExprAccess() { return false; }
    virtual bool isaFieldAccess() { return false; }

    //*****************************************************************
    // Info methods 
    //*****************************************************************
    
    //! whether USE/DEF MemRefExpr    
    MemRefType getMRType() { return mMemRefType; }

    //! is this a def mem ref
    bool isDef() { return (mMemRefType==DEF 
                           || mMemRefType == USEDEF || mMemRefType == DEFUSE); }

    //! is this a use mem ref
    bool isUse() { return (mMemRefType==USE 
                          || mMemRefType == USEDEF || mMemRefType == DEFUSE); }

    //! is this a defuse mem ref
    bool isDefUse() { return (mMemRefType==DEFUSE); }

    //! is this a usedef mem ref
    bool isUseDef() { return (mMemRefType==USEDEF); }

    //*****************************************************************
    // Construction methods 
    //*****************************************************************

    //! specify the memory reference type
    void setMemRefType(MemRefExpr::MemRefType mrType) { mMemRefType = mrType; }

    //*****************************************************************
    // Relationship methods 
    //*****************************************************************

    //! an ordering for locations, needed for use within STL containers
    virtual bool operator<(MemRefExpr & other);

    //! check if two memory references are equal at the level of
    //! accuracy provided by the MemRefExpr approximation
    virtual bool operator==(MemRefExpr& other);
    
    //! check if two memory references are not equal at the level of
    //! accuracy provided by the MemRefExpr approximation
    bool operator!=(MemRefExpr& other) { return ! ((*this)==other); }
    
    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const; 

    // helper functions for output
    virtual std::string typeString() { return "MemRefExpr"; }
    std::string toString(MemRefType) const;

    //*****************************************************************
    // Debugging methods 
    //*****************************************************************
    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR);
    virtual void dump(std::ostream& os, IRHandlesIRInterface& pIR);
    virtual void dump(std::ostream& os);
    virtual std::string comment(const IRHandlesIRInterface& pIR) const;

    virtual int getOrder() { assert(0); return sOrder; }

private:
    static const int sOrder = -100;
    MemRefType mMemRefType;
};

/*!
   A named memory reference has a SymHandle base. 
   Examples include references involving local and global variables.
*/
class NamedRef: public MemRefExpr {
  public:

    NamedRef(MemRefType mrType, SymHandle sh, bool slocal)
        : MemRefExpr(mrType), mSymHandle(sh), mStrictlyLocal(slocal) { }

    //! copy constructor
    NamedRef(NamedRef &mre) 
        : MemRefExpr(mre), mSymHandle(mre.mSymHandle), 
          mStrictlyLocal(mre.isStrictlyLocal()) 
        { }
    
    NamedRef(MemRefExpr &mre, SymHandle sh, bool slocal) 
        : MemRefExpr(mre), mSymHandle(sh), mStrictlyLocal(slocal) { }

    ~NamedRef() { }

    void acceptVisitor(MemRefExprVisitor& pVisitor);

    //! return a ptr to a copy of self
    OA_ptr<MemRefExpr> clone();

    //*****************************************************************
    // Subclass type methods 
    //*****************************************************************
    bool isaNamed() { return true; }
    
    //*****************************************************************
    // Info methods 
    //*****************************************************************
    SymHandle getSymHandle() { return mSymHandle; }
    virtual std::string typeString() { return "NamedRef"; }
    bool isStrictlyLocal() const { return mStrictlyLocal; }    
    //*****************************************************************
    // Relationship methods 
    //*****************************************************************

    bool operator<(MemRefExpr & other);

    //! check if two memory references are equal at the level of
    //! accuracy provided by the MemRefExpr approximation
    bool operator==(MemRefExpr& other);

    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;

    //*****************************************************************
    // Debugging methods 
    //*****************************************************************
    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR);
    virtual void dump(std::ostream& os, IRHandlesIRInterface& pIR);
    virtual void dump(std::ostream& os);
    virtual std::string comment(const IRHandlesIRInterface& pIR) const;

    virtual int getOrder() { return sOrder; }

  private:
    static const int sOrder = 100;
    SymHandle mSymHandle;
    bool mStrictlyLocal;
};

/*!
   An unnamed memory reference has a StmtHandle base. 
   Examples include references involving dynamically allocated locations.
*/
class UnnamedRef: public MemRefExpr {
  public:

    UnnamedRef(MemRefType mrType, ExprHandle sh, ProcHandle proc)
        : MemRefExpr(mrType), mExprHandle(sh), mLocal(true), mProcHandle(proc) { }


    UnnamedRef(MemRefType mrType, ExprHandle sh)
        : MemRefExpr(mrType), mExprHandle(sh), mLocal(false) { }


    //! copy constructor
    UnnamedRef(UnnamedRef &mre) : MemRefExpr(mre), 
                                  mExprHandle(mre.mExprHandle),
                                  mLocal(mre.mLocal),
                                  mProcHandle(mre.mProcHandle) {}
    
    UnnamedRef(MemRefExpr &mre, ExprHandle s, ProcHandle p)
        : MemRefExpr(mre), mExprHandle(s), mLocal(true), mProcHandle(p) {}


    UnnamedRef(MemRefExpr &mre, ExprHandle s)
        : MemRefExpr(mre), mExprHandle(s), mLocal(false) {}


    ~UnnamedRef() { }

    void acceptVisitor(MemRefExprVisitor& pVisitor);

    //! return a ptr to a copy of self
    OA_ptr<MemRefExpr> clone();

    //*****************************************************************
    // Subclass type methods 
    //*****************************************************************
    bool isaUnnamed() { return true; }
    
    //*****************************************************************
    // Info methods 
    //*****************************************************************
    ExprHandle getExprHandle() { return mExprHandle; }

    bool isLocal() const { return mLocal; }

    ProcHandle getProcHandle() { return mProcHandle; }
    
    //*****************************************************************
    // Relationship methods 
    //*****************************************************************

    bool operator<(MemRefExpr & other);

    //! check if two memory references are equal at the level of
    //! accuracy provided by the MemRefExpr approximation
    bool operator==(MemRefExpr& other);
    
    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;
    virtual std::string typeString() { return "UnnamedRef"; }

    //*****************************************************************
    // Debugging methods 
    //*****************************************************************
    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR);
    virtual void dump(std::ostream& os, IRHandlesIRInterface& pIR);
    virtual void dump(std::ostream& os);
    virtual std::string comment(const IRHandlesIRInterface& pIR) const;

    virtual int getOrder() { return sOrder; }

  private:
    static const int sOrder = 200;
    ExprHandle mExprHandle;
    bool mLocal;
    ProcHandle mProcHandle;
};

/*!
   An unknown memory reference has an unknown base.
   Examples include references involving involving arbitrary function
   calls, eg. *hello()
*/
class UnknownRef: public MemRefExpr {
  public:

    UnknownRef(MemRefType mrType)
        : MemRefExpr(mrType) { }

    //! copy constructor
    UnknownRef(UnknownRef &mre) : MemRefExpr(mre) {}

    ~UnknownRef() { }

    void acceptVisitor(MemRefExprVisitor& pVisitor);

    //! return a ptr to a copy of self
    OA_ptr<MemRefExpr> clone();

    //*****************************************************************
    // Subclass type methods 
    //*****************************************************************
    bool isaUnknown() { return true; }
    
    //*****************************************************************
    // Relationship methods 
    //*****************************************************************

    bool operator<(MemRefExpr & other);

    //! check if two memory references are equal at the level of
    //! accuracy provided by the MemRefExpr approximation
    bool operator==(MemRefExpr& other);
    
    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;
    virtual std::string typeString() { return "UnknownRef"; }

    //*****************************************************************
    // Debugging methods 
    //*****************************************************************
    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR);
    virtual void dump(std::ostream& os, IRHandlesIRInterface& pIR);
    virtual void dump(std::ostream& os);
    virtual std::string comment(const IRHandlesIRInterface& pIR) const;

    virtual int getOrder() { return sOrder; }

  private:
    static const int sOrder = 100000000;
};


/*!
   The RefOp implements a decorator pattern for memory references.
   If we decorate a memory reference with a RefOp it represents either
   a dereference or referencing a subset of a location (eg. field access
   or array access).
*/
class RefOp: public MemRefExpr {
  public:

    RefOp( MemRefType mrType, OA_ptr<MemRefExpr> mre)
        : MemRefExpr(mrType), mMRE(mre) { }
 

    //! copy constructor
    RefOp(RefOp & mre) : MemRefExpr(mre), mMRE(mre.mMRE) { }

    virtual ~RefOp() { }
    
    //*****************************************************************
    // MemRefExpr subclass type methods 
    //*****************************************************************
    bool isaRefOp() { return true; }

    //*****************************************************************
    // Info methods 
    //*****************************************************************
    //! iterate up every decorating MRE until we find the base, then
    //! return the symbol handle for this base.
    SymHandle getBaseSym();

    virtual std::string typeString() { return "RefOp"; }

    //! iterate up through every decorating MRE until we find the base
    //! and return it.
    OA_ptr<MemRefExpr> getBase();

    //! Return the MRE that decorating this object.
    OA_ptr<MemRefExpr> getMemRefExpr() { return mMRE; }

    //*****************************************************************
    // Relationship methods, will be defined in subclasses 
    //*****************************************************************

    //*****************************************************************
    // Construction Method
    //*****************************************************************
    
    //! Will make this Refop wrap the given mre and return the result
    virtual OA_ptr<MemRefExpr> composeWith(OA_ptr<MemRefExpr> mre) = 0;
    
    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    virtual void output(IRHandlesIRInterface& ir) const;


protected:
    OA_ptr<MemRefExpr> mMRE;

};


/* AddressOf class: represents if MemRefExpr has AddressTaken*/
class AddressOf: public RefOp { 
  public: 

    AddressOf(MemRefType mrType, OA_ptr<MemRefExpr> mre)
        : RefOp(mrType, mre) { }
  
    //! copy constructor 
    AddressOf(AddressOf & mre) : RefOp(mre) { } 
 
    ~AddressOf() { } 

    void acceptVisitor(MemRefExprVisitor& pVisitor);

    //! return a ptr to a copy of self
    OA_ptr<MemRefExpr> clone();

    virtual std::string typeString() { return "AddressOf"; }

    //*****************************************************************
    // RefOp subclass type methods
    //*****************************************************************
    bool isaAddressOf() { return true; }

    //*****************************************************************
    // Relationship methods
    //*****************************************************************

    bool operator<(MemRefExpr & other);

    //! check if two memory references are equal at the level of
    //! accuracy provided by the MemRefExpr approximation
    bool operator==(MemRefExpr& other);

    //*****************************************************************
    // Construction Method
    //*****************************************************************
    //! Will make this Refop wrap the given mre and return the result
    OA_ptr<MemRefExpr> composeWith(OA_ptr<MemRefExpr> mre);

    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;

    //*****************************************************************
    // Debugging methods
    //*****************************************************************
    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR);
    virtual void dump(std::ostream& os, IRHandlesIRInterface& pIR);
    virtual void dump(std::ostream& os);
    virtual std::string comment(const IRHandlesIRInterface& pIR) const;

    virtual int getOrder() { return sOrder; }

 private: 
    static const int sOrder = 350; 
}; 


/*!
   The Deref class indicates how many times a memory reference is 
   being dereferenced.
*/
class Deref: public RefOp {
  public:

    Deref(MemRefType mrType, OA_ptr<MemRefExpr> mre,
          int numDeref)
      : RefOp(mrType, mre), mNumDeref(numDeref) { }

    
    // default values
    Deref(MemRefType mrType, OA_ptr<MemRefExpr> mre)
      : RefOp(mrType, mre), mNumDeref(1) { }

    
    //! copy constructor
    Deref(Deref &mre) : RefOp(mre), mNumDeref(mre.mNumDeref) {}

    ~Deref() { }

    void acceptVisitor(MemRefExprVisitor& pVisitor);

    //! return a ptr to a copy of self
    OA_ptr<MemRefExpr> clone();

    //*****************************************************************
    // RefOp subclass type methods 
    //*****************************************************************
    bool isaDeref() { return true; }
    
    //*****************************************************************
    // Info methods 
    //*****************************************************************
    int getNumDerefs() { return mNumDeref; }
    virtual std::string typeString() { return "Deref"; }

    //*****************************************************************
    // Relationship methods
    //*****************************************************************
    
    bool operator<(MemRefExpr & other);

    //! check if two memory references are equal at the level of
    //! accuracy provided by the MemRefExpr approximation
    bool operator==(MemRefExpr& other);
    
    //*****************************************************************
    // Construction Method
    //*****************************************************************
    
    //! Will make this Refop wrap the given mre and return the result
    OA_ptr<MemRefExpr> composeWith(OA_ptr<MemRefExpr> mre);
    
    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;

    //*****************************************************************
    // Debugging methods 
    //*****************************************************************
    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR);
    virtual void dump(std::ostream& os, IRHandlesIRInterface& pIR);
    virtual void dump(std::ostream& os);
    virtual std::string comment(const IRHandlesIRInterface& pIR) const;

    virtual int getOrder() { return sOrder; }

private:
    static const int sOrder = 300;
    int mNumDeref;
};

/*!
   The SubSetRef implements a decorator pattern for memory references.
   If we decorate a memory reference with a subclass
   of SubSetRef it represents some operation such as a field 
   access or array access occuring.
   Many sub set reference operations are possible abstractions are possible.
*/
class SubSetRef: public RefOp {
  public:

    SubSetRef( MemRefType mrType, OA_ptr<MemRefExpr> mre)
      : RefOp(mrType, mre) {} //{ if (!mre.ptrEqual(0)) assert( ! mre->isaSubSetRef() ); }

    //! copy constructor
    SubSetRef(SubSetRef & mre) : RefOp(mre) { }

    virtual ~SubSetRef() { }

    void acceptVisitor(MemRefExprVisitor& pVisitor);

    //! return a ptr to a copy of self
    OA_ptr<MemRefExpr> clone();

    //*****************************************************************
    // RefOp subclass type methods 
    //*****************************************************************
    virtual bool isaSubSetRef() { return true; }
    virtual bool isSubClassOfSubSetRef() { return false; }
    
    //*****************************************************************
    // Relationship methods, will be defined in subclasses
    //*****************************************************************
    
    virtual bool operator<(MemRefExpr & other);

    //! check if two memory references are equal at the level of
    //! accuracy provided by the MemRefExpr approximation
    virtual bool operator==(MemRefExpr& other);
    
    //*****************************************************************
    // Construction Method
    //*****************************************************************
    
    //! Will make this Refop wrap the given mre and return the result
    OA_ptr<MemRefExpr> composeWith(OA_ptr<MemRefExpr> mre);
 
    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;
    virtual std::string typeString() { return "SubSetRef"; }

    //*****************************************************************
    // Debugging methods    
    //****************************************************************    
    virtual void dump(std::ostream& os, 
                       OA_ptr<IRHandlesIRInterface> pIR);  

    virtual void dump(std::ostream& os, IRHandlesIRInterface& pIR);
    virtual void dump(std::ostream& os);
    virtual std::string comment(const IRHandlesIRInterface& pIR) const;
    virtual int getOrder() { return sOrder; }          

private:
    static const int sOrder = 600;
};


/*!
   The IdxAccess class indicates a constant array index.
*/
class IdxAccess: public SubSetRef {
  public:

    IdxAccess(MemRefType mrType, OA_ptr<MemRefExpr> mre, int idx)
      : SubSetRef(mrType,mre), mIdx(idx) { }

    //! copy constructor
    IdxAccess(IdxAccess &mre) : SubSetRef(mre), mIdx(mre.mIdx) {}

    ~IdxAccess() { }

    void acceptVisitor(MemRefExprVisitor& pVisitor);

    //! return a ptr to a copy of self
    OA_ptr<MemRefExpr> clone();

    //*****************************************************************
    // SubSetRef subclass type methods 
    //*****************************************************************
    bool isaIdxAccess() { return true; }
    bool isSubClassOfSubSetRef() { return true; }
    
    //*****************************************************************
    // Info methods 
    //*****************************************************************
    int getIdx() { return mIdx; }
    virtual std::string typeString() { return "IdxAccess"; }

    //*****************************************************************
    // Relationship methods
    //*****************************************************************
    
    bool operator<(MemRefExpr & other);

    //! check if two memory references are equal at the level of
    //! accuracy provided by the MemRefExpr approximation
    bool operator==(MemRefExpr& other);
    
    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;

    //*****************************************************************
    // Debugging methods 
    //*****************************************************************
    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR);
    virtual void dump(std::ostream& os, IRHandlesIRInterface& pIR);
    virtual void dump(std::ostream& os);
    virtual std::string comment(const IRHandlesIRInterface& pIR) const;

    virtual int getOrder() { return sOrder; }

private:
    static const int sOrder = 400;
    int mIdx;
};

/*!
   The IdxExprAccess class indicates an array reference with an expression
   being used for an index.

   I'm assuming that all index expressions are affine.
*/
class IdxExprAccess : public SubSetRef {
  public:

    IdxExprAccess( MemRefType mrType, OA_ptr<MemRefExpr> mre,
                   MemRefHandle hExpr)
    : SubSetRef( mrType, mre)
    {
        mhExpr = hExpr;
    }

    //! copy constructor
    IdxExprAccess(IdxExprAccess &mre)
      : SubSetRef(mre),
        mhExpr(mre.mhExpr)
    {
    }

    ~IdxExprAccess() { }

    virtual void acceptVisitor(MemRefExprVisitor& pVisitor);

    //! return a ptr to a copy of self
    OA_ptr<MemRefExpr> clone();

    //*****************************************************************
    // SubSetRef subclass type methods 
    //*****************************************************************
    bool isaIdxExprAccess() { return true; }
    bool isSubClassOfSubSetRef() { return true; }

    //*****************************************************************
    // Info methods 
    //*****************************************************************
    MemRefHandle getExpr() { return mhExpr; }
    virtual std::string typeString() { return "IdxExprAccess"; }

    //*****************************************************************
    // Relationship methods
    //*****************************************************************
    bool operator<(MemRefExpr & other);

    //! check if two memory references are equal at the level of
    //! accuracy provided by the MemRefExpr approximation
    bool operator==(MemRefExpr& other);

    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;

    //*****************************************************************
    // Debugging methods
    //*****************************************************************
    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR);
    virtual void dump(std::ostream& os, IRHandlesIRInterface& pIR);
    virtual void dump(std::ostream& os);
    virtual std::string comment(const IRHandlesIRInterface& pIR) const;

    virtual int getOrder() { return sOrder; }

  private:
    static const int sOrder = 450;
    MemRefHandle mhExpr;
};

/*!
   The FieldAccess class indicates a field access to an object or struct,
   that is, a.b or a.foo(), not a->b nor b->foo().
*/
class FieldAccess: public SubSetRef {
  public:

    FieldAccess( MemRefType mrType,
            OA_ptr<MemRefExpr> mre, std::string field)
      : SubSetRef(mrType,mre), mFieldName(field) { }

   
    //! copy constructor
    FieldAccess(FieldAccess &mre) : SubSetRef(mre), mFieldName(mre.mFieldName)
        {}

    ~FieldAccess() { }

    void acceptVisitor(MemRefExprVisitor& pVisitor);

    //! return a ptr to a copy of self
    OA_ptr<MemRefExpr> clone();

    //*****************************************************************
    // SubSetRef subclass type methods 
    //*****************************************************************
    bool isaFieldAccess() { return true; }
    bool isSubClassOfSubSetRef() { return true; }
    
    //*****************************************************************
    // Info methods 
    //*****************************************************************
    std::string getFieldName() { return mFieldName; }
    virtual std::string typeString() { return "FieldAccess"; }

    //*****************************************************************
    // Relationship methods
    //*****************************************************************
    
    bool operator<(MemRefExpr & other);

    //! check if two memory references are equal at the level of
    //! accuracy provided by the MemRefExpr approximation
    bool operator==(MemRefExpr& other);
    
    //*****************************************************************
    // Annotation Interface
    //*****************************************************************
    void output(IRHandlesIRInterface& ir) const;

    //*****************************************************************
    // Debugging methods 
    //*****************************************************************
    virtual void dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> pIR);
    virtual void dump(std::ostream& os, IRHandlesIRInterface& pIR);
    virtual void dump(std::ostream& os);
    virtual std::string comment(const IRHandlesIRInterface& pIR) const;

    virtual int getOrder() { return sOrder; }

private:
    static const int sOrder = 500;
    std::string mFieldName;
};


} // end of OA namespace

#endif


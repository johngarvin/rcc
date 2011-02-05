/*! \file
  
  \brief Definition of IRHandles, abstract interfaces for iterators over 
         handles, and abstract IRInterface for generating debug output
         for the handles.

  \authors Nathan Tallent, Michelle Strout
  \version $Id: IRHandles.hpp,v 1.30.6.2 2005/09/19 05:25:17 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  Any iterator class over handles should be specified here.
  Otherwise, multiple analyses need to return the same type of
  iterator, but they can't because it doesn't appear that covariant
  returns are implemented in gcc compilers.  If the abstract iterator
  classes are declared here in the OA namespace, any IRInterface can
  have a method that returns one of these iterators.  Each Analyis
  IRInterface definition will probably only need a subset of the
  iterators to have a concrete subclass defined.  For example, the
  CFGIRInterface only requires that a subclass of IRRegionStmtIterator
  be defined.

  Reasoning for convert all typedef'd types into classes.
  Advantages:
  - Distinct types permits IRInterface function overloading
  - Inheritance shows the is-a relationship between some types
  - This should be identical to the typedefs in memory usage and
    runtime overhead (excepting run time type identification)
  - An instantiation of IRHandle (or any of the derived classes)
    should require the same amount of space as 'irhandle_t'.
  - Because there are no virtual functions (not even the destructor),
    there should be no need for the compiler to generate a dynamic virtual
    function table.

  Disadvantage:
  - Distinct types could lead to essentially duplicate templates.
    Possible Solution: Use IRHandle and dynamic casting.
*/

#ifndef IRHandles_H
#define IRHandles_H

//#define OA_IRHANDLETYPE_UL /* should be in CXXFLAGS */
#include <sstream>
#include <string>
#include <list>

#include <inttypes.h>

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <iostream>

//! Namespace for the whole OpenAnalysis Toolkit
namespace OA {

class IRHandlesIRInterface;


//***************************************************************************
//-----------------------------------------------------------------------------
// Below are generic types for handles to relate objects to a user's IR. 0 is
// reserved as a NULL value.  There are two sets of types: 
//  1) one whose type/size is relative to the current platform, and 
//  2) one whose type/size is maximum, which is useful for cross-platform tools
//
//   OA_IRHANDLETYPE_UL:  For handles on this platform
//   OA_IRHANDLETYPE_SZ64: Maximum size for cross-platform handles 
//-----------------------------------------------------------------------------
#if defined(OA_IRHANDLETYPE_UL)
  typedef unsigned long irhandle_t;
#elif defined(OA_IRHANDLETYPE_SZ64)
  typedef uint64_t irhandle_t;
#else
# error OpenAnalysis handle type must be specified!
#endif
  
//***************************************************************************


/*! 
   \brief Base class for all handles

   Do not make any of these methods virtual.  We do not want virtual
   table lookups to happen with these.
*/
class IRHandle : public Annotation {
public:
  IRHandle();
  IRHandle(irhandle_t h_);
  virtual ~IRHandle();

  IRHandle(const IRHandle& x) : h(x.h) { }
  IRHandle& operator=(const IRHandle& x) { h = x.h; return *this; }
  IRHandle& operator=(irhandle_t h_) { h = h_; return *this; }
  
  bool operator==(const IRHandle& x) const { return (h == x.h); }
  bool operator!=(const IRHandle& x)  const { return (h != x.h); }
  bool operator<(const IRHandle& x)  const { return (h < x.h); }
  bool operator>(const IRHandle& x)  const { return (h > x.h); }

  operator bool() { return h != 0; }
  operator std::string() { std::ostringstream oss;  oss << h; return oss.str(); }

  virtual void output(OA::IRHandlesIRInterface& ir) const;

  irhandle_t hval() const { return h; }

private:
  irhandle_t h;
};


class ProcHandle : public IRHandle {
public:
  ProcHandle();
  ProcHandle(irhandle_t h_) : IRHandle(h_) { }
  virtual ~ProcHandle();

  ProcHandle(const ProcHandle& x) : IRHandle(x) { }
  ProcHandle& operator=(const ProcHandle& x) 
    { IRHandle::operator=(x); return *this; }
  friend std::ostream &operator<<(
    std::ostream &os, const ProcHandle &proc);

  virtual void output(OA::IRHandlesIRInterface& ir) const;
};
std::ostream &operator<<(
    std::ostream &os,
    const OA::ProcHandle &proc);



class StmtHandle : public IRHandle {
public:
  StmtHandle();
  StmtHandle(irhandle_t h_) : IRHandle(h_) { }
  virtual ~StmtHandle();

  StmtHandle(const StmtHandle& x) : IRHandle(x) { }
  StmtHandle& operator=(const StmtHandle& x) 
    { IRHandle::operator=(x); return *this; }

  friend std::ostream &operator<<(
    std::ostream &os,
    const OA::StmtHandle &stmt);

  virtual void output(OA::IRHandlesIRInterface& ir) const;
};
std::ostream &operator<<(
    std::ostream &os,
    const OA::StmtHandle &stmt);


class StmtLabel : public IRHandle {
public:
  StmtLabel();
  StmtLabel(irhandle_t h_) : IRHandle(h_) { }
  virtual ~StmtLabel();

  StmtLabel(const StmtLabel& x) : IRHandle(x) { }
  StmtLabel& operator=(const StmtLabel& x) 
    { IRHandle::operator=(x); return *this; }
  friend std::ostream &operator<<(
    std::ostream &os, const StmtLabel &stmLbl);
  virtual void output(OA::IRHandlesIRInterface& ir) const;
};
std::ostream &operator<<(
    std::ostream &os,
    const OA::StmtLabel &stmLbl);


class SymHandle : public IRHandle {
public:
  SymHandle();
  SymHandle(irhandle_t h_) : IRHandle(h_) { }
  virtual ~SymHandle();

  SymHandle(const SymHandle& x) : IRHandle(x) { }
  SymHandle& operator=(const SymHandle& x) 
    { IRHandle::operator=(x); return *this; }
  friend std::ostream &operator<<(
    std::ostream &os, const SymHandle &sym);
  virtual void output(OA::IRHandlesIRInterface& ir) const;
};
std::ostream &operator<<(
    std::ostream &os,
    const OA::SymHandle &sym);


class ExprHandle : public IRHandle {
public:
  ExprHandle();
  ExprHandle(irhandle_t h_) : IRHandle(h_) { }
  virtual ~ExprHandle();

  ExprHandle(const ExprHandle& x) : IRHandle(x) { }
  ExprHandle& operator=(const ExprHandle& x) 
    { IRHandle::operator=(x); return *this; }
  friend std::ostream &operator<<(
    std::ostream &os, const ExprHandle &expr);
  virtual void output(OA::IRHandlesIRInterface& ir) const;
};
std::ostream &operator<<(
    std::ostream &os,
    const OA::ExprHandle &expr);


class MemRefHandle : public ExprHandle {
public:
  MemRefHandle();
  MemRefHandle(irhandle_t h_) : ExprHandle(h_) { }
  virtual ~MemRefHandle();

  MemRefHandle(const MemRefHandle& x) : ExprHandle(x) { }
  MemRefHandle& operator=(const MemRefHandle& x) 
    { IRHandle::operator=(x); return *this; }
  friend std::ostream &operator<<(
    std::ostream &os, const MemRefHandle &memref);
  virtual void output(OA::IRHandlesIRInterface& ir) const;
};
std::ostream &operator<<(
    std::ostream &os,
    const OA::MemRefHandle &memref);


class CallHandle : public ExprHandle {
public:
  CallHandle();
  CallHandle(irhandle_t h_) : ExprHandle(h_) { }
  virtual ~CallHandle();

  CallHandle(const CallHandle& x) : ExprHandle(x) { }
  CallHandle& operator=(const CallHandle& x) 
    { IRHandle::operator=(x); return *this; }
  friend std::ostream &operator<<(
    std::ostream &os, const CallHandle &call);
  virtual void output(OA::IRHandlesIRInterface& ir) const;
};
std::ostream &operator<<(
    std::ostream &os,
    const OA::CallHandle &call);


class OpHandle : public IRHandle {
public:
  OpHandle();
  OpHandle(irhandle_t h_) : IRHandle(h_) { }
  virtual ~OpHandle();

  OpHandle(const OpHandle& x) : IRHandle(x) { }
  OpHandle& operator=(const OpHandle& x) 
    { IRHandle::operator=(x); return *this; }
  friend std::ostream &operator<<(
    std::ostream &os, const OpHandle &op);
  virtual void output(OA::IRHandlesIRInterface& ir) const;
};
std::ostream &operator<<(
    std::ostream &os,
    const OA::OpHandle &op);


class ConstSymHandle : public ExprHandle {
public:
  ConstSymHandle();
  ConstSymHandle(irhandle_t h_) : ExprHandle(h_) { }
  virtual ~ConstSymHandle();

  ConstSymHandle(const ConstSymHandle& x) : ExprHandle(x) { }
  ConstSymHandle& operator=(const ConstSymHandle& x) 
    { IRHandle::operator=(x); return *this; }
  friend std::ostream &operator<<(
    std::ostream &os, const ConstSymHandle &constSym);
  virtual void output(OA::IRHandlesIRInterface& ir) const;
};
std::ostream &operator<<(
    std::ostream &os,
    const OA::ConstSymHandle &constSym);


class ConstValHandle : public ExprHandle {
public:
  ConstValHandle();
  ConstValHandle(const ConstValHandle& x);
  ConstValHandle(irhandle_t h_);
  virtual ~ConstValHandle();

  ConstValHandle& operator=(const ConstValHandle& x) 
    { IRHandle::operator=(x); return *this; }
  friend std::ostream &operator<<(
    std::ostream &os, const ConstValHandle &constVal);
  virtual void output(OA::IRHandlesIRInterface& ir) const;
};
std::ostream &operator<<(
    std::ostream &os,
    const OA::ConstValHandle &constVal);



/*!
   An expression that is also a leaf expression
   N.B.: All LeafHandles are also ExprHandles. We would like to be
   able to convert between them easily.
*/
// FIXME: needed?
class LeafHandle : public ExprHandle {
public:
  LeafHandle();
  LeafHandle(irhandle_t h_) : ExprHandle(h_) { }
  LeafHandle(ExprHandle x) : ExprHandle(x.hval()) { }
  virtual ~LeafHandle();

  LeafHandle(const LeafHandle& x) : ExprHandle(x) { }
  LeafHandle& operator=(const LeafHandle& x) 
    { IRHandle::operator=(x); return *this; }
  virtual void output(OA::IRHandlesIRInterface& ir) const;
};


//********************************************** All the iterators

//! Implemented iterator that any of the IRHandles could use
//! if IRHandles are being stored in a set.  Don't want
//! to use this as base class for all things that return an
//! iterator over various handles because it requires that
//! a pointer to a set be passed into the constructor
template <class T>
class IRHandleSetIterator {
public:
    IRHandleSetIterator (OA_ptr<std::set<T> > pSet): mSet(pSet), mIter(pSet->begin()) { }
    virtual ~IRHandleSetIterator () { }
    virtual void operator ++ () { if (mIter != mSet->end()) mIter++; }
    
    //! is the iterator at the end
    virtual bool isValid() const { return (mIter != mSet->end()); }
    
    //! return current node
    virtual T current() const { return (*mIter); }

    virtual void reset() { mIter = mSet->begin(); }

private:
    OA_ptr<std::set<T> > mSet;
    typename std::set<T>::iterator mIter;
};
 
//! Implemented iterator that any of the IRHandles could use
template <class T>
class IRHandleListIterator {
public:
    IRHandleListIterator (OA_ptr<std::list<T> > pList) : mList(pList) 
    { mIter = mList->begin(); }
    ~IRHandleListIterator () { }

    virtual void operator ++ () { if (mIter != mList->end()) mIter++; }
    
    //! is the iterator at the end
    virtual bool isValid() const { return (mIter != mList->end()); }
    
    //! return current node
    virtual T current() const { return (*mIter); }

    virtual void reset() { mIter = mList->begin(); }

private:
    OA_ptr<std::list<T> > mList;
    typename std::list<T>::iterator mIter;
};
 
//! Generic iterator over ProcHandle's
class ProcHandleIterator {
public:
  ProcHandleIterator();
  virtual ~ProcHandleIterator();

  virtual ProcHandle current() const = 0;  // Returns the current item.
  virtual bool isValid() const = 0;        // False when all items are exhausted.
        
  virtual void operator++() = 0;     // prefix
  void operator++(int) { ++*this; }  // postfix
  // FIXME: doesn't seem to work, for example Open64IRProcIterator 
  // doesn't see it, however InterActive.cpp:148 does see it

  virtual void reset() = 0;
};


/*!
 * Enumerate over all procedures in the IR.
 */
typedef ProcHandleIterator IRProcIterator;

/*
class IRProcIterator {
public:
  IRProcIterator() { }
  virtual ~IRProcIterator() { };

  virtual ProcHandle current() = 0;  // Returns the current item.
  virtual bool isValid() = 0;        // False when all items are exhausted.
        
  virtual void operator++() = 0;     // prefix
  void operator++(int) { ++*this; }  // postfix

  virtual void reset() = 0;
};
*/

//! Generic iterator over ExprHandle's
class ExprHandleIterator {
public:
  ExprHandleIterator();
  virtual ~ExprHandleIterator();

  virtual ExprHandle current() const = 0;  // Returns the current item.
  virtual bool isValid() const = 0;   // False when all items are exhausted.
        
  virtual void operator++() = 0;     // prefix
  void operator++(int) { ++*this; }  // postfix

  virtual void reset() = 0;
};

//! Enumerate all (actual) parameters within a callsite
//! Iterator should visit parameters in called order
typedef ExprHandleIterator IRCallsiteParamIterator;

//! Generic iterator over StmtHandles
class StmtHandleIterator {
public:
  StmtHandleIterator();
  virtual ~StmtHandleIterator();

  virtual StmtHandle current() const = 0;  // Returns the current item.
  virtual bool isValid()  const = 0;      // False when all items are exhausted.
        
  virtual void operator++() = 0;     // prefix
  void operator++(int) { ++*this; }  // postfix

  virtual void reset() = 0;
};


/*!
 * Enumerate all the statements in a program region, e.g. all the statements
 * in a procedure or a loop.  This iterator does NOT step into compound
 * statements.
 */
typedef StmtHandleIterator IRRegionStmtIterator;

/*!
 * Enumerate all the statements in a program 
 * This iterator DOES step into compound statements.
 */
typedef StmtHandleIterator IRStmtIterator;

//! Generic iterator over memory references
class MemRefHandleIterator {
public:
  MemRefHandleIterator();
  virtual ~MemRefHandleIterator();

  virtual MemRefHandle current() const = 0;  // Returns the current item.
  virtual bool isValid() const = 0;  // False when all items are exhausted.
        
  virtual void operator++() = 0;     // prefix
  void operator++(int) { ++*this; }  // postfix

  virtual void reset() = 0;
};

//! Generic iterator over memory references
class CallHandleIterator {
public:
  CallHandleIterator();
  virtual ~CallHandleIterator();

  virtual CallHandle current() const = 0;  // Returns the current item.
  virtual bool isValid() const = 0;  // False when all items are exhausted.
        
  virtual void operator++() = 0;     // prefix
  void operator++(int) { ++*this; }  // postfix

  virtual void reset() = 0;
};

/*!
 * Enumerate over all procedure calls in a statement
 */
typedef CallHandleIterator IRCallsiteIterator;

//! An iterator over symbols
class SymHandleIterator {
public:
  SymHandleIterator();
  virtual ~SymHandleIterator();

  virtual SymHandle current() const = 0;  // Returns the current item.
  virtual bool isValid() const = 0; // False when all items are exhausted.
        
  virtual void operator++() = 0;     // prefix
  void operator++(int) { ++*this; }  // postfix

  virtual void reset() = 0;
};

typedef SymHandleIterator IRSymIterator;
typedef SymHandleIterator IRFormalParamIterator;


//******************************************* Used to output handle data

class IRHandlesIRInterface {

public:
  virtual ~IRHandlesIRInterface();

  //--------------------------------------------------------
  // create a string for the given handle, should be succinct
  // and there should be no newlines
  //--------------------------------------------------------  
  virtual std::string toString(const ProcHandle h) const = 0;
  virtual std::string toString(const StmtHandle h) const= 0;
  virtual std::string toString(const ExprHandle h) const = 0;
  virtual std::string toString(const OpHandle h) const = 0;
  virtual std::string toString(const MemRefHandle h) const = 0;
  virtual std::string toString(const CallHandle h) const = 0;
  virtual std::string toString(const SymHandle h) const = 0;
  virtual std::string toString(const ConstSymHandle h) const = 0;
  virtual std::string toString(const ConstValHandle h) const = 0;

  // should deprecate this
  //virtual const std::string getName(OA::SymHandle h) = 0;

  //--------------------------------------------------------
  // dumps that do pretty prints
  //--------------------------------------------------------  

  //! Given a memory reference, pretty-print it to the output stream os.
  virtual void dump(MemRefHandle h, std::ostream& os) = 0;

  //! Given a statement, pretty-print it to the output stream os.
  virtual void dump(StmtHandle stmt, std::ostream& os) = 0;

  //! Cause Open64 to change its current global state to allow
  //! processing of arbitrary procedures (i.e., not all in a row)
  //virtual void currentProc(OA::ProcHandle p) = 0;

};

//***************************************************************************

/*
#include <iostream>
using std::cout;
using std::endl;

int 
main()
{
  ProcHandle p(1);
  ExprHandle e(2);
  LeafHandle l1((irhandle_t)&e);  // This is a little awkward...
  LeafHandle l2 = (irhandle_t)&e; // This is a little awkward...
  
  cout << "P: " << *p << endl;
  cout << "E: " << *e << endl;
  cout << "L1: " << *l1 << endl
       << "L2: " << *l2 << endl
       << "L1 == L2: " << ((l1 == l2) ? "true" : "false") << endl;
  
  
  ExprHandle ee1(l1);    // create expr from leaf
  ExprHandle ee2 = l1;   // assign a leaf to an expression
  ee2 = l1;              // assign a leaf to an expression
  
  LeafHandle ll1(ee1);   // create leaf from expr
  LeafHandle ll2 = ee1;  // assign expr to a leaf
  ll2 = ee1;             // assign expr to a leaf
  
  ExprHandle ee3 = *ll1; // initialize a expr with a leaf
  LeafHandle ll3 = *ee1; // initialize a leaf with an expr
  
  ExprHandle* ee4    = dynamic_cast<ExprHandle*>(&ll1); // ok
  // LeafHandle* ll4 = dynamic_cast<LeafHandle*>(&ee1); // error
  
  cout << "EE1: " << *ee1 << endl
       << "EE2: " << *ee2 << endl
       << "EE3: " << *ee3 << endl
       << "EE4: " << *(*ee4) << endl;
  cout << "LL1: " << *ll1 << endl
       << "LL2: " << *ll2 << endl
       << "LL3: " << *ll2 << endl;

  return 0;
}
*/

} // end of namespace OA

#endif

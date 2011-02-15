/*! \file
    
  \brief Iterators for Test IR Interface implementations

  \authors Michelle Strout
  \version $Id
 
  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <string>
#include <list>
#include <map>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>

#ifndef IRHANDLEITERATORS_H
#define IRHANDLEITERATORS_H

//! An iterator over a list of symbols 
class TestIRSymListIterator : public virtual OA::IRSymIterator,
                              public OA::IRHandleListIterator<OA::SymHandle>
{
public:
  TestIRSymListIterator(OA::OA_ptr<std::list<OA::SymHandle> > pList) 
    : OA::IRHandleListIterator<OA::SymHandle>(pList) {}
  ~TestIRSymListIterator() {}

  void operator++() { OA::IRHandleListIterator<OA::SymHandle>::operator++(); }
  bool isValid() const
    { return OA::IRHandleListIterator<OA::SymHandle>::isValid(); }
  OA::SymHandle current() const
    { return OA::IRHandleListIterator<OA::SymHandle>::current(); }
  void reset() { OA::IRHandleListIterator<OA::SymHandle>::reset(); }
};

//! An iterator over a set of symbols 
class TestIRSymSetIterator : public virtual OA::IRSymIterator,
                          public OA::IRHandleSetIterator<OA::SymHandle>
{
public:
  TestIRSymSetIterator(OA::OA_ptr<std::set<OA::SymHandle> > pSet) 
    : OA::IRHandleSetIterator<OA::SymHandle>(pSet) {}
  ~TestIRSymSetIterator() {}

  void operator++() { OA::IRHandleSetIterator<OA::SymHandle>::operator++(); }
  bool isValid() const
    { return OA::IRHandleSetIterator<OA::SymHandle>::isValid(); }
  OA::SymHandle current() const
    { return OA::IRHandleSetIterator<OA::SymHandle>::current(); }
  void reset() { OA::IRHandleSetIterator<OA::SymHandle>::reset(); }
};


//! An iterator over a list of memory references 
class TestIRMemRefListIterator : public virtual OA::MemRefHandleIterator,
                             public OA::IRHandleListIterator<OA::MemRefHandle>
{
public:
  TestIRMemRefListIterator(OA::OA_ptr<std::list<OA::MemRefHandle> > pList) 
    : OA::IRHandleListIterator<OA::MemRefHandle>(pList) {}
  ~TestIRMemRefListIterator() {}

  void operator++() { OA::IRHandleListIterator<OA::MemRefHandle>::operator++(); }
  bool isValid() const
    { return OA::IRHandleListIterator<OA::MemRefHandle>::isValid(); }
  OA::MemRefHandle current() const
    { return OA::IRHandleListIterator<OA::MemRefHandle>::current(); }
  void reset() { OA::IRHandleListIterator<OA::MemRefHandle>::reset(); }
};

//! An iterator over a set of memory references 
class TestIRMemRefSetIterator : public virtual OA::MemRefHandleIterator,
                             public OA::IRHandleSetIterator<OA::MemRefHandle>
{
public:
  TestIRMemRefSetIterator(OA::OA_ptr<std::set<OA::MemRefHandle> > pSet) 
    : OA::IRHandleSetIterator<OA::MemRefHandle>(pSet) {}
  ~TestIRMemRefSetIterator() {}

  void operator++() { OA::IRHandleSetIterator<OA::MemRefHandle>::operator++(); }
  bool isValid() const
    { return OA::IRHandleSetIterator<OA::MemRefHandle>::isValid(); }
  OA::MemRefHandle current() const
    { return OA::IRHandleSetIterator<OA::MemRefHandle>::current(); }
  void reset() { OA::IRHandleSetIterator<OA::MemRefHandle>::reset(); }
};


//! An iterator over stmts 
class TestIRStmtListIterator : public virtual OA::IRStmtIterator,
                           public OA::IRHandleListIterator<OA::StmtHandle>
{
public:
  TestIRStmtListIterator(OA::OA_ptr<std::list<OA::StmtHandle> > pList) 
    : OA::IRHandleListIterator<OA::StmtHandle>(pList) {}
  ~TestIRStmtListIterator() {}

  void operator++() { OA::IRHandleListIterator<OA::StmtHandle>::operator++(); }
  bool isValid() const
    { return OA::IRHandleListIterator<OA::StmtHandle>::isValid(); }
  OA::StmtHandle current() const
    { return OA::IRHandleListIterator<OA::StmtHandle>::current(); }
  void reset() { OA::IRHandleListIterator<OA::StmtHandle>::reset(); }
};

//! An iterator over expression 
class TestIRExprIterator : public virtual OA::ExprHandleIterator,
                           public OA::IRHandleListIterator<OA::ExprHandle>
{
public:
  TestIRExprIterator(OA::OA_ptr<std::list<OA::ExprHandle> > pList) 
    : OA::IRHandleListIterator<OA::ExprHandle>(pList) {}
  ~TestIRExprIterator() {}

  void operator++() { OA::IRHandleListIterator<OA::ExprHandle>::operator++(); }
  bool isValid() const
    { return OA::IRHandleListIterator<OA::ExprHandle>::isValid(); }
  OA::ExprHandle current() const
    { return OA::IRHandleListIterator<OA::ExprHandle>::current(); }
  void reset() { OA::IRHandleListIterator<OA::ExprHandle>::reset(); }
};


//! An iterator over memory reference expressions
class TestIRMemRefExprSetIterator : public OA::MemRefExprIterator {
public:
  TestIRMemRefExprSetIterator(OA::OA_ptr<std::set<OA::OA_ptr<OA::MemRefExpr> > > pSet)  : mSet(pSet) { mIter = mSet->begin(); }
  ~TestIRMemRefExprSetIterator() { }

  OA::OA_ptr<OA::MemRefExpr> current() const 
    { return *mIter; }
  bool isValid() const { return mIter != mSet->end(); }
        
  void operator++() { mIter++; }

  virtual void reset() { mIter = mSet->begin(); }
private:
  OA::OA_ptr<std::set<OA::OA_ptr<OA::MemRefExpr> > > mSet;
  std::set<OA::OA_ptr<OA::MemRefExpr> >:: iterator mIter;
};

//! An iterator over procs 
class TestIRProcListIterator : public virtual OA::IRProcIterator,
                           public OA::IRHandleListIterator<OA::ProcHandle>
{
public:
  TestIRProcListIterator(OA::OA_ptr<std::list<OA::ProcHandle> > pList) 
    : OA::IRHandleListIterator<OA::ProcHandle>(pList) {}
  ~TestIRProcListIterator() {}

  void operator++() { OA::IRHandleListIterator<OA::ProcHandle>::operator++(); }
  bool isValid() const
    { return OA::IRHandleListIterator<OA::ProcHandle>::isValid(); }
  OA::ProcHandle current() const
    { return OA::IRHandleListIterator<OA::ProcHandle>::current(); }
  void reset() { OA::IRHandleListIterator<OA::ProcHandle>::reset(); }
};

//! An iterator over procs 
class TestIRProcSetIterator : public virtual OA::IRProcIterator,
                           public OA::IRHandleSetIterator<OA::ProcHandle>
{
public:
  TestIRProcSetIterator(OA::OA_ptr<std::set<OA::ProcHandle> > pSet) 
    : OA::IRHandleSetIterator<OA::ProcHandle>(pSet) {}
  ~TestIRProcSetIterator() {}

  void operator++() { OA::IRHandleSetIterator<OA::ProcHandle>::operator++(); }
  bool isValid() const
    { return OA::IRHandleSetIterator<OA::ProcHandle>::isValid(); }
  OA::ProcHandle current() const
    { return OA::IRHandleSetIterator<OA::ProcHandle>::current(); }
  void reset() { OA::IRHandleSetIterator<OA::ProcHandle>::reset(); }
};


#endif

#ifndef R_SimpleIterators_hpp
#define R_SimpleIterators_hpp

#include <list>
#include <assert.h>
#include <rinternals.h>

//! Abstract class to enumerate a set of R expressions.
class R_ExpIterator
{
public:
  virtual ~R_ExpIterator() { }
  
  virtual SEXP current() const = 0;
  virtual bool isValid() const = 0;
  virtual void operator++() = 0;
  void operator++(int) { ++*this; }
  virtual void reset() = 0;
};

//! Singleton iterator for a single statement.
class R_SingletonIterator : public R_ExpIterator
{
protected:
  const SEXP exp;
  bool valid;
public:
  R_SingletonIterator(SEXP _exp) : exp(_exp) { valid = true; }

  SEXP current() const { return exp; }
  bool isValid() const { return valid; }
  void operator++() { valid = false; }
  void reset() { valid = true; }
};

//! Enumerate the elements of a list (in R, a chain of CONS cells).
//! To make sure all locations are unique, the iterator gives you the
//! cons cell; take the CAR to get the data you want.
class R_ListIterator : public R_ExpIterator
{
protected:
  const SEXP exp;
  SEXP curr;
public:
  R_ListIterator(SEXP _exp) : exp(_exp)
  {
    // make sure it's of list type: data cons cell, language cons cell, or nil
    assert(TYPEOF(exp) == LISTSXP || TYPEOF(exp) == LANGSXP || exp == R_NilValue);
    curr = exp;
  }
  virtual ~R_ListIterator() { }
  
  //  SEXP current() const { return CAR(curr); }
  SEXP current() const { return curr; }
  bool isValid() const { return (curr != R_NilValue); }
  void operator++()
  {
    // must be a data or language cons cell to be able to take the CDR
    assert(TYPEOF(curr) == LISTSXP || TYPEOF(curr) == LANGSXP);
    curr = CDR(curr);
  }
  void reset() { curr = exp; }
};

//! preorder traversal of an R object through CARs and CDRs
class R_PreorderIterator
{
private:
  std::list<SEXP> preorder;
  std::list<SEXP>::iterator iter;
  const SEXP exp;
  SEXP curr;
  void build_pre(SEXP e);
public:
  R_PreorderIterator(SEXP _exp) : exp(_exp)
  {
    build_pre(exp);
    iter = preorder.begin();
  }
  virtual ~R_PreorderIterator() { }
  
  SEXP current() const { return *iter; }
  bool isValid() const { return (iter != preorder.end()); }
  void operator++() { ++iter; }
  void reset() { iter = preorder.begin(); }
};

#endif

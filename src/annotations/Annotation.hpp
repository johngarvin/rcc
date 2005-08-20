// -*-Mode: C++;-*-
// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/Annotation.hpp,v 1.3 2005/08/20 21:54:41 garvin Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/annotations/Attic/Annotation.hpp,v $
//
// Purpose:
//   [The purpose of this file]
//
// Description:
//   [The set of functions, macros, etc. defined in the file]
//
//***************************************************************************

//************************* System Include Files ****************************

#include <map>
#include <set>
#include <string>

//**************************** R Include Files ******************************

#include <Rinternals.h>

//*************************** User Include Files ****************************

#include "AnnotationBase.hpp"

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RAnnot {

class VarInfo;

// N.B.: In order to make memory management sane, all Annotations
// should live within and owned by an AnnotationSet.  This allows
// Annotations to share pointers to other Annotations without
// worring about reference counting.

//****************************************************************************
// Annotations: Environments
//****************************************************************************

// ---------------------------------------------------------------------------
// Environment: 
// ---------------------------------------------------------------------------

class Environment 
  : public AnnotationBase
{
public:
  enum AllocT {
    Alloc_Heap,  // the heap
    Alloc_Frame, // the local frame of the execution stack
    Alloc_Stack  // separately managed stack of objects
  };

  typedef VarInfo*                                          key_type;
  typedef AllocT                                            mapped_type;
  
  typedef std::map<VarInfo*, AllocT>                        MyMap_t;
  typedef std::pair<const key_type, mapped_type>            value_type;
  typedef MyMap_t::key_compare                              key_compare;
  typedef MyMap_t::allocator_type                           allocator_type;
  typedef MyMap_t::reference                                reference;
  typedef MyMap_t::const_reference                          const_reference;
  typedef MyMap_t::iterator                                 iterator;
  typedef MyMap_t::const_iterator                           const_iterator;
  typedef MyMap_t::size_type                                size_type;

public:
  Environment();
  virtual ~Environment();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual Environment* clone() { return new Environment(*this); }

  // -------------------------------------------------------
  // iterator, find/insert, etc 
  // -------------------------------------------------------

  // iterators:
  iterator begin() 
    { return mVars.begin(); }
  const_iterator begin() const 
    { return mVars.begin(); }
  iterator end() 
    { return mVars.end(); }
  const_iterator end() const 
    { return mVars.end(); }
  
  // capacity:
  size_type size() const
    { return mVars.size(); }

  // element access:
  reference operator[](const key_type& x)
    { return operator[](x); }

  // modifiers:
  std::pair<iterator, bool> insert(const value_type& x)
    { return mVars.insert(x); }
  iterator insert(iterator position, const value_type& x)
    { return mVars.insert(position, x); }

  void erase(iterator position) 
    { mVars.erase(position); }
  size_type erase(const key_type& x) 
    { return mVars.erase(x); }
  void erase(iterator first, iterator last) 
    { return mVars.erase(first, last); }

  void clear() 
    { mVars.clear(); }

  // map operations:
  iterator find(const key_type& x)
    { return mVars.find(x); }
  const_iterator find(const key_type& x) const
    { return mVars.find(x); }
  size_type count(const key_type& x) const
    { return mVars.count(x); }
  
  // -------------------------------------------------------
  // code generation
  // -------------------------------------------------------
  // genCode1: generate code to initialize environment
  // genCode2...

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  MyMap_t mVars; // (contents of map not owned)
};


//****************************************************************************
// Annotations: Expressions
//****************************************************************************

// ---------------------------------------------------------------------------
// Expression: Abstract base class for expressions
// ---------------------------------------------------------------------------
class Expression
  : public AnnotationBase
{
public:
  Expression();
  virtual ~Expression();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual Expression* clone() { return new Expression(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
};


// ---------------------------------------------------------------------------
// Term: Base class for terminal expressions
// ---------------------------------------------------------------------------
class Term
  : public Expression
{
public:
  Term();
  virtual ~Term();
  
  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual Term* clone() { return new Term(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
};


// ---------------------------------------------------------------------------
// Var: A variable reference (includes uses and defs)
// ---------------------------------------------------------------------------
class Var
  : public Term
{
public:
  enum VarT {
    Var_USE,
    Var_DEF
  };

public:
  Var();
  virtual ~Var();
  
  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------
  
  // type
  VarT getType() const 
    { return mType; }
  void setType(VarT x)
    { mType = x; }

  // reaching-definition
  VarInfo* getReachingDef() const
    { return mReachingDef; }
  void setReachingDef(VarInfo* x)
    { mReachingDef = x; }
  
  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual Var* clone() { return new Var(*this); }

  // -------------------------------------------------------
  // code generation
  // -------------------------------------------------------
  //  generating a handle that can be used for accessing the value:
  //    if binding is unknown
  //      find the scope in which the var will be bound
  //      update the value in the proper scope
  //    if binding is condtional
  //      update full/empty bit
  
  //   genCode - phi: generate a handle that can be used to access value
  //     if all reaching defs do not have the same loc binding
  //       generate new handle bound to proper location
  //     if all reaching defs have same loc binding, do nothing
  void genCodeInit();
  void genCodeRHS();
  void genCodeLHS();

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  VarT mType;
  VarInfo* mReachingDef; // (not owned)
};


// ---------------------------------------------------------------------------
// FuncVar: A function reference (includes uses and defs)
// ---------------------------------------------------------------------------
class FuncVar
  : public Var
{
public:
  FuncVar();
  virtual ~FuncVar();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual FuncVar* clone() { return new FuncVar(*this); }

  // -------------------------------------------------------
  // code generation
  // -------------------------------------------------------

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  bool mIsReachingDefKnown;
};


// ---------------------------------------------------------------------------
// Literal: A literal value (assumed to be constant)
// ---------------------------------------------------------------------------
class Literal
  : public Term
{
public:
  Literal();
  virtual ~Literal();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual Literal* clone() { return new Literal(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
};


//****************************************************************************
// Annotations: Variable info
//****************************************************************************

// ---------------------------------------------------------------------------
// VarInfo: 'Definition' information about a variable
// ---------------------------------------------------------------------------
class VarInfo
  : public AnnotationBase
{
public:
  typedef Var*                                              key_type;

  typedef std::set<key_type>                                MySet_t;
  typedef key_type                                          value_type;
  typedef MySet_t::key_compare                              key_compare;
  typedef MySet_t::value_compare                            value_compare;
  typedef MySet_t::iterator                                 iterator;
  typedef MySet_t::const_iterator                           const_iterator;
  typedef MySet_t::size_type                                size_type;

public:
  VarInfo();
  virtual ~VarInfo();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual VarInfo* clone() { return new VarInfo(*this); }

  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------

  // definition
  SEXP getDef() const
    { return mDef; }
  void setDef(SEXP x)
    { mDef = x; }
  
  // is-binding-known
  bool getIsBindingKnown() const
    { return mIsBindingKnown; }
  void setIsBindingKnown(bool x)
    { mIsBindingKnown = x; }

  // environment
  Environment* getEnvironment() const
    { return mEnv; }
  void setEnvironment(Environment* x)
    { mEnv = x; }

  // uses iterators:
  iterator beginUses()
    { return mUses.begin(); }
  const_iterator beginUses() const
    { return mUses.begin(); }
  iterator endUses()
    { return mUses.end(); }
  const_iterator endUses() const
    { return mUses.end(); }
  
  // uses capacity:
  size_type sizeUses() const
    { return mUses.size(); }
  
  // uses modifiers:
  std::pair<iterator,bool> insertUses(const value_type& x)
    { return mUses.insert(x); }
  iterator insertUses(iterator position, const value_type& x)
    { return mUses.insert(position, x); }

  void eraseUses(iterator position)
    { mUses.erase(position); }
  size_type eraseUses(const key_type& x)
    { return mUses.erase(x); }
  void eraseUses(iterator first, iterator last)
    { return mUses.erase(first, last); }

  void clearUses()
    { mUses.clear(); }

  // uses set operations:
  iterator findUses(const key_type& x) const
    { return mUses.find(x); }
  size_type countUses(const key_type& x) const
    { return mUses.count(x); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  // data_type mType
  SEXP mDef;
  bool mIsBindingKnown;
  Environment* mEnv;
  MySet_t mUses;
  // scope of def // may be vector if upwardly exposed to cond defs
    // ???how do scopes compare to environmetns?
};


// ---------------------------------------------------------------------------
// FuncInfo: 'Definition' information about a function
// ---------------------------------------------------------------------------
class FuncInfo
  : public VarInfo
{
public:
  FuncInfo();
  virtual ~FuncInfo();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual FuncInfo* clone() { return new FuncInfo(*this); }

  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------
  
  // arguments
  unsigned int getNumArgs() const
    { return mNumArgs; }
  void setNumArgs(unsigned int x)
    { mNumArgs = x; }

  // has-variable-arguments
  bool getHasVarArgs() const
    { return mHasVarArgs; }
  void setHasVarArgs(bool x)
    { mHasVarArgs = x; }

  // c-linkage-name
  const std::string& getCName() const
    { return mCName; }
  void getCName(std::string& x)
    { mCName = x; }

  // environment

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  unsigned int mNumArgs; // number of known arguments
  bool mHasVarArgs;      // variable number of arguments
  std::string mCName;    // C linkage name
  Environment* mEnv;     // (not owned)
  // argument description: types, strict?
};


// ---------------------------------------------------------------------------
// Phi: 
// ---------------------------------------------------------------------------
class Phi
  : public VarInfo
{
public:
  typedef VarInfo*                                          key_type;

  typedef std::set<key_type>                                MySet_t;
  typedef key_type                                          value_type;
  typedef MySet_t::key_compare                              key_compare;
  typedef MySet_t::value_compare                            value_compare;
  typedef MySet_t::iterator                                 iterator;
  typedef MySet_t::const_iterator                           const_iterator;
  typedef MySet_t::size_type                                size_type;

public:
  Phi();
  virtual ~Phi();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual Phi* clone() { return new Phi(*this); }

  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------

  // reaching-defs iterators:
  iterator beginReachingDefs()
    { return mReachingDefs.begin(); }
  const_iterator beginReachingDefs() const
    { return mReachingDefs.begin(); }
  iterator endReachingDefs()
    { return mReachingDefs.end(); }
  const_iterator endReachingDefs() const
    { return mReachingDefs.end(); }
  
  // reaching-defs capacity:
  size_type sizeReachingDefs() const
    { return mReachingDefs.size(); }
  
  // reaching-defs modifiers:
  std::pair<iterator,bool> insertReachingDefs(const value_type& x)
    { return mReachingDefs.insert(x); }
  iterator insertReachingDefs(iterator position, const value_type& x)
    { return mReachingDefs.insert(position, x); }

  void eraseReachingDefs(iterator position)
    { mReachingDefs.erase(position); }
  size_type eraseReachingDefs(const key_type& x)
    { return mReachingDefs.erase(x); }
  void eraseReachingDefs(iterator first, iterator last)
    { return mReachingDefs.erase(first, last); }

  void clearReachingDefs()
    { mReachingDefs.clear(); }

  // reaching-defs set operations:
  iterator findReachingDefs(const key_type& x) const
    { return mReachingDefs.find(x); }
  size_type countReachingDefs(const key_type& x) const
    { return mReachingDefs.count(x); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  MySet_t mReachingDefs; // (set members not owned)
};


//****************************************************************************
// Annotations: Arguments
//****************************************************************************

// ---------------------------------------------------------------------------
// ArgInfo
// ---------------------------------------------------------------------------
class ArgInfo
  : public AnnotationBase
{
public:
  ArgInfo();
  virtual ~ArgInfo();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual ArgInfo* clone() { return new ArgInfo(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
};


// ---------------------------------------------------------------------------
// ActualArg
// ---------------------------------------------------------------------------
class ActualArg
  : public ArgInfo
{
public:
  ActualArg();
  virtual ~ActualArg();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual ActualArg* clone() { return new ActualArg(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  // value/promise
};


// ---------------------------------------------------------------------------
// FormalArg
// ---------------------------------------------------------------------------
class FormalArg
  : public ArgInfo
{
public:
  FormalArg();
  virtual ~FormalArg();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual FormalArg* clone() { return new FormalArg(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  // strict
};

//****************************************************************************

} // end of RAnnot namespace


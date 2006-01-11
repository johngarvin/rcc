// -*-Mode: C++;-*-

#ifndef ANNOTATION_ANNOTATION_H
#define ANNOTATION_ANNOTATION_H

// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/Attic/Annotation.h,v 1.13 2006/01/11 19:56:51 garvin Exp $

// * BeginCopyright *********************************************************
// *********************************************************** EndCopyright *

//***************************************************************************
//
// File:
//   $Source: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/Attic/Annotation.h,v $
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

#include <include/R/R_RInternals.h>

//*********************** OpenAnalysis Include Files ************************

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/CFG/Interface.hpp>

//*************************** User Include Files ****************************

#include <support/trees/NonUniformDegreeTreeTmpl.h>
#include <analysis/LocalityType.h>

#include "PropertyHndl.h"
#include "AnnotationBase.h"

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RAnnot {

// forward declarations

class VarInfo;
class Var;
class FuncInfo;

// N.B.: In order to make memory management sane, all Annotations
// should live within and owned by an AnnotationSet.  This allows
// Annotations to share pointers to other Annotations without
// worring about reference counting.


//****************************************************************************
// Annotations: Symbol tables
//****************************************************************************

// ---------------------------------------------------------------------------
// Name
// ---------------------------------------------------------------------------

typedef SEXP Name;  // should be of type SYMSXP

// ---------------------------------------------------------------------------
// Symbol table
// ---------------------------------------------------------------------------

class SymbolTable 
  : public AnnotationBase
{
public:
  enum AllocT {
    Alloc_Heap,  // the heap
    Alloc_Frame, // the local frame of the execution stack
    Alloc_Stack  // separately managed stack of objects
  };

  typedef Name                                              key_type;
  typedef VarInfo*                                          mapped_type;
  
  typedef std::map<Name, VarInfo*>                          MyMap_t;
  typedef std::pair<const key_type, mapped_type>            value_type;
  typedef MyMap_t::key_compare                              key_compare;
  typedef MyMap_t::allocator_type                           allocator_type;
  typedef MyMap_t::reference                                reference;
  typedef MyMap_t::const_reference                          const_reference;
  typedef MyMap_t::iterator                                 iterator;
  typedef MyMap_t::const_iterator                           const_iterator;
  typedef MyMap_t::size_type                                size_type;

public:
  SymbolTable();
  virtual ~SymbolTable();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual SymbolTable* clone() { return new SymbolTable(*this); }

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
  // genCode1: generate code to initialize ST
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
class ExpressionInfo
  : public AnnotationBase
{
public:
  static PropertyHndlT ExpressionInfoProperty;
public:
  ExpressionInfo();
  virtual ~ExpressionInfo();

  typedef Var *                   value_type, key_type;

  typedef std::set<Var *>         MySet_t;
  typedef MySet_t::iterator       iterator;
  typedef MySet_t::const_iterator const_iterator;

  // -------------------------------------------------------
  // set operations
  // -------------------------------------------------------
  std::pair<iterator, bool> insert_var(const value_type& x)
    { return mVars.insert(x); }

  // iterators:
  iterator begin() 
    { return mVars.begin(); }
  const_iterator begin() const 
    { return mVars.begin(); }
  iterator end() 
    { return mVars.end(); }
  const_iterator end() const 
    { return mVars.end(); }

  // definition of the expression
  SEXP getDefn() const
    { return mDefn; }
  void setDefn(SEXP x)
    { mDefn = x; }

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual ExpressionInfo* clone() { return new ExpressionInfo(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  SEXP mDefn;
  MySet_t mVars;  // contents of set not owned
};


// ---------------------------------------------------------------------------
// TermInfo: Base class for terminal expressions
// ---------------------------------------------------------------------------
class TermInfo
  : public ExpressionInfo
{
public:
  TermInfo();
  virtual ~TermInfo();
  
  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual TermInfo* clone() { return new TermInfo(*this); }

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
  : public TermInfo
{
public:
  static PropertyHndlT VarProperty;
public:
  enum UseDefT {
    Var_USE,
    Var_DEF
  };
  
  enum MayMustT {
    Var_MAY,
    Var_MUST 
  };

  enum ScopeT {
    Var_TOP,
    Var_LOCAL,
    Var_GLOBAL,
    Var_FREE_ONE_SCOPE,
    Var_FREE,
    Var_INDEFINITE
  };

public:
  Var();
  virtual ~Var();
  
  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------
  
  // use/def type
  UseDefT getUseDefType() const
    { return mUseDefType; }

  // may/must type
  MayMustT getMayMustType() const 
    { return mMayMustType; }
  void setMayMustType(MayMustT x)
    { mMayMustType = x; }

  // scope type
  ScopeT getScopeType() const
    { return mScopeType; }
  void setScopeType(ScopeT x)
    { mScopeType = x; }

  // reaching-definition
  VarInfo* getReachingDef() const
    { return mReachingDef; }
  void setReachingDef(VarInfo* x)
    { mReachingDef = x; }

  FuncInfo* getBoundScope() const
    { return mBoundScope; }
  void setBoundScope(FuncInfo* x)
    { mBoundScope = x; }
  
  // Mention
  SEXP getMention() const
    { return mSEXP; }
  void setMention(SEXP x)
    { mSEXP = x; }

  virtual SEXP getName() const = 0;

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  //virtual Var* clone() { return new Var(*this); }

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

protected:
  SEXP mSEXP;
  UseDefT mUseDefType;
  MayMustT mMayMustType;
  ScopeT mScopeType;
  VarInfo* mReachingDef; // (not owned)
  FuncInfo* mBoundScope;  // (not owned)
  
};

// ---------------------------------------------------------------------------
// UseVar: A variable reference that is a use
// ---------------------------------------------------------------------------
class UseVar
  : public Var
{
public:
  enum PositionT {
    UseVar_FUNCTION,
    UseVar_ARGUMENT
  };

public:
  UseVar();
  virtual ~UseVar();

  // function or argument position
  PositionT getPositionType() const
    { return mPositionType; }
  void setPositionType(PositionT x)
    { mPositionType = x; }

  virtual SEXP getName() const;

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual UseVar* clone() { return new UseVar(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  PositionT mPositionType;
};

// ---------------------------------------------------------------------------
// DefVar: A variable reference that is a def
// ---------------------------------------------------------------------------
class DefVar
  : public Var
{
public:
  enum SourceT {
    DefVar_ASSIGN,  // defined by an assignment statement
    DefVar_FORMAL,  // defined as a formal variable by a function
    DefVar_PHI      // defined by a phi function
  };
public:
  DefVar();
  virtual ~DefVar();

  // source of definition: assignment statement, formal argument, or phi function
  SourceT getSourceType() const
    { return mSourceType; }
  void setSourceType(SourceT x)
    { mSourceType = x; }

  virtual SEXP getName() const;

  SEXP getRhs() const
    { return mRhs; }
  void setRhs(SEXP x)
    { mRhs = x; }

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual DefVar* clone() { return new DefVar(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  SourceT mSourceType;
  SEXP mRhs;               // right side, if assignment
  // bool mIsBindingKnown;
  // set of uses reached?
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
  //virtual FuncVar* clone() { return new FuncVar(*this); }

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
  : public TermInfo
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
// VarInfo: Location information about a variable
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

  // symbol table
  SymbolTable* getSymbolTable() const
    { return mST; }
  void setSymbolTable(SymbolTable* x)
    { mST = x; }

  // FIXME: should this be a set of mentions instead of uses?

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

  // singleton status
  bool isSingleton() const
    { return mIsSingleton; }
  void setSingleton(bool x)
    { mIsSingleton = x; }

  FuncInfo* getSingletonDef() const
    { assert(mIsSingleton); return mSingletonDef; }
  void setSingletonDef(FuncInfo* x)
    { mSingletonDef = x; }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  // data_type mType
  SymbolTable* mST;
  MySet_t mUses;
  bool mIsSingleton;
  FuncInfo* mSingletonDef; // if it exists
  //  FuncInfo* mDefiningScope;  
};

//****************************************************************************
// Annotations: Function info
//****************************************************************************


// ---------------------------------------------------------------------------
// FuncInfo: 'Definition' information about a function
// ---------------------------------------------------------------------------
class FuncInfo : public NonUniformDegreeTreeNodeTmpl<FuncInfo>, 
		 public AnnotationBase
{
public:
  static PropertyHndlT FuncInfoProperty;

public:
  typedef Var*                                              mentions_key_type;

  typedef std::set<mentions_key_type>                       MySet_t;
  typedef MySet_t::iterator                                 mentions_iterator;
  typedef MySet_t::const_iterator                           const_mentions_iterator;

public:
  FuncInfo(FuncInfo *lexParent, SEXP name, SEXP defn);
  virtual ~FuncInfo();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual AnnotationBase* clone() { return 0; } // don't support this since it is linked! 

  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------
  
  // arguments
  unsigned int getNumArgs() const 
     { return mNumArgs; }
  void setNumArgs(unsigned int x) 
     { mNumArgs = x; }
  SEXP getArgs(); 
  bool areAllValue();
  bool isArgValue(SEXP arg);
  SEXP getArg(int position);
  int findArgPosition(char* name);

  // definition
  SEXP getDefn() 
     { return mDefn; }

  // has-variable-arguments
  bool getHasVarArgs() const
    { return mHasVarArgs; }
  void setHasVarArgs(bool x)
    { mHasVarArgs = x; }

  // c-linkage-name
  const std::string& getCName() const
    { return mCName; }
  void setCName(std::string& x)
    { mCName = x; }

  // context
  void setRequiresContext(bool requiresContext); 
  bool getRequiresContext(); 

  // mention set
  void insertMention(Var * v);

  // mention iterators
  mentions_iterator beginMentions()
    { return mMentions.begin(); }
  const_mentions_iterator beginMentions() const
    { return mMentions.begin(); }
  mentions_iterator endMentions()
    { return mMentions.end(); }
  const_mentions_iterator endMentions() const
    { return mMentions.end(); }

  // symbol table
  SymbolTable* getSymbolTable() const
    { return mST; }
  void setSymbolTable(SymbolTable* x)
    { mST = x; }

  // CFG
  OA::OA_ptr<OA::CFG::Interface> getCFG() const
    { return mCFG; }
  void setCFG(OA::OA_ptr<OA::CFG::Interface> x)
    { mCFG = x; }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;
  void dumpStmtsInCFG(std::ostream& os) const;

private:
  unsigned int mNumArgs;   // number of known arguments
  bool mHasVarArgs;        // variable number of arguments
  std::string mCName;      // C linkage name
  bool mRequiresContext;   // is an R context object needed for the function?
  SymbolTable* mST;       // (not owned)

  OA::OA_ptr<OA::CFG::Interface> mCFG; // control flow graph

  MySet_t mMentions;    // uses and defs inside function (including nested functions)

  SEXP mDefn;            // function definition
  SEXP mFirstName;       // name of function at original definition 
  FuncInfo *mLexicalParent; // parent scope definition

  // argument description: types, strict?
};

class FuncInfoChildIterator: public NonUniformDegreeTreeNodeChildIteratorTmpl<FuncInfo> {
public:
  FuncInfoChildIterator(const FuncInfo *fi, bool firstToLast = true) :
	NonUniformDegreeTreeNodeChildIteratorTmpl<FuncInfo>(fi, firstToLast) {};
};

class FuncInfoIterator: public NonUniformDegreeTreeIteratorTmpl<FuncInfo> {
public:
  FuncInfoIterator(const FuncInfo* fi, TraversalOrder torder = PreOrder,
				   NonUniformDegreeTreeEnumType how =
				     NON_UNIFORM_DEGREE_TREE_ENUM_ALL_NODES) :
	NonUniformDegreeTreeIteratorTmpl<FuncInfo>(fi, torder, how) {};
};


class FuncVarInfo
  : public VarInfo
{
public:
  FuncVarInfo();
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
// ActualArgInfo
// ---------------------------------------------------------------------------
class ActualArgInfo
  : public ArgInfo
{
public:
  ActualArgInfo();
  virtual ~ActualArgInfo();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual ActualArgInfo* clone() { return new ActualArgInfo(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  bool isvalue;  // value/promise
};


// ---------------------------------------------------------------------------
// FormalArgInfo
// ---------------------------------------------------------------------------
class FormalArgInfo
  : public ArgInfo
{
public:
  static PropertyHndlT FormalArgInfoProperty;
public:
  FormalArgInfo();
  virtual ~FormalArgInfo();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual FormalArgInfo* clone() { return new FormalArgInfo(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;
  
  bool isValue() { return isvalue; }
  void setIsValue(bool _isvalue) { isvalue = _isvalue; }

private:
  bool isvalue; // value/promise
};

//****************************************************************************

} // end of RAnnot namespace

#endif

// -*-Mode: C++;-*-

#ifndef ANNOTATION_ANNOTATION_H
#define ANNOTATION_ANNOTATION_H

// $Header: /home/garvin/cvs-svn/cvs-repos/developer/rcc/src/analysis/Attic/Annotation.h,v 1.20 2006/05/13 07:31:08 garvin Exp $

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

#include <support/RccError.h>
#include <support/trees/NonUniformDegreeTreeTmpl.h>

#include <analysis/AnnotationBase.h>
#include <analysis/LocalityType.h>
#include <analysis/PropertyHndl.h>

//*************************** Forward Declarations ***************************

//****************************************************************************

namespace RAnnot {

// forward declarations

class Var;
class FuncInfo;

// N.B.: In order to make memory management sane, all Annotations
// should live within and owned by an AnnotationMap.  This allows
// Annotations to share pointers to other Annotations without
// worring about reference counting.

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
  ExpressionInfo();
  virtual ~ExpressionInfo();

  typedef Var *                      MyVar_t;
  typedef std::list<MyVar_t>         MyVarSet_t;
  typedef MyVarSet_t::iterator       var_iterator;
  typedef MyVarSet_t::const_iterator const_var_iterator;

  typedef SEXP                            MyCallSite_t;
  typedef std::list<MyCallSite_t>         MyCallSiteSet_t;
  typedef MyCallSiteSet_t::iterator       call_site_iterator;
  typedef MyCallSiteSet_t::const_iterator const_call_site_iterator;

  // -------------------------------------------------------
  // set operations
  // -------------------------------------------------------
  void insert_var(const MyVar_t & x)
    { mVars.push_back(x); }
  void insert_call_site(const MyCallSite_t & x)
    { mCallSites.push_back(x); }

  // iterators:
  var_iterator begin_vars()
    { return mVars.begin(); }
  const_var_iterator begin_vars() const 
    { return mVars.begin(); }
  var_iterator end_vars()
    { return mVars.end(); }
  const_var_iterator end_vars() const 
    { return mVars.end(); }

  call_site_iterator begin_call_sites()
    { return mCallSites.begin(); }
  const_call_site_iterator begin_call_sites() const 
    { return mCallSites.begin(); }
  call_site_iterator end_call_sites()
    { return mCallSites.end(); }
  const_call_site_iterator end_call_sites() const 
    { return mCallSites.end(); }

  // definition of the expression
  SEXP getDefn() const
    { return mDefn; }
  void setDefn(SEXP x)
    { mDefn = x; }

  static PropertyHndlT handle();

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
  MyVarSet_t mVars;            // contents of set not owned
  MyCallSiteSet_t mCallSites;  // contents of set not owned
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
  enum UseDefT {
    Var_USE,
    Var_DEF
  };
  
  enum MayMustT {
    Var_MAY,
    Var_MUST 
  };

  enum ScopeT {
    //    Var_GLOBAL,         // information at this level moved to VarBinding
    //    Var_FREE_ONE_SCOPE,
    Var_TOP,
    Var_LOCAL,
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

  // Mention
  SEXP getMention() const
    { return mSEXP; }
  void setMention(SEXP x)
    { mSEXP = x; }

  virtual SEXP getName() const = 0;

  static PropertyHndlT handle();

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
};


const std::string typeName(const Var::UseDefT x);
const std::string typeName(const Var::MayMustT x);
const std::string typeName(const Var::ScopeT x);

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

const std::string typeName(const UseVar::PositionT x);


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

const std::string typeName(const DefVar::SourceT x);

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

  // uses iterators:
  iterator beginDefs()
    { return mDefs.begin(); }
  const_iterator beginDefs() const
    { return mDefs.begin(); }
  iterator endDefs()
    { return mDefs.end(); }
  const_iterator endDefs() const
    { return mDefs.end(); }
  
  // uses capacity:
  size_type sizeDefs() const
    { return mDefs.size(); }
  
  // uses modifiers:
  std::pair<iterator,bool> insertDef(const value_type& x)
    { return mDefs.insert(x); }
  iterator insertDef(iterator position, const value_type& x)
    { return mDefs.insert(position, x); }

  void eraseDefs(iterator position)
    { mDefs.erase(position); }
  size_type eraseDefs(const key_type& x)
    { return mDefs.erase(x); }
  void eraseDefs(iterator first, iterator last)
    { return mDefs.erase(first, last); }

  void clearDefs()
    { mDefs.clear(); }

  // uses set operations:
  iterator findDefs(const key_type& x) const
    { return mDefs.find(x); }
  size_type countDefs(const key_type& x) const
    { return mDefs.count(x); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  // data_type mType
  MySet_t mDefs;
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
  typedef Var*                                             MentionT;

  typedef std::list<MentionT>                              MentionSetT;
  typedef MentionSetT::iterator                            mention_iterator;
  typedef MentionSetT::const_iterator                      const_mention_iterator;

  typedef SEXP                                             CallSiteT;
  typedef std::list<CallSiteT>                             CallSiteSetT;
  typedef CallSiteSetT::iterator                           call_site_iterator;
  typedef CallSiteSetT::const_iterator                     const_call_site_iterator;
public:
  FuncInfo(FuncInfo *lexParent, SEXP name, SEXP defn);
  virtual ~FuncInfo();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual AnnotationBase* clone() { return 0; } // don't support this since it is linked! 

  static PropertyHndlT handle();

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

  // insert methods into sets
  void insertMention(MentionT v);
  void insertCallSite(CallSiteT e);

  // CFG
  OA::OA_ptr<OA::CFG::Interface> getCFG() const
    { return mCFG; }
  void setCFG(OA::OA_ptr<OA::CFG::Interface> x)
    { mCFG = x; }

  // mention iterators
  mention_iterator beginMentions()
    { return mMentions.begin(); }
  const_mention_iterator beginMentions() const
    { return mMentions.begin(); }
  mention_iterator endMentions()
    { return mMentions.end(); }
  const_mention_iterator endMentions() const
    { return mMentions.end(); }

  // call site iterators
  call_site_iterator beginCallSites()
    { return mCallSites.begin(); }
  const_call_site_iterator beginCallSites() const
    { return mCallSites.begin(); }
  call_site_iterator endCallSites()
    { return mCallSites.end(); }
  const_call_site_iterator endCallSites() const
    { return mCallSites.end(); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  unsigned int mNumArgs;   // number of known arguments
  bool mHasVarArgs;        // variable number of arguments
  std::string mCName;      // C linkage name
  bool mRequiresContext;   // is an R context object needed for the function?

  OA::OA_ptr<OA::CFG::Interface> mCFG; // control flow graph

  MentionSetT mMentions; // uses and defs inside function (NOT including nested functions)
  CallSiteSetT mCallSites; // call sites inside function (NOT including nested functions)

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
  FormalArgInfo();
  virtual ~FormalArgInfo();

  static PropertyHndlT handle();

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

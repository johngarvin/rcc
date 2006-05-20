// -*-Mode: C++;-*-

#ifndef ANNOTATION_VAR_H
#define ANNOTATION_VAR_H

#include <ostream>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationBase.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

// ---------------------------------------------------------------------------
// Var: A variable reference (includes uses and defs)
// ---------------------------------------------------------------------------
class Var
  : public AnnotationBase
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

  // Mention (cons cell whose CAR is the name)
  SEXP getMention_c() const
    { return mSEXP; }
  void setMention_c(SEXP x)
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

}

#endif

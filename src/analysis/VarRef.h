// -*- Mode: C++ -*-
#ifndef VAR_REF_H
#define VAR_REF_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>  // find out why forward ref won't work

#include <include/R/R_RInternals.h>

//! R_VarRef
//! An R_VarRef represents a mention (use or def) of a variable in R
//! code. Designed to be compared in different ways: two different
//! locations with the same name are "==" but not "equiv".  The name of
//! the mention is a SYMSXP in the R AST. Since all SYMSXPs with the
//! same name are the same pointer, we differentiate refs by
//! representing an R_VarRef as a CONS cell containing the symbol.
//! This is a base class: the way to get the name will be different
//! depending on what kind of reference it is.
class R_VarRef {
public:
  virtual ~R_VarRef() {};

  // Comparison operators for use in STL containers
  virtual bool operator<(R_VarRef & loc2) const = 0;
  virtual bool operator<(const R_VarRef & loc2) const = 0;
  virtual bool operator==(R_VarRef & loc2) const = 0;

  virtual bool equiv(R_VarRef & loc2) const = 0;

  virtual std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const = 0;
  virtual std::string toString() const = 0;

  virtual SEXP get_sexp() const = 0;
  virtual SEXP get_name() const = 0;
};

//! R_BodyVarRef
//! A VarRef found in code. A CONS cell where the CAR is the symbol.
class R_BodyVarRef : public R_VarRef {
public:
  R_BodyVarRef(SEXP _loc) : m_loc(_loc) {}
  
  // Comparison operators for use in STL containers
  bool operator<(R_VarRef & loc2) const;
  bool operator<(const R_VarRef & loc2) const;
  bool operator==(R_VarRef & loc2) const;

  bool equiv(R_VarRef & loc2) const;

  std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const;
  std::string toString() const;

  SEXP get_sexp() const;
  SEXP get_name() const;
  
private:
  SEXP m_loc;
};

//! R_ArgVarRef
//! A VarRef found as a formal argument. A CONS cell where the TAG is the symbol.
class R_ArgVarRef : public R_VarRef {
public:
  R_ArgVarRef(SEXP _loc) : m_loc(_loc) {}
  
  // Comparison operators for use in STL containers
  bool operator<(R_VarRef & loc2) const;
  bool operator<(const R_VarRef & loc2) const;
  bool operator==(R_VarRef & loc2) const;

  bool equiv(R_VarRef & loc2) const;

  std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const;
  std::string toString() const;

  SEXP get_sexp() const;
  SEXP get_name() const;

private:
  SEXP m_loc;
};

#endif

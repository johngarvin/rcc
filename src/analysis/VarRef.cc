#include <string>

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include "VarRef.h"

//--------------------------------------------------------------------
// R_BodyVarRef methods
//--------------------------------------------------------------------

// Comparison operators for use in STL containers

bool R_BodyVarRef::operator<(R_VarRef & loc2) const {
  return (get_name() < loc2.get_name());
}

bool R_BodyVarRef::operator<(const R_VarRef & loc2) const {
  // FIXME: find that chapter on how to const cast to get rid of repeated code
  return (get_name() < loc2.get_name());
}

//! Do the locations refer to the same name?
bool R_BodyVarRef::operator==(R_VarRef & loc2) const {
  return (get_name() == loc2.get_name());
}

//! Are they the same location?
bool R_BodyVarRef::equiv(R_VarRef & loc2) const {
  return (m_loc == loc2.get_sexp());
}

std::string R_BodyVarRef::toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const {
  return toString();
}

std::string R_BodyVarRef::toString() const {
  return std::string(CHAR(PRINTNAME(CAR(m_loc))));
}

SEXP R_BodyVarRef::get_sexp() const {
  return m_loc;
}

SEXP R_BodyVarRef::get_name() const {
  return CAR(m_loc);
}

//--------------------------------------------------------------------
// R_ArgVarRef methods
//--------------------------------------------------------------------

// Comparison operators for use in STL containers

bool R_ArgVarRef::operator<(R_VarRef & loc2) const {
  return (get_name() < loc2.get_name());
}

bool R_ArgVarRef::operator<(const R_VarRef & loc2) const {
  return (get_name() < loc2.get_name());
}

//! Do the locations refer to the same name?
bool R_ArgVarRef::operator==(R_VarRef & loc2) const {
  return (get_name() == loc2.get_name());
}

//! Are they the same location?
bool R_ArgVarRef::equiv(R_VarRef & loc2) const {
  return (m_loc == loc2.get_sexp());
}

std::string R_ArgVarRef::toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const {
  return toString();
}

std::string R_ArgVarRef::toString() const {
  return std::string(CHAR(PRINTNAME(TAG(m_loc))));
}

SEXP R_ArgVarRef::get_sexp() const {
  return m_loc;
}

SEXP R_ArgVarRef::get_name() const {
  return TAG(m_loc);
}

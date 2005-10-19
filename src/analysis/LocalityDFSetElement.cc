#include "LocalityDFSetElement.h"

using namespace OA;
using namespace RAnnot;

// constructors

namespace Locality {

DFSetElement::DFSetElement(OA_ptr<R_VarRef> _loc, LocalityType _type)
  : m_loc(_loc), m_type(_type)
  {}

DFSetElement::DFSetElement(const DFSetElement& other)
  : m_loc(other.m_loc), m_type(other.m_type)
  {}

// access

OA_ptr<R_VarRef> DFSetElement::get_loc() const { return m_loc; }
LocalityType DFSetElement::get_locality_type() const { return m_type; }


//! not doing a deep copy
OA_ptr<DFSetElement> DFSetElement::clone() { 
  OA_ptr<DFSetElement> retval;
  retval = new DFSetElement(*this);
  return retval;
}
  
//! copy an DFSetElement, not a deep copy, will refer to same Location
//! as other
DFSetElement& DFSetElement::operator=(const DFSetElement& other) {
  m_loc = other.get_loc();
  m_type = other.get_locality_type();
  return *this;
}

//! Equality operator for DFSetElement.  Just inspects location contents.
bool DFSetElement::operator== (const DFSetElement &other) const {
  return (m_loc==other.get_loc());
}

//! Inequality operator.
bool DFSetElement::operator!= (const DFSetElement &other) const {
  return !(*this==other);
}


//! Just based on location, this way when insert a new DFSetElement it can
//! override the existing DFSetElement with same location
bool DFSetElement::operator< (const DFSetElement &other) const { 
  return (m_loc < other.get_loc());
}

//! Equality method for DFSetElement.
bool DFSetElement::equiv(const DFSetElement& other) {
  return (m_loc == other.get_loc() && m_type == other.get_locality_type());
}

bool DFSetElement::sameLoc (const DFSetElement &other) const {
  return (m_loc == other.get_loc());
}

std::string DFSetElement::toString(OA_ptr<IRHandlesIRInterface> pIR) {
  return toString();
}

//! Return a string that contains a character representation of
//! an DFSetElement.
std::string DFSetElement::toString() {
  std::ostringstream oss;
  oss << "<";
  oss << m_loc->toString();
  switch (m_type) {
  case Locality_TOP:
    oss << ",TOP>"; break;
  case Locality_BOTTOM: 
    oss << ",BOTTOM>"; break;
  case Locality_LOCAL:
    oss << ",LOCAL>"; break;
  case Locality_FREE:
    oss << ",FREE>"; break;
  }
  return oss.str();
}

} // namespace Locality

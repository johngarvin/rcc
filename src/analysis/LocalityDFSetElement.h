// -*- Mode: C++ -*-

#ifndef LOCALITY_DF_SET_ELEMENT_H
#define LOCALITY_DF_SET_ELEMENT_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

#include <analysis/LocalityType.h>
#include <analysis/Annotation.h>
#include <analysis/VarRef.h>

namespace Locality {

//! Associates an R_VarRef with a LocalityType. This is the unit that
//! DFSet (which implements DataFlowSet) contains.
//! Modeled after ConstDef in ReachConstsStandard.hpp.
class DFSetElement {
public:
  // constructors
  DFSetElement(OA::OA_ptr<R_VarRef> _loc, LocalityType _type);
  DFSetElement(const DFSetElement& other);

  // access
  OA::OA_ptr<R_VarRef> get_loc() const;
  LocalityType get_locality_type() const;

  // relationships
  DFSetElement& operator= (const DFSetElement& other);

  //! not doing a deep copy
  OA::OA_ptr<DFSetElement> clone();
    
  //! operator== just compares content of loc
  bool operator== (const DFSetElement &other) const;
  //! method equiv compares all parts of DFSetElement as appropriate
  bool equiv(const DFSetElement& other);
  
  bool operator!= (const DFSetElement &other) const;
  bool operator< (const DFSetElement &other) const;
  bool sameLoc (const DFSetElement &other) const;

  std::string DFSetElement::toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir);
  std::string DFSetElement::toString();

private:
  OA::OA_ptr<R_VarRef> m_loc;
  LocalityType m_type;
};

} // namespace Locality

#endif

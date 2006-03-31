// -*- Mode: C++ -*-

#include <analysis/LocalVariableAnalysis.h>

#include "ExpressionInfoAnnotationMap.h"

namespace RAnnot {

// type definitions for readability
typedef ExpressionInfoAnnotationMap::MyKeyT MyKeyT;
typedef ExpressionInfoAnnotationMap::MyMappedT MyMappedT;
typedef ExpressionInfoAnnotationMap::iterator iterator;
typedef ExpressionInfoAnnotationMap::const_iterator const_iterator;

// ----- constructor/destructor ----- 
  
ExpressionInfoAnnotationMap::ExpressionInfoAnnotationMap(bool ownsAnnotations /* = true */)
: m_computed(false),
  m_map()
  {}

ExpressionInfoAnnotationMap::~ExpressionInfoAnnotationMap() {}

// ----- demand-driven analysis ----- 

// Subscripting is here temporarily to allow PutProperty -->
// PropertySet::insert to work right.
// FIXME: delete this when fully refactored to disallow insertion from outside.
MyMappedT & ExpressionInfoAnnotationMap::operator[](const MyKeyT & k) {
  return m_map[k];
}

// Perform the computation if necessary and return the requested data.
MyMappedT ExpressionInfoAnnotationMap::get(const MyKeyT & k) {
  if (!is_computed()) {
    compute(k);
    m_computed = true;
  }
  
  // after computing, an annotation ought to exist for every valid
  // key. No annotations means an error, most likely an invalid key.
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_map.find(k);
  if (annot == m_map.end()) {
    rcc_error("Possible invalid key not found in map");
  }

  return annot->second;
}
  
bool ExpressionInfoAnnotationMap::is_computed() {
  return m_computed;
}

//  ----- iterators ----- 

iterator ExpressionInfoAnnotationMap::begin() { return m_map.begin(); }
const_iterator ExpressionInfoAnnotationMap::begin() const { return m_map.begin(); }
iterator  ExpressionInfoAnnotationMap::end() { return m_map.end(); }
const_iterator ExpressionInfoAnnotationMap::end() const { return m_map.end(); }

// ----- computation -----

void ExpressionInfoAnnotationMap::compute(const MyKeyT & k) {
  const SEXP e = reinterpret_cast<const SEXP>(k.hval());
  LocalVariableAnalysis lva(e);
  LocalVariableAnalysis::const_iterator it;
  for(it = lva.begin(); it != lva.end(); ++it) {
    m_map[k] = *it;
  }
}  

}

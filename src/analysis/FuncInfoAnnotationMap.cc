// -*- Mode: C++ -*-

// Set of FuncInfo annotations representing function information,
// including the methods for computing the information. This should
// probably be split up into the different analyses at some point.

#include "FuncInfoAnnotationMap.h"

namespace RAnnot {

// type definitions for readability
typedef FuncInfoAnnotationMap::MyKeyT MyKeyT;
typedef FuncInfoAnnotationMap::MyMappedT MyMappedT;
typedef FuncInfoAnnotationMap::iterator iterator;
typedef FuncInfoAnnotationMap::const_iterator const_iterator;

//  ----- constructor/destructor ----- 

FuncInfoAnnotationMap::FuncInfoAnnotationMap(bool ownsAnnotations /* = true */)
: m_computed(false),
  m_map()
  {}

FuncInfoAnnotationMap::~FuncInfoAnnotationMap() {}

//  ----- demand-driven analysis ----- 

// Subscripting is here temporarily to allow PutProperty -->
// PropertySet::insert to work right.
// FIXME: delete this when fully refactored to disallow insertion from outside.
MyMappedT & FuncInfoAnnotationMap::operator[](const MyKeyT & k) {
  return m_map[k];
}

// Perform the computation if necessary and return the requested data.
MyMappedT FuncInfoAnnotationMap::get(const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  // after computing, an annotation ought to exist for every valid
  // key. If not, it's an error
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_map.find(k);
  if (annot == m_map.end()) {
    rcc_error("Possible invalid key not found in map");
  }

  return annot->second;
}

bool FuncInfoAnnotationMap::is_computed() {
  return m_computed;
}

//  ----- iterators ----- 

iterator FuncInfoAnnotationMap::begin() { return m_map.begin(); }
const_iterator  FuncInfoAnnotationMap::begin() const { return m_map.begin(); }
iterator  FuncInfoAnnotationMap::end() { return m_map.end(); }
const_iterator  FuncInfoAnnotationMap::end() const { return m_map.end(); }

// FIXME: implement this
void FuncInfoAnnotationMap::compute() {
}

}

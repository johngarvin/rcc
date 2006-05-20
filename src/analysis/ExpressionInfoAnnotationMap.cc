// -*- Mode: C++ -*-

#include <analysis/AnalysisResults.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/LocalVariableAnalysis.h>
#include <analysis/PropertySet.h>

#include "ExpressionInfoAnnotationMap.h"

namespace RAnnot {

// type definitions for readability
typedef ExpressionInfoAnnotationMap::MyKeyT MyKeyT;
typedef ExpressionInfoAnnotationMap::MyMappedT MyMappedT;
typedef ExpressionInfoAnnotationMap::iterator iterator;
typedef ExpressionInfoAnnotationMap::const_iterator const_iterator;

// ----- constructor/destructor ----- 

ExpressionInfoAnnotationMap::ExpressionInfoAnnotationMap(bool ownsAnnotations /* = true */)
  : m_map()
  {}

ExpressionInfoAnnotationMap::~ExpressionInfoAnnotationMap() {}

// ----- demand-driven analysis ----- 

// Subscripting is here temporarily to allow PutProperty -->
// PropertySet::insert to work right.
// FIXME: delete this when fully refactored to disallow insertion from outside.
MyMappedT & ExpressionInfoAnnotationMap::operator[](const MyKeyT & k) {
  std::map<MyKeyT, MyMappedT>::iterator annot = m_map.find(k);
  if (annot == m_map.end()) {
    compute(k);
    return m_map[k];
  } else {
    return annot->second; 
  }
}

/// Perform the computation for the given expression if necessary and
/// return the requested data.
MyMappedT ExpressionInfoAnnotationMap::get(const MyKeyT & k) {
  return (*this)[k];
}
  
/// Expression info is computed piece by piece, so it's never fully computed
bool ExpressionInfoAnnotationMap::is_computed() {
  return false;
}

//  ----- iterators ----- 

iterator ExpressionInfoAnnotationMap::begin() { return m_map.begin(); }
const_iterator ExpressionInfoAnnotationMap::begin() const { return m_map.begin(); }
iterator  ExpressionInfoAnnotationMap::end() { return m_map.end(); }
const_iterator ExpressionInfoAnnotationMap::end() const { return m_map.end(); }

// ----- computation -----

void ExpressionInfoAnnotationMap::compute(const MyKeyT & k) {
  SEXP e = HandleInterface::make_sexp(k);
  ExpressionInfo * annot = new ExpressionInfo();
  annot->setDefn(e);
  LocalVariableAnalysis lva(e);
  lva.perform_analysis();
  LocalVariableAnalysis::const_var_iterator v;
  for(v = lva.begin_vars(); v != lva.end_vars(); ++v) {
    annot->insert_var(*v);
  }
  LocalVariableAnalysis::const_call_site_iterator cs;
  for(cs = lva.begin_call_sites(); cs != lva.end_call_sites(); ++cs) {
    annot->insert_call_site(*cs);
  }
  
  m_map[k] = annot;
}

// ----- singleton pattern -----

ExpressionInfoAnnotationMap * ExpressionInfoAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT ExpressionInfoAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

// Create the singleton instance and register the map in PropertySet
// for getProperty
void ExpressionInfoAnnotationMap::create() {
  m_instance = new ExpressionInfoAnnotationMap(true);
  analysisResults.add(m_handle, m_instance);
}

ExpressionInfoAnnotationMap * ExpressionInfoAnnotationMap::m_instance = 0;
PropertyHndlT ExpressionInfoAnnotationMap::m_handle = "ExpressionInfo";

}

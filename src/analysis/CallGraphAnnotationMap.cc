#include "CallGraphAnnotationMap.h"

#include <analysis/AnalysisResults.h>

namespace RAnnot {

// ----- type definitions for readability -----
  
typedef CallGraphAnnotationMap::MyKeyT MyKeyT;
typedef CallGraphAnnotationMap::MyMappedT MyMappedT;
typedef CallGraphAnnotationMap::iterator iterator;
typedef CallGraphAnnotationMap::const_iterator const_iterator;
  
// ----- constructor, destructor -----

CallGraphAnnotationMap::CallGraphAnnotationMap()
  : m_node_map(), m_edge_set()
{}
  
CallGraphAnnotationMap::~CallGraphAnnotationMap() {
  CallGraphAnnotationMap::const_iterator i;
  
  // delete CallGraphInfo objects
  for(i = m_node_map.begin(); i != m_node_map.end(); ++i) {
    delete(i->second);
  }
  
  // delete edges
  std::set<const CallGraphEdge *>::const_iterator j;
  for(j = m_edge_set.begin(); j != m_edge_set.end(); ++j) {
    delete(*j);
  }
}  
  
// ----- singleton pattern -----

CallGraphAnnotationMap * CallGraphAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}
  
PropertyHndlT CallGraphAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}
  
void CallGraphAnnotationMap::create() {
  m_instance = new CallGraphAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}
  
CallGraphAnnotationMap * CallGraphAnnotationMap::m_instance = 0;
PropertyHndlT CallGraphAnnotationMap::m_handle = "CallGraph";
  
//  ----- demand-driven analysis ----- 

// Subscripting is here temporarily to allow PutProperty -->
// PropertySet::insert to work right.
// FIXME: delete this when fully refactored to disallow insertion from outside.
MyMappedT & CallGraphAnnotationMap::operator[](const MyKeyT & k) {
  return m_node_map[k];
}

// Perform the computation if necessary and returns the requested
// data.
MyMappedT CallGraphAnnotationMap::get(const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  // after computing, an annotation ought to exist for every valid
  // key. If not, it's an error
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_node_map.find(k);
  if (annot == m_node_map.end()) {
    rcc_error("Possible invalid key not found in map");
  }

  return annot->second;
}
  
bool CallGraphAnnotationMap::is_computed() {
  return m_computed;
}

//  ----- iterators ----- 

iterator CallGraphAnnotationMap::begin() { return m_node_map.begin(); }
iterator CallGraphAnnotationMap::end() { return m_node_map.end(); }
const_iterator CallGraphAnnotationMap::begin() const { return m_node_map.begin(); }
const_iterator CallGraphAnnotationMap::end() const { return m_node_map.end(); }

// ----- computation -----  

void CallGraphAnnotationMap::compute() {
  // TODO: implement
}

} // end namespace RAnnot

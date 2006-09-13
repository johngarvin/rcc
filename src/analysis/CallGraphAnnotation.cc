#include "CallGraphAnnotation.h"

#include <analysis/CallGraphAnnotationMap.h>

namespace RAnnot {

  // typedef for readability
  typedef CallGraphAnnotation::MySetT MySetT;

  CallGraphAnnotation::CallGraphAnnotation()
  {}

  CallGraphAnnotation::~CallGraphAnnotation()
  {}

  // ----- methods to insert nodes -----

  void CallGraphAnnotation::insert_node(const CallGraphAnnotationMap::CallGraphNode * const node) {
    m_nodes.insert(node);
  }

  // ----- iterators -----

  MySetT::const_iterator CallGraphAnnotation::begin() const {
    return m_nodes.begin();
  }

  MySetT::const_iterator CallGraphAnnotation::end() const {
    return m_nodes.end();
  }

  // ----- get_singleton_if_exists -----

  const CallGraphAnnotationMap::CallGraphNode * CallGraphAnnotation::get_singleton_if_exists() const {
    if (begin() == end()) return 0;
    MySetT::const_iterator elt1 = begin();
    MySetT::const_iterator elt2 = begin();
    elt2++;
    if (elt2 == end()) {  // set contains one element
      return *elt1;
    } else {
      return 0;
    }
  }

  // ----- AnnotationBase -----

  AnnotationBase * CallGraphAnnotation::clone() {
    return 0; // don't support this
  }

  PropertyHndlT CallGraphAnnotation::handle() {
    return CallGraphAnnotationMap::handle();
  }

  std::ostream & CallGraphAnnotation::dump(std::ostream & stream) const {
    MySetT::const_iterator it;
    stream << "Nodes:" << std::endl;
    for (it = begin(); it != end(); ++it) {
      stream << "node" << std::endl;
    }
  }

} // end namespace RAnnot

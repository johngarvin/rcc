#include "CallGraphInfo.h"

#include <analysis/CallGraphAnnotationMap.h>

namespace RAnnot {

  // typedef for readability
  typedef CallGraphInfo::MySetT MySetT;

  CallGraphInfo::CallGraphInfo()
  {}

  CallGraphInfo::~CallGraphInfo()
  {}

  // ----- methods to insert edges -----

  void CallGraphInfo::insert_call_in(const CallGraphEdge * const edge) {
    m_calls_in.insert(edge);
  }

  void CallGraphInfo::insert_call_out(const CallGraphEdge * const edge) {
    m_calls_out.insert(edge);
  }

  // ----- iterators -----

  MySetT::const_iterator CallGraphInfo::begin_calls_in() const {
    return m_calls_in.begin();
  }

  MySetT::const_iterator CallGraphInfo::end_calls_in() const {
    return m_calls_in.end();
  }

  MySetT::const_iterator CallGraphInfo::begin_calls_out() const {
    return m_calls_out.begin();
  }

  MySetT::const_iterator CallGraphInfo::end_calls_out() const {
    return m_calls_out.end();
  }

  // ----- AnnotationBase -----

  AnnotationBase * CallGraphInfo::clone() {
    return 0; // don't support this
  }

  PropertyHndlT CallGraphInfo::handle() {
    return CallGraphAnnotationMap::handle();
  }

  std::ostream & CallGraphInfo::dump(std::ostream & stream) const {
    MySetT::const_iterator it;
    stream << "Calls in:" << std::endl;
    for (it = begin_calls_in(); it != end_calls_in(); ++it) {
      stream << "edge" << std::endl;
    }
    stream << "Calls out:" << std::endl;
    for (it = begin_calls_out(); it != end_calls_out(); ++it) {
      stream << "edge" << std::endl;
    }
    stream << "End call graph node";
  }

} // end namespace RAnnot

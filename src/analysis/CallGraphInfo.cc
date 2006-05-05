#include "CallGraphInfo.h"

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

  std::ostream & CallGraphInfo::dump(std::ostream & stream) const {
    // TODO: implement
  }

} // end namespace RAnnot

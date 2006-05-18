#include "CallGraphInfo.h"

#include <support/DumpMacros.h>

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

  // ----- debug -----

  std::ostream & CallGraphInfo::dump(std::ostream & stream) const {
    MySetT::const_iterator it;

    if (m_calls_in.empty()) {
      stream << "Calls in: <empty>" << std::endl;
    } else {
      stream << "Calls in:" << std::endl;
      for (it = begin_calls_in(); it != end_calls_in(); ++it) {
	const CallGraphEdge * edge = *it;
	dumpPtr(stream, edge->get_source());
      }
    }

    if (m_calls_out.empty()) {
      stream << "Calls out: <empty>" << std::endl;
    } else {
      stream << "Calls out:" << std::endl;
      for (it = begin_calls_out(); it != end_calls_out(); ++it) {
	const CallGraphEdge * edge = *it;
	dumpPtr(stream, edge->get_sink());
      }
    }
  }

} // end namespace RAnnot

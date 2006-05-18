// -*- Mode: C++ -*-

#ifndef CALL_GRAPH_INFO_H
#define CALL_GRAPH_INFO_H

#include <set>

#include <analysis/CallGraphEdge.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class CallGraphInfo {
public:
  typedef std::set<const CallGraphEdge *> MySetT;
  typedef MySetT::const_iterator const_iterator;

  explicit CallGraphInfo();
  virtual ~CallGraphInfo();

  void insert_call_in(const CallGraphEdge * const edge);
  void insert_call_out(const CallGraphEdge * const edge);

  const_iterator begin_calls_in() const;
  const_iterator end_calls_in() const;
  const_iterator begin_calls_out() const;
  const_iterator end_calls_out() const;

  std::ostream & dump(std::ostream & stream) const;

private:
  MySetT m_calls_in;
  MySetT m_calls_out;
};

} // end namespace RAnnot

#endif

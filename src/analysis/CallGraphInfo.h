// -*- Mode: C++ -*-

#ifndef CALL_GRAPH_INFO_H
#define CALL_GRAPH_INFO_H

#include <set>

#include <analysis/AnnotationBase.h>
#include <analysis/CallGraphEdge.h>

namespace RAnnot {

class CallGraphInfo : public AnnotationBase {
public:
  typedef std::set<const CallGraphEdge *> MySetT;

  explicit CallGraphInfo();
  virtual ~CallGraphInfo();

  void insert_call_in(const CallGraphEdge * const edge);
  void insert_call_out(const CallGraphEdge * const edge);

  MySetT::const_iterator begin_calls_in() const;
  MySetT::const_iterator end_calls_in() const;
  MySetT::const_iterator begin_calls_out() const;
  MySetT::const_iterator end_calls_out() const;

  AnnotationBase * clone();
  
  std::ostream & dump(std::ostream & stream) const;

private:
  MySetT m_calls_in;
  MySetT m_calls_out;
};

} // end namespace RAnnot

#endif

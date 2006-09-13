// -*- Mode: C++ -*-

#ifndef CALL_GRAPH_ANNOTATION_H
#define CALL_GRAPH_ANNOTATION_H

#include <set>

#include <analysis/AnnotationBase.h>
#include <analysis/CallGraphEdge.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class CallGraphAnnotation : public AnnotationBase {
public:
  typedef std::set<const CallGraphAnnotationMap::CallGraphNode *> MySetT;
  typedef MySetT::const_iterator const_iterator;

  explicit CallGraphAnnotation();
  virtual ~CallGraphAnnotation();

  void insert_node(const CallGraphAnnotationMap::CallGraphNode * const node);

  const_iterator begin() const;
  const_iterator end() const;

  const CallGraphAnnotationMap::CallGraphNode * get_singleton_if_exists() const;

  // ----- methods that implement AnnotationBase -----

  AnnotationBase * clone();

  static PropertyHndlT handle();

  // ----- debugging -----

  std::ostream & dump(std::ostream & stream) const;

private:
  MySetT m_nodes;
};

} // end namespace RAnnot

#endif

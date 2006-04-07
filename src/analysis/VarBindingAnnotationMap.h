// -*-Mode: C++;-*-

#ifndef VAR_BINDING_ANNOTATION_MAP_H
#define VAR_BINDING_ANNOTATION_MAP_H

// Set of VarBinding annotations that describe the binding scopes of
// variables.

#include <map>

#include <analysis/AnnotationMap.h>

namespace RAnnot {

class VarBindingAnnotationMap : public AnnotationMap {
public:
  // constructor/deconstructor
  VarBindingAnnotationMap(bool ownsAnnotations = true);
  virtual ~VarBindingAnnotationMap();

  // demand-driven analysis
  MyMappedT get(const MyKeyT & k);
  bool is_computed();

  // iterators
  // TODO: switch to OA-style iterator
#if 0
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
#endif

private:
  void compute();

private:
  bool m_computed; // has our information been computed yet?
  std::map<MyKeyT, MyMappedT> m_map;
};

}

#endif

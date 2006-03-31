// -*-Mode: C++;-*-

#ifndef VAR_BINDING_ANNOTATION_MAP_H
#define VAR_BINDING_ANNOTATION_MAP_H

// Set of VarBinding annotations that describe the binding scopes of
// variables.

#include <analysis/AnnotationMap.h>

namespace RAnnot {

class VarBindingAnnotationMap : public AnnotationMap {
public:
  // constructor/deconstructor
  VarBindingAnnotationMap(bool ownsAnnotations = true);
  virtual ~VarBindingAnnotationMap();

  // analysis
  virtual void compute_if_needed();

private:
  bool m_computed; // has our information been computed yet?
  void compute_bound_scopes();
};

}

#endif

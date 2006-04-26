// -*-Mode: C++;-*-

#ifndef VAR_BINDING_ANNOTATION_MAP_H
#define VAR_BINDING_ANNOTATION_MAP_H

// Set of VarBinding annotations that describe the binding scopes of
// variables.

#include <map>

#include <OpenAnalysis/Utils/Iterator.hpp>

#include <analysis/AnnotationMap.h>

namespace RAnnot {

class VarBindingAnnotationMap : public AnnotationMap
{
public:
  // constructor/deconstructor
  VarBindingAnnotationMap(bool ownsAnnotations = true);
  virtual ~VarBindingAnnotationMap();

  // singleton pattern
  static VarBindingAnnotationMap * get_instance();
  static PropertyHndlT handle();

  // demand-driven analysis
  MyMappedT & operator[](const MyKeyT & k);
  MyMappedT get(const MyKeyT & k);
  bool is_computed();

  // iterators
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

private:
  void compute();

  static void create();

private:
  bool m_computed; // has our information been computed yet?
  std::map<MyKeyT, MyMappedT> m_map;

  static VarBindingAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
};

}

#endif

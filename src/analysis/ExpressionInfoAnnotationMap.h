// -*-Mode: C++;-*-

#ifndef EXPRESSION_INFO_ANNOTATION_MAP_H
#define EXPRESSION_INFO_ANNOTATION_MAP_H

// Set of ExpressionInfo annotations.

#include <map>

#include <analysis/AnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class ExpressionInfoAnnotationMap : public AnnotationMap {
public:
  // deconstructor
  virtual ~ExpressionInfoAnnotationMap();

  // singleton
  static ExpressionInfoAnnotationMap * get_instance();

  // getting the handle causes this map to be created and registered
  static PropertyHndlT handle();

  // demand-driven analysis
  MyMappedT & operator[](const MyKeyT & k); // FIXME: remove this when refactoring is done
  MyMappedT get(const MyKeyT & k);
  bool is_computed();

  // iterators
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

private:
  // singleton: only this class is allowed to instantiate
  ExpressionInfoAnnotationMap(bool ownsAnnotations = true);

  void compute(const MyKeyT & k);

  std::map<MyKeyT, MyMappedT> m_map;

  // static members and methods for singleton
  static ExpressionInfoAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();
};

}

#endif

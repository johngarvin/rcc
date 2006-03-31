// -*-Mode: C++;-*-

#ifndef EXPRESSION_INFO_ANNOTATION_MAP_H
#define EXPRESSION_INFO_ANNOTATION_MAP_H

// Set of ExpressionInfo annotations.

#include <map>

#include <analysis/AnnotationMap.h>

namespace RAnnot {

class ExpressionInfoAnnotationMap : public AnnotationMap {
public:
  // constructor/deconstructor
  ExpressionInfoAnnotationMap(bool ownsAnnotations = true);
  virtual ~ExpressionInfoAnnotationMap();

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
  void compute(const MyKeyT & k);

private:
  bool m_computed; // has our information been computed yet?
  std::map<MyKeyT, MyMappedT> m_map;
};

}

#endif

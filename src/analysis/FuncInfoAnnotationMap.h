// -*-Mode: C++;-*-

#ifndef FUNC_INFO_ANNOTATION_MAP_H
#define FUNC_INFO_ANNOTATION_MAP_H

// Set of FuncInfo annotations representing function information,
// including the methods for computing the information. This should
// probably be split up into the different analyses at some point.

#include <map>

#include <analysis/AnnotationMap.h>

namespace RAnnot {

class FuncInfoAnnotationMap : public AnnotationMap {
public:
  // constructor/deconstructor
  FuncInfoAnnotationMap(bool ownsAnnotations = true);
  virtual ~FuncInfoAnnotationMap();

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
  void compute();

private:
  bool m_computed; // has our information been computed yet?
  std::map<MyKeyT, MyMappedT> m_map;

};

}

#endif

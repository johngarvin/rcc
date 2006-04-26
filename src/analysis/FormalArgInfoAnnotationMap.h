// -*-Mode: C++;-*-

#ifndef FORMAL_ARG_INFO_ANNOTATION_MAP_H
#define FORMAL_ARG_INFO_ANNOTATION_MAP_H

// Set of FormalArgInfo annotations.

#include <map>

#include <analysis/AnnotationMap.h>

namespace RAnnot {

class FormalArgInfoAnnotationMap : public AnnotationMap {
public:
  // constructor/deconstructor
  FormalArgInfoAnnotationMap(bool ownsAnnotations = true);
  virtual ~FormalArgInfoAnnotationMap();

  // demand-driven analysis
  MyMappedT & operator[](const MyKeyT & k); // FIXME: remove this when refactoring is done
  MyMappedT get(const MyKeyT & k);
  bool is_computed();

  // singleton
  static FormalArgInfoAnnotationMap * get_instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

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
  
  static FormalArgInfoAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();
};

}

#endif

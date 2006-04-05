// -*-Mode: C++;-*-

#ifndef VAR_ANNOTATION_MAP_H
#define VAR_ANNOTATION_MAP_H

// Set of Var annotations, representing basic variable information,
// including the methods for computing the information. This should
// probably be split up into the different analyses at some point.

#include <map>

#include <analysis/AnnotationMap.h>

// ----- forward declarations -----

class R_IRInterface;

namespace RAnnot {

class VarAnnotationMap : public AnnotationMap {
public:
  // constructor/deconstructor
  VarAnnotationMap(bool ownsAnnotations = true);
  virtual ~VarAnnotationMap();

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
  void compute_all_syntactic_info();
  void compute_all_locality_info();
  void compute_locality_info(OA::OA_ptr<R_IRInterface> interface,
			     OA::ProcHandle proc,
			     OA::OA_ptr<OA::CFG::Interface> cfg);
  void compute_all_bindings();

private:
  bool m_computed; // has our information been computed yet?
  std::map<MyKeyT, MyMappedT> m_map;
};

}

#endif

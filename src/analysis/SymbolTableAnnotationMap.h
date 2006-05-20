// -*- Mode: C++ -*-

#ifndef SYMBOL_TABLE_ANNOTATION_MAP_H
#define SYMBOL_TABLE_ANNOTATION_MAP_H

/// Symbol table for each procedure. A symbol table maps names (SEXP
/// of SYMSXP type) to VarInfos.

#include <map>

#include <OpenAnalysis/CFG/Interface.hpp>

#include <analysis/AnnotationMap.h>
#include <analysis/PropertyHndl.h>

// ----- forward declarations -----

class R_IRInterface;

namespace RAnnot {

class SymbolTableAnnotationMap : public AnnotationMap {
public:
  // deconstructor
  virtual ~SymbolTableAnnotationMap();

  // demand-driven analysis
  MyMappedT & operator[](const MyKeyT & k); // FIXME: remove this when refactoring is done
  MyMappedT get(const MyKeyT & k);
  bool is_computed();

  // singleton
  static SymbolTableAnnotationMap * get_instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

  // iterators
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

private:
  // private constructor for singleton pattern
  SymbolTableAnnotationMap(bool ownsAnnotations = true);

  void compute();
  void compute_all_syntactic_info();
  void compute_all_locality_info();
  void compute_locality_info(OA::OA_ptr<R_IRInterface> interface,
			     OA::ProcHandle proc,
			     OA::OA_ptr<OA::CFG::Interface> cfg);

  bool m_computed; // has our information been computed yet?
  std::map<MyKeyT, MyMappedT> m_map;

  static SymbolTableAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();
};

}


#endif

// -*-Mode: C++;-*-

#ifndef FUNC_INFO_ANNOTATION_MAP_H
#define FUNC_INFO_ANNOTATION_MAP_H

// Set of FuncInfo annotations representing function information,
// including the methods for computing the information. We use a
// special destructor for the FuncInfos uses as values in the map;
// FuncInfos use the NonUniformDegreeTree library, which has its own
// mechanism for deletion.

// TODO: This should be split up into the different analyses at some
// point.

#include <map>

#include <analysis/DefaultAnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class FuncInfo;

class FuncInfoAnnotationMap : public DefaultAnnotationMap {
public:
  // constructor/deconstructor
  virtual ~FuncInfoAnnotationMap();

  // singleton
  static FuncInfoAnnotationMap * get_instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

private:
  /// private constructor for singleton pattern
  explicit FuncInfoAnnotationMap();

  /// traverse the program, create a FuncInfo for each function
  /// definition with the whole program as the root
  void compute();

  /// Build the scope tree with the given SEXP as the root, using the
  /// recursive version. Precondition: root must be an assignment
  /// where the right side is a function.
  void build_scope_tree(SEXP root);

  /// Recursively traverse e to build the scope tree. 'parent' is a
  /// pointer to the parent lexical scope.
  void build_scope_tree_rec(SEXP e, FuncInfo * parent);

  /// Get FuncInfos for libraries called in the program
  void collect_libraries();

  static FuncInfoAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();
};

}

#endif

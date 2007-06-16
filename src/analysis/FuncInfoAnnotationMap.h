// -*-Mode: C++;-*-

#ifndef FUNC_INFO_ANNOTATION_MAP_H
#define FUNC_INFO_ANNOTATION_MAP_H

// Set of FuncInfo annotations representing function information,
// including the methods for computing the information. This should
// probably be split up into the different analyses at some point.

#include <map>

#include <analysis/AnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class FuncInfo;

class FuncInfoAnnotationMap : public AnnotationMap {
public:
  // constructor/deconstructor
  virtual ~FuncInfoAnnotationMap();

  // demand-driven analysis
  MyMappedT & operator[](const MyKeyT & k); // TODO: remove this when refactoring is done
  MyMappedT get(const MyKeyT & k);
  bool is_computed();

  // singleton
  static FuncInfoAnnotationMap * get_instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

  // iterators
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

private:
  /// private constructor for singleton pattern
  FuncInfoAnnotationMap();

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

  bool m_computed; // has our information been computed yet?
  std::map<MyKeyT, MyMappedT> m_map;

  static FuncInfoAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();
};

}

#endif

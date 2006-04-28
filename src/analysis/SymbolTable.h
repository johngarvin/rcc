// -*- Mode: C++ -*-

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

//****************************************************************************
// Annotations: Symbol tables
//
// A symbol table maps names (SEXPs of SYMSXP type) to VarInfos.
//****************************************************************************

#include <map>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationBase.h>
#include <analysis/PropertyHndl.h>

// ---------------------------------------------------------------------------
// Name (symbol table key)
// ---------------------------------------------------------------------------

typedef SEXP Name;  // should be of type SYMSXP

// ---------------------------------------------------------------------------
// Symbol table
// ---------------------------------------------------------------------------

namespace RAnnot {

// ----- forward declarations -----

class VarInfo;

// ----- class definition -----

class SymbolTable 
  : public AnnotationBase
{
public:
  enum AllocT {
    Alloc_Heap,  // the heap
    Alloc_Frame, // the local frame of the execution stack
    Alloc_Stack  // separately managed stack of objects
  };

  typedef Name                                              key_type;
  typedef VarInfo *                                         mapped_type;
  
  typedef std::map<Name, VarInfo *>                         MyMap_t;
  typedef std::pair<const key_type, mapped_type>            value_type;
  typedef MyMap_t::key_compare                              key_compare;
  typedef MyMap_t::allocator_type                           allocator_type;
  typedef MyMap_t::reference                                reference;
  typedef MyMap_t::const_reference                          const_reference;
  typedef MyMap_t::iterator                                 iterator;
  typedef MyMap_t::const_iterator                           const_iterator;
  typedef MyMap_t::size_type                                size_type;

public:
  SymbolTable();
  virtual ~SymbolTable();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual SymbolTable * clone() { return new SymbolTable(*this); }

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

  // -------------------------------------------------------
  // iterator, find/insert, etc 
  // -------------------------------------------------------

  // iterators:
  iterator begin() 
    { return mVars.begin(); }
  const_iterator begin() const 
    { return mVars.begin(); }
  iterator end() 
    { return mVars.end(); }
  const_iterator end() const 
    { return mVars.end(); }
  
  // capacity:
  size_type size() const
    { return mVars.size(); }

  // element access:
  mapped_type & operator[](const key_type& x)
    { return mVars[x]; }

  // modifiers:
  std::pair<iterator, bool> insert(const value_type& x)
    { return mVars.insert(x); }
  iterator insert(iterator position, const value_type& x)
    { return mVars.insert(position, x); }

  void erase(iterator position) 
    { mVars.erase(position); }
  size_type erase(const key_type& x) 
    { return mVars.erase(x); }
  void erase(iterator first, iterator last) 
    { return mVars.erase(first, last); }

  void clear() 
    { mVars.clear(); }

  // map operations:
  iterator find(const key_type& x)
    { return mVars.find(x); }
  const_iterator find(const key_type& x) const
    { return mVars.find(x); }
  size_type count(const key_type& x) const
    { return mVars.count(x); }
  
  // -------------------------------------------------------
  // code generation
  // -------------------------------------------------------
  // genCode1: generate code to initialize ST
  // genCode2...

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  MyMap_t mVars; // (contents of map not owned)
};

} // end namespace RAnnot

#endif

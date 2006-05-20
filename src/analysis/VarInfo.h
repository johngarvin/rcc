#ifndef ANNOTATION_VAR_INFO_H
#define ANNOTATION_VAR_INFO_H

#include <ostream>
#include <set>

#include <analysis/DefVar.h>

namespace RAnnot {

//****************************************************************************
// Annotations: Variable info
//****************************************************************************

// ---------------------------------------------------------------------------
// VarInfo: Location information about a variable
// ---------------------------------------------------------------------------
class VarInfo
  : public AnnotationBase
{
public:
  typedef DefVar*                                           key_type;

  typedef std::set<key_type>                                MySet_t;
  typedef key_type                                          value_type;
  typedef MySet_t::key_compare                              key_compare;
  typedef MySet_t::value_compare                            value_compare;
  typedef MySet_t::iterator                                 iterator;
  typedef MySet_t::const_iterator                           const_iterator;
  typedef MySet_t::size_type                                size_type;

public:
  VarInfo();
  virtual ~VarInfo();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual VarInfo* clone() { return new VarInfo(*this); }

  // uses iterators:
  iterator beginDefs()
    { return mDefs.begin(); }
  const_iterator beginDefs() const
    { return mDefs.begin(); }
  iterator endDefs()
    { return mDefs.end(); }
  const_iterator endDefs() const
    { return mDefs.end(); }
  
  // uses capacity:
  size_type sizeDefs() const
    { return mDefs.size(); }
  
  // uses modifiers:
  std::pair<iterator,bool> insertDef(const value_type& x)
    { return mDefs.insert(x); }
  iterator insertDef(iterator position, const value_type& x)
    { return mDefs.insert(position, x); }

  void eraseDefs(iterator position)
    { mDefs.erase(position); }
  size_type eraseDefs(const key_type& x)
    { return mDefs.erase(x); }
  void eraseDefs(iterator first, iterator last)
    { return mDefs.erase(first, last); }

  void clearDefs()
    { mDefs.clear(); }

  // uses set operations:
  iterator findDefs(const key_type& x) const
    { return mDefs.find(x); }
  size_type countDefs(const key_type& x) const
    { return mDefs.count(x); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  // data_type mType
  MySet_t mDefs;
};

}

#endif

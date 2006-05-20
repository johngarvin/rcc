#ifndef ANNOTATION_PHI_H
#define ANNOTATION_PHI_H

#include <set>
#include <ostream>

#include <analysis/VarInfo.h>

namespace RAnnot {

// ---------------------------------------------------------------------------
// Phi: 
// ---------------------------------------------------------------------------
class Phi
  : public VarInfo
{
public:
  typedef VarInfo*                                          key_type;

  typedef std::set<key_type>                                MySet_t;
  typedef key_type                                          value_type;
  typedef MySet_t::key_compare                              key_compare;
  typedef MySet_t::value_compare                            value_compare;
  typedef MySet_t::iterator                                 iterator;
  typedef MySet_t::const_iterator                           const_iterator;
  typedef MySet_t::size_type                                size_type;

public:
  Phi();
  virtual ~Phi();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual Phi* clone() { return new Phi(*this); }

  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------

  // reaching-defs iterators:
  iterator beginReachingDefs()
    { return mReachingDefs.begin(); }
  const_iterator beginReachingDefs() const
    { return mReachingDefs.begin(); }
  iterator endReachingDefs()
    { return mReachingDefs.end(); }
  const_iterator endReachingDefs() const
    { return mReachingDefs.end(); }
  
  // reaching-defs capacity:
  size_type sizeReachingDefs() const
    { return mReachingDefs.size(); }
  
  // reaching-defs modifiers:
  std::pair<iterator,bool> insertReachingDefs(const value_type& x)
    { return mReachingDefs.insert(x); }
  iterator insertReachingDefs(iterator position, const value_type& x)
    { return mReachingDefs.insert(position, x); }

  void eraseReachingDefs(iterator position)
    { mReachingDefs.erase(position); }
  size_type eraseReachingDefs(const key_type& x)
    { return mReachingDefs.erase(x); }
  void eraseReachingDefs(iterator first, iterator last)
    { return mReachingDefs.erase(first, last); }

  void clearReachingDefs()
    { mReachingDefs.clear(); }

  // reaching-defs set operations:
  iterator findReachingDefs(const key_type& x) const
    { return mReachingDefs.find(x); }
  size_type countReachingDefs(const key_type& x) const
    { return mReachingDefs.count(x); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  MySet_t mReachingDefs; // (set members not owned)
};

}

#endif

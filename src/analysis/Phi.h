// -*- Mode: C++ -*-
//
// Copyright (c) 2006 Rice University
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 

// File: Phi.h
//
// Annotation for a phi node.
//
// Author: John Garvin (garvin@cs.rice.edu)

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

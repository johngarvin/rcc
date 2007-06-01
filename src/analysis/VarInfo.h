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

// File: VarInfo.h
//
// In each SymbolTable, a name maps to a VarInfo. A VarInfo is
// associated with a coordinate (a name in a scope) and contains a
// list of defs that define the name in that scope. (The defs are not
// necessarily in the variable's scope; they may be local ("<-") in
// the variable's scope or free ("<<-") in child scopes.)
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_VAR_INFO_H
#define ANNOTATION_VAR_INFO_H

#include <ostream>
#include <list>

#include <analysis/DefVar.h>

// ----- forward declaration -----

class SubexpBuffer;

namespace RAnnot {

/// VarInfo: Location information about a variable
class VarInfo
  : public AnnotationBase
{
public:
  typedef DefVar *                                          key_type;

  typedef std::list<key_type>                               MySet_t;
  typedef key_type                                          value_type;
  typedef MySet_t::iterator                                 iterator;
  typedef MySet_t::const_iterator                           const_iterator;
  typedef MySet_t::size_type                                size_type;

public:
  VarInfo();
  virtual ~VarInfo();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual VarInfo * clone() { return new VarInfo(*this); }

  // defs iterators:
  iterator beginDefs()
    { return mDefs.begin(); }
  const_iterator beginDefs() const
    { return mDefs.begin(); }
  iterator endDefs()
    { return mDefs.end(); }
  const_iterator endDefs() const
    { return mDefs.end(); }
  
  // defs capacity:
  size_type sizeDefs() const
    { return mDefs.size(); }
  
  // defs modifiers:
  void insertDef(const value_type& x)
    { mDefs.push_back(x); }
  iterator insertDef(iterator position, const value_type& x)
    { return mDefs.insert(position, x); }

  void eraseDefs(iterator position)
    { mDefs.erase(position); }
  void eraseDefs(iterator first, iterator last)
    { mDefs.erase(first, last); }

  void clearDefs()
    { mDefs.clear(); }

  /// get the location in the R environment
  std::string get_location(SubexpBuffer* sb);

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  // data_type mType
  MySet_t mDefs;
  std::string m_c_location;
};

}

#endif
